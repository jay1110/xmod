// Direct OpenGL function loading for screenshot capture
// Bypasses engine syscall system and calls OpenGL directly
// Based on nitmod/StackOverflow approach
//
// Windows: GetModuleHandle("opengl32") + GetProcAddress()
// Linux/aarch64: dlopen("libGL.so.1") + dlsym()
// macOS: dlopen OpenGL.framework + dlsym()
// Android: NOT SUPPORTED - cgame runs in different thread than OpenGL renderer

#include <bgame/impl.h>
#include "jxac_opengl.h"

// Platform detection - MUST be at top before any code uses it
#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #define JXAC_PLATFORM_MACOS
#elif defined(__ANDROID__)
    #define JXAC_PLATFORM_ANDROID
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <cstdlib>  // for malloc/free
#include <climits>
#include <cstdio>
#include <cstring>

namespace jxac {
namespace OpenGL {

// Function pointers
static glReadPixels_t       qglReadPixels = NULL;
static glReadBuffer_t       qglReadBuffer = NULL;
static glPixelStorei_t      qglPixelStorei = NULL;
static glGetError_t         qglGetError = NULL;
static glBindFramebuffer_t  qglBindFramebuffer = NULL;

#ifdef _WIN32
typedef void (JXAC_GL_CALL *GetIntegervFn)(unsigned int, int*);
typedef const unsigned char* (JXAC_GL_CALL *GetStringFn)(unsigned int);
typedef void (JXAC_GL_CALL *GenTexturesFn)(int, unsigned int*);
typedef void (JXAC_GL_CALL *DeleteTexturesFn)(int, const unsigned int*);
typedef void (JXAC_GL_CALL *BindTextureFn)(unsigned int, unsigned int);
typedef void (JXAC_GL_CALL *TexImage2DFn)(unsigned int, int, int, int, int, int, unsigned int, unsigned int, const void*);
typedef void (JXAC_GL_CALL *CopyTexSubImage2DFn)(unsigned int, int, int, int, int, int, int, int);
typedef void (JXAC_GL_CALL *GetTexImageFn)(unsigned int, int, unsigned int, unsigned int, void*);
typedef void (JXAC_GL_CALL *BindBufferFn)(unsigned int, unsigned int);
static GetIntegervFn qglGetIntegerv = NULL;
static GetStringFn qglGetString = NULL;
static GenTexturesFn qglGenTextures = NULL;
static DeleteTexturesFn qglDeleteTextures = NULL;
static BindTextureFn qglBindTexture = NULL;
static TexImage2DFn qglTexImage2D = NULL;
static CopyTexSubImage2DFn qglCopyTexSubImage2D = NULL;
static GetTexImageFn qglGetTexImage = NULL;
static BindBufferFn qglBindBuffer = NULL;

enum {
    GL_VERSION_VALUE = 0x1F02, GL_EXTENSIONS_VALUE = 0x1F03,
    GL_TEXTURE_2D_VALUE = 0x0DE1, GL_TEXTURE_BINDING_2D_VALUE = 0x8069,
    GL_MAX_TEXTURE_SIZE_VALUE = 0x0D33, GL_RGB8_VALUE = 0x8051,
    GL_READ_BUFFER_VALUE = 0x0C02, GL_READ_FRAMEBUFFER_VALUE = 0x8CA8,
    GL_READ_FRAMEBUFFER_BINDING_VALUE = 0x8CAA, GL_DRAW_FRAMEBUFFER_BINDING_VALUE = 0x8CA6,
    GL_PACK_ROW_LENGTH_VALUE = 0x0D02, GL_PACK_SKIP_ROWS_VALUE = 0x0D03,
    GL_PACK_SKIP_PIXELS_VALUE = 0x0D04,
    GL_PIXEL_PACK_BUFFER_VALUE = 0x88EB, GL_PIXEL_UNPACK_BUFFER_VALUE = 0x88EC,
    GL_PIXEL_PACK_BUFFER_BINDING_VALUE = 0x88ED, GL_PIXEL_UNPACK_BUFFER_BINDING_VALUE = 0x88EF
};

static bool validExtensionProc(PROC proc) {
    const INT_PTR value = reinterpret_cast<INT_PTR>(proc);
    return proc && value != 1 && value != 2 && value != 3 && value != -1;
}

static bool hasExtension(const char* extensions, const char* name) {
    if (!extensions) return false;
    const size_t length = std::strlen(name);
    for (const char* p = extensions; (p = std::strstr(p, name)) != NULL; p += length) {
        if ((p == extensions || p[-1] == ' ') && (p[length] == ' ' || !p[length])) return true;
    }
    return false;
}

// Every state changed below is restored, including the default framebuffer's
// read-buffer selection when the engine originally had a different FBO bound.
struct CaptureState {
    int texture = 0, readBuffer = 0, defaultReadBuffer = 0;
    int readFramebuffer = 0, packBuffer = 0, unpackBuffer = 0;
    int pack[4] = {};
    bool separateFbo = false, hasFbo = false, hasPbo = false;
    bool active = false, defaultBufferSaved = false;
    unsigned int temporaryTexture = 0;

    static unsigned int packName(unsigned index) {
        const unsigned int names[] = {JXAC_GL_PACK_ALIGNMENT, GL_PACK_ROW_LENGTH_VALUE,
            GL_PACK_SKIP_ROWS_VALUE, GL_PACK_SKIP_PIXELS_VALUE};
        return names[index];
    }
    bool restore() {
        if (!active) return true;
        active = false;
        qglBindTexture(GL_TEXTURE_2D_VALUE, static_cast<unsigned int>(texture));
        if (temporaryTexture) qglDeleteTextures(1, &temporaryTexture);
        for (unsigned i = 0; i < 4; ++i) qglPixelStorei(packName(i), pack[i]);
        if (hasPbo) {
            qglBindBuffer(GL_PIXEL_PACK_BUFFER_VALUE, static_cast<unsigned int>(packBuffer));
            qglBindBuffer(GL_PIXEL_UNPACK_BUFFER_VALUE, static_cast<unsigned int>(unpackBuffer));
        }
        const unsigned int target = separateFbo ? GL_READ_FRAMEBUFFER_VALUE : JXAC_GL_FRAMEBUFFER;
        if (defaultBufferSaved) {
            if (hasFbo) qglBindFramebuffer(target, 0);
            qglReadBuffer(static_cast<unsigned int>(defaultReadBuffer));
        }
        if (hasFbo) qglBindFramebuffer(target, static_cast<unsigned int>(readFramebuffer));
        qglReadBuffer(static_cast<unsigned int>(readBuffer));
        return qglGetError() == JXAC_GL_NO_ERROR;
    }
    ~CaptureState() { restore(); }
};

static bool captureTexture(int x, int y, int width, int height, unsigned char* buffer) {
    if (!buffer || x < 0 || y < 0 || width <= 0 || height <= 0 ||
        width > INT_MAX - x || height > INT_MAX - y) return false;
    // A lost/missing context must not turn error draining into an endless loop.
    unsigned errors = 0;
    while (qglGetError() != JXAC_GL_NO_ERROR) if (++errors == 32) return false;
    const char* version = reinterpret_cast<const char*>(qglGetString(GL_VERSION_VALUE));
    int major = 0, minor = 0;
    if (!version || std::sscanf(version, "%d.%d", &major, &minor) != 2 || major < 1) return false;
    const char* extensions = major < 3
        ? reinterpret_cast<const char*>(qglGetString(GL_EXTENSIONS_VALUE)) : NULL;
    CaptureState state;
    state.separateFbo = major >= 3 || hasExtension(extensions, "GL_ARB_framebuffer_object") ||
        hasExtension(extensions, "GL_EXT_framebuffer_blit");
    state.hasFbo = state.separateFbo || hasExtension(extensions, "GL_EXT_framebuffer_object");
    state.hasPbo = major > 2 || (major == 2 && minor >= 1) ||
        hasExtension(extensions, "GL_ARB_pixel_buffer_object") || hasExtension(extensions, "GL_EXT_pixel_buffer_object");
    if ((state.hasFbo && !qglBindFramebuffer) || (state.hasPbo && !qglBindBuffer)) return false;
    int limit = 0;
    qglGetIntegerv(GL_MAX_TEXTURE_SIZE_VALUE, &limit);
    if (qglGetError() != JXAC_GL_NO_ERROR || limit <= 0 || width > limit || height > limit) return false;
    // Power-of-two allocation also works on desktop OpenGL 1.1. Copy only the
    // requested rectangle, then strip padded columns during RGB readback.
    int textureWidth = 1, textureHeight = 1;
    while (textureWidth < width) { if (textureWidth > limit / 2) return false; textureWidth *= 2; }
    while (textureHeight < height) { if (textureHeight > limit / 2) return false; textureHeight *= 2; }
    const size_t memoryLimit = 64u * 1024u * 1024u;
    if (static_cast<size_t>(textureWidth) > memoryLimit / 3 / static_cast<size_t>(textureHeight)) return false;
    const size_t textureBytes = static_cast<size_t>(textureWidth) * textureHeight * 3;
    unsigned char* pixels = static_cast<unsigned char*>(std::malloc(textureBytes));
    if (!pixels) return false;

    qglGetIntegerv(GL_TEXTURE_BINDING_2D_VALUE, &state.texture);
    qglGetIntegerv(GL_READ_BUFFER_VALUE, &state.readBuffer);
    for (unsigned i = 0; i < 4; ++i) qglGetIntegerv(CaptureState::packName(i), &state.pack[i]);
    if (state.hasFbo) qglGetIntegerv(state.separateFbo ? GL_READ_FRAMEBUFFER_BINDING_VALUE :
        GL_DRAW_FRAMEBUFFER_BINDING_VALUE, &state.readFramebuffer);
    if (state.hasPbo) {
        qglGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_VALUE, &state.packBuffer);
        qglGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING_VALUE, &state.unpackBuffer);
    }
    if (qglGetError() != JXAC_GL_NO_ERROR) { std::free(pixels); return false; }
    state.active = true;
    bool success = false;
    do {
        if (state.hasFbo) qglBindFramebuffer(state.separateFbo ? GL_READ_FRAMEBUFFER_VALUE : JXAC_GL_FRAMEBUFFER, 0);
        if (qglGetError() != JXAC_GL_NO_ERROR) break;
        qglGetIntegerv(GL_READ_BUFFER_VALUE, &state.defaultReadBuffer);
        if (qglGetError() != JXAC_GL_NO_ERROR) break;
        state.defaultBufferSaved = true;
        // Capture the displayed frame without invoking the glReadPixels path.
        qglReadBuffer(JXAC_GL_FRONT);
        if (qglGetError() != JXAC_GL_NO_ERROR) break;
        if (state.hasPbo) {
            qglBindBuffer(GL_PIXEL_PACK_BUFFER_VALUE, 0);
            qglBindBuffer(GL_PIXEL_UNPACK_BUFFER_VALUE, 0);
        }
        for (unsigned i = 0; i < 4; ++i) qglPixelStorei(CaptureState::packName(i), i == 0 ? 1 : 0);
        qglGenTextures(1, &state.temporaryTexture);
        if (!state.temporaryTexture || qglGetError() != JXAC_GL_NO_ERROR) break;
        qglBindTexture(GL_TEXTURE_2D_VALUE, state.temporaryTexture);
        if (qglGetError() != JXAC_GL_NO_ERROR) break;
        qglTexImage2D(GL_TEXTURE_2D_VALUE, 0, GL_RGB8_VALUE, textureWidth, textureHeight, 0,
            JXAC_GL_RGB, JXAC_GL_UNSIGNED_BYTE, NULL);
        if (qglGetError() != JXAC_GL_NO_ERROR) break;
        qglCopyTexSubImage2D(GL_TEXTURE_2D_VALUE, 0, 0, 0, x, y, width, height);
        if (qglGetError() != JXAC_GL_NO_ERROR) break;
        qglGetTexImage(GL_TEXTURE_2D_VALUE, 0, JXAC_GL_RGB, JXAC_GL_UNSIGNED_BYTE, pixels);
        if (qglGetError() != JXAC_GL_NO_ERROR) break;
        for (int row = 0; row < height; ++row) std::memcpy(buffer + static_cast<size_t>(row) * width * 3,
            pixels + static_cast<size_t>(row) * textureWidth * 3, static_cast<size_t>(width) * 3);
        success = true;
    } while (false);
    const bool restored = state.restore();
    std::free(pixels);
    return success && restored;
}
#endif // _WIN32

// Library handle (non-Windows only)
#ifndef _WIN32
static void* glLibrary = NULL;
#endif

static bool isInit = false;

///////////////////////////////////////////////////////////////////////////////

bool init() {
    if (isInit) {
        return true;
    }
    

// Android: Direct OpenGL calls from cgame are NOT possible.
// The cgame module runs in a different thread than the OpenGL renderer,
// and the EGL context is not bound in the cgame thread - even during CG_DrawActiveFrame.
// On Android, we use trap_R_ReadPixels (engine syscall) instead, which is handled
// in Client::captureScreenshot(). Direct OpenGL init is not needed.
#if defined(JXAC_PLATFORM_ANDROID)
    return false;
#endif

#ifdef _WIN32
    // Windows (32-bit and 64-bit): Get handle to opengl32.dll (already loaded by engine)
    // Use GetModuleHandleA explicitly for ANSI string
    HMODULE hOpenGL = GetModuleHandleA("opengl32.dll");
    if (!hOpenGL) {
        return false;
    }
    
    // Get function pointers - these are standard OpenGL functions exported by opengl32.dll
    qglReadBuffer = (glReadBuffer_t)GetProcAddress(hOpenGL, "glReadBuffer");
    qglPixelStorei = (glPixelStorei_t)GetProcAddress(hOpenGL, "glPixelStorei");
    qglGetError = (glGetError_t)GetProcAddress(hOpenGL, "glGetError");
    qglGetIntegerv = (GetIntegervFn)GetProcAddress(hOpenGL, "glGetIntegerv");
    qglGetString = (GetStringFn)GetProcAddress(hOpenGL, "glGetString");
    qglGenTextures = (GenTexturesFn)GetProcAddress(hOpenGL, "glGenTextures");
    qglDeleteTextures = (DeleteTexturesFn)GetProcAddress(hOpenGL, "glDeleteTextures");
    qglBindTexture = (BindTextureFn)GetProcAddress(hOpenGL, "glBindTexture");
    qglTexImage2D = (TexImage2DFn)GetProcAddress(hOpenGL, "glTexImage2D");
    qglCopyTexSubImage2D = (CopyTexSubImage2DFn)GetProcAddress(hOpenGL, "glCopyTexSubImage2D");
    qglGetTexImage = (GetTexImageFn)GetProcAddress(hOpenGL, "glGetTexImage");
    
    // glBindFramebuffer is an extension, need to get via wglGetProcAddress
    // Note: PROC is the correct return type per Windows API, but void* works the same on x64
    typedef PROC (WINAPI *wglGetProcAddress_t)(LPCSTR);
    wglGetProcAddress_t wglGetProcAddr = (wglGetProcAddress_t)GetProcAddress(hOpenGL, "wglGetProcAddress");
    if (wglGetProcAddr) {
        PROC framebuffer = wglGetProcAddr("glBindFramebuffer");
        if (!validExtensionProc(framebuffer)) framebuffer = wglGetProcAddr("glBindFramebufferEXT");
        qglBindFramebuffer = validExtensionProc(framebuffer) ? (glBindFramebuffer_t)framebuffer : NULL;
        PROC bindBuffer = wglGetProcAddr("glBindBuffer");
        if (!validExtensionProc(bindBuffer)) bindBuffer = wglGetProcAddr("glBindBufferARB");
        qglBindBuffer = validExtensionProc(bindBuffer) ? (BindBufferFn)bindBuffer : NULL;
    }
    
    
#else
    // Unix-like: Linux, macOS, Android
    // Try different libraries depending on platform
    
#if defined(JXAC_PLATFORM_MACOS)
    // macOS: Use OpenGL framework
    const char* libPaths[] = {
        "/System/Library/Frameworks/OpenGL.framework/OpenGL",
        "/System/Library/Frameworks/OpenGL.framework/Versions/A/OpenGL",
        NULL
    };
#elif defined(JXAC_PLATFORM_ANDROID)
    // Android: Use OpenGL ES
    const char* libPaths[] = {
        "libGLESv3.so",
        "libGLESv2.so",
        NULL
    };
#else
    // Linux (x86, x86_64, aarch64): Use libGL
    const char* libPaths[] = {
        "libGL.so.1",
        "libGL.so",
        NULL
    };
#endif
    
    // Try to get handle to already-loaded library
    for (int i = 0; libPaths[i] != NULL; i++) {
        glLibrary = dlopen(libPaths[i], RTLD_LAZY | RTLD_NOLOAD);
        if (glLibrary) {
            break;
        }
    }
    
    // If not already loaded, try loading it
    if (!glLibrary) {
        for (int i = 0; libPaths[i] != NULL; i++) {
            glLibrary = dlopen(libPaths[i], RTLD_LAZY);
            if (glLibrary) {
                break;
            }
        }
    }
    
    if (!glLibrary) {
        return false;
    }
    
    // Get function pointers
    qglReadPixels = (glReadPixels_t)dlsym(glLibrary, "glReadPixels");
    qglReadBuffer = (glReadBuffer_t)dlsym(glLibrary, "glReadBuffer");
    qglPixelStorei = (glPixelStorei_t)dlsym(glLibrary, "glPixelStorei");
    qglGetError = (glGetError_t)dlsym(glLibrary, "glGetError");
    qglBindFramebuffer = (glBindFramebuffer_t)dlsym(glLibrary, "glBindFramebuffer");
    if (!qglBindFramebuffer) {
        qglBindFramebuffer = (glBindFramebuffer_t)dlsym(glLibrary, "glBindFramebufferEXT");
    }
    
#endif
    
    // Check required functions
#ifdef _WIN32
    if (!qglReadBuffer || !qglPixelStorei || !qglGetError || !qglGetIntegerv || !qglGetString ||
        !qglGenTextures || !qglDeleteTextures || !qglBindTexture || !qglTexImage2D ||
        !qglCopyTexSubImage2D || !qglGetTexImage) return false;
#else
    if (!qglReadPixels) {
        return false;
    }
#endif
    
    // glReadBuffer and glPixelStorei are optional but recommended
    
    isInit = true;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void shutdown() {
#ifndef _WIN32
    if (glLibrary) {
        // Don't actually close it - engine might still need it
        // dlclose(glLibrary);
        glLibrary = NULL;
    }
#endif
    
    qglReadPixels = NULL;
    qglReadBuffer = NULL;
    qglPixelStorei = NULL;
    qglGetError = NULL;
    qglBindFramebuffer = NULL;
#ifdef _WIN32
    qglGetIntegerv = NULL;
    qglGetString = NULL;
    qglGenTextures = NULL;
    qglDeleteTextures = NULL;
    qglBindTexture = NULL;
    qglTexImage2D = NULL;
    qglCopyTexSubImage2D = NULL;
    qglGetTexImage = NULL;
    qglBindBuffer = NULL;
#endif
    isInit = false;
}

///////////////////////////////////////////////////////////////////////////////

bool isInitialized() {
    return isInit;
}

///////////////////////////////////////////////////////////////////////////////

bool captureFramebuffer(int x, int y, int width, int height, unsigned char* buffer) {
    if (!isInit) {
        if (!init()) {
            return false;
        }
    }
    
    if (!buffer) {
        return false;
    }

#ifdef _WIN32
    // Failure remains a capture failure; never fall back to glReadPixels.
    return captureTexture(x, y, width, height, buffer);
#else
    
    // Clear any existing GL errors
    if (qglGetError) {
        unsigned int err;
        while ((err = qglGetError()) != JXAC_GL_NO_ERROR) {
        }
    }
    
    // IMPORTANT: Bind default framebuffer (0) first
    // This ensures we're not reading from an FBO
    if (qglBindFramebuffer) {
        qglBindFramebuffer(JXAC_GL_FRAMEBUFFER, 0);
    }
    
    // Set pixel store alignment to 1 (no padding between rows)
    // This is important for proper pixel reading
    if (qglPixelStorei) {
        qglPixelStorei(JXAC_GL_PACK_ALIGNMENT, 1);
    }
    
    // Set up read buffer
    // IMPORTANT: glReadBuffer does NOT exist in OpenGL ES (Android)!
    // ET Legacy doesn't use glReadBuffer at all - they just call glReadPixels directly.
    // We only use glReadBuffer on desktop OpenGL where it exists.
#if defined(JXAC_PLATFORM_ANDROID)
    // Android/OpenGL ES: DO NOT call glReadBuffer - it doesn't exist!
    // Just read directly from the default framebuffer (which is already bound above)
#else
    // Desktop OpenGL: Set read buffer
    if (qglReadBuffer) {
#ifdef _WIN32
        // Windows: Try GL_FRONT first (displayed buffer after SwapBuffers)
        qglReadBuffer(JXAC_GL_FRONT);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                qglReadBuffer(JXAC_GL_BACK);
            }
        }
#else
        // Linux/macOS: Try GL_BACK first (modern compositors don't maintain GL_FRONT)
        qglReadBuffer(JXAC_GL_BACK);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                qglReadBuffer(JXAC_GL_FRONT);
            }
        }
#endif
    }
#endif
    
    // Read pixels from framebuffer
    // 
    // ANALYSIS OF ET LEGACY CODE (src/renderer/tr_init.c line 284):
    //   glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, bufstart);
    // 
    // ET Legacy uses GL_RGB on ALL platforms including Android!
    // They do NOT use GL_RGBA and they do NOT call glReadBuffer.
    //
    // Based on OpenGL ES 2.0 spec, the ONLY guaranteed combination is:
    //   GL_RGBA + GL_UNSIGNED_BYTE
    // However, many implementations support GL_RGB as well.
    //
    // Strategy:
    // 1. Try GL_RGB first (like ET Legacy does - it works on most implementations)
    // 2. If that fails with GL_INVALID_OPERATION, try GL_RGBA and convert
    
#if defined(JXAC_PLATFORM_ANDROID)
    // Android: Try GL_RGB first (ET Legacy approach), fall back to GL_RGBA if it fails
    
    // Clear any errors
    if (qglGetError) {
        while (qglGetError() != JXAC_GL_NO_ERROR) {}
    }
    
    qglReadPixels(x, y, width, height, JXAC_GL_RGB, JXAC_GL_UNSIGNED_BYTE, buffer);
    
    bool rgbSuccess = true;
    if (qglGetError) {
        unsigned int err = qglGetError();
        if (err != JXAC_GL_NO_ERROR) {
            rgbSuccess = false;
        }
    }
    
    if (!rgbSuccess) {
        // GL_RGB failed, try GL_RGBA
        
        size_t rgbaBufferSize = (size_t)width * (size_t)height * 4;
        if (rgbaBufferSize > 64 * 1024 * 1024) {
            return false;
        }
        
        unsigned char* rgbaBuffer = (unsigned char*)malloc(rgbaBufferSize);
        if (!rgbaBuffer) {
            return false;
        }
        
        qglReadPixels(x, y, width, height, JXAC_GL_RGBA, JXAC_GL_UNSIGNED_BYTE, rgbaBuffer);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                free(rgbaBuffer);
                return false;
            }
        }
        
        // Convert RGBA to RGB
        for (int i = 0; i < width * height; i++) {
            buffer[i * 3 + 0] = rgbaBuffer[i * 4 + 0];
            buffer[i * 3 + 1] = rgbaBuffer[i * 4 + 1];
            buffer[i * 3 + 2] = rgbaBuffer[i * 4 + 2];
        }
        free(rgbaBuffer);
    }
#else
    // Desktop OpenGL (Windows, Linux, macOS): GL_RGB works fine
    qglReadPixels(x, y, width, height, JXAC_GL_RGB, JXAC_GL_UNSIGNED_BYTE, buffer);
    
    // Check for errors
    if (qglGetError) {
        unsigned int err = qglGetError();
        if (err != JXAC_GL_NO_ERROR) {
            return false;
        }
    }
#endif
    
    return true;
#endif // _WIN32
}

} // namespace OpenGL
} // namespace jxac
