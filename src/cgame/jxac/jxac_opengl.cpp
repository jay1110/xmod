// Direct OpenGL function loading for screenshot capture
// Bypasses engine syscall system and calls OpenGL directly
// Based on nitmod/StackOverflow approach
//
// Windows: GetModuleHandle("opengl32") + GetProcAddress()
// Linux/aarch64: dlopen("libGL.so.1") + dlsym()
// macOS: dlopen OpenGL.framework + dlsym()

#include <bgame/impl.h>
#include "jxac_opengl.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <cstdlib>  // for malloc/free

// Platform detection
#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #define JXAC_PLATFORM_MACOS
#elif defined(__ANDROID__)
    #define JXAC_PLATFORM_ANDROID
#endif

namespace jxac {
namespace OpenGL {

// Function pointers
static glReadPixels_t       qglReadPixels = NULL;
static glReadBuffer_t       qglReadBuffer = NULL;
static glPixelStorei_t      qglPixelStorei = NULL;
static glGetError_t         qglGetError = NULL;
static glBindFramebuffer_t  qglBindFramebuffer = NULL;

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

#if defined(JXAC_PLATFORM_ANDROID)
    // Android: OpenGL ES screenshot capture is not yet supported
    // Return false to gracefully skip screenshot instead of crashing
    CG_Printf("^3JXAC: Screenshot capture not supported on Android\n");
    return false;
#endif

#ifdef _WIN32
    // Windows (32-bit and 64-bit): Get handle to opengl32.dll (already loaded by engine)
    HMODULE hOpenGL = GetModuleHandleA("opengl32.dll");
    if (!hOpenGL) {
        CG_Printf("^1JXAC ERROR: Could not get opengl32.dll handle\n");
        return false;
    }
    
    // Get function pointers
    qglReadPixels = (glReadPixels_t)GetProcAddress(hOpenGL, "glReadPixels");
    qglReadBuffer = (glReadBuffer_t)GetProcAddress(hOpenGL, "glReadBuffer");
    qglPixelStorei = (glPixelStorei_t)GetProcAddress(hOpenGL, "glPixelStorei");
    qglGetError = (glGetError_t)GetProcAddress(hOpenGL, "glGetError");
    
    // glBindFramebuffer is an extension, need to get via wglGetProcAddress
    typedef PROC (WINAPI *wglGetProcAddress_t)(LPCSTR);
    wglGetProcAddress_t wglGetProcAddr = (wglGetProcAddress_t)GetProcAddress(hOpenGL, "wglGetProcAddress");
    if (wglGetProcAddr) {
        qglBindFramebuffer = (glBindFramebuffer_t)wglGetProcAddr("glBindFramebuffer");
        if (!qglBindFramebuffer) {
            qglBindFramebuffer = (glBindFramebuffer_t)wglGetProcAddr("glBindFramebufferEXT");
        }
    }
    
#else
    // Unix-like: Linux, macOS
    // (Android returns early above)
    
#if defined(JXAC_PLATFORM_MACOS)
    const char* libPaths[] = {
        "/System/Library/Frameworks/OpenGL.framework/OpenGL",
        "/System/Library/Frameworks/OpenGL.framework/Versions/A/OpenGL",
        NULL
    };
#else
    // Linux (x86, x86_64, aarch64)
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
        CG_Printf("^1JXAC ERROR: Could not load OpenGL library: %s\n", dlerror());
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
    if (!qglReadPixels) {
        CG_Printf("^1JXAC ERROR: glReadPixels not found\n");
        return false;
    }
    
    isInit = true;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void shutdown() {
#ifndef _WIN32
    if (glLibrary) {
        glLibrary = NULL;
    }
#endif
    
    qglReadPixels = NULL;
    qglReadBuffer = NULL;
    qglPixelStorei = NULL;
    qglGetError = NULL;
    qglBindFramebuffer = NULL;
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
        CG_Printf("^1JXAC ERROR: captureFramebuffer() - buffer is NULL\n");
        return false;
    }
    
    // Clear any existing GL errors
    if (qglGetError) {
        while (qglGetError() != JXAC_GL_NO_ERROR) { }
    }
    
    // Bind default framebuffer (0) - ensures we're not reading from an FBO
    if (qglBindFramebuffer) {
        qglBindFramebuffer(JXAC_GL_FRAMEBUFFER, 0);
        if (qglGetError) {
            qglGetError(); // Clear any error, not fatal
        }
    }
    
    // Set pixel store alignment to 1 (no padding between rows)
    if (qglPixelStorei) {
        qglPixelStorei(JXAC_GL_PACK_ALIGNMENT, 1);
        if (qglGetError) {
            qglGetError(); // Clear any error, not fatal
        }
    }
    
    // Set up read buffer
    if (qglReadBuffer) {
#ifdef _WIN32
        // Windows: Try GL_FRONT first
        qglReadBuffer(JXAC_GL_FRONT);
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                qglReadBuffer(JXAC_GL_BACK);
                if (qglGetError) qglGetError();
            }
        }
#else
        // Linux/macOS: Try GL_BACK first
        qglReadBuffer(JXAC_GL_BACK);
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                qglReadBuffer(JXAC_GL_FRONT);
                if (qglGetError) qglGetError();
            }
        }
#endif
    }
    
    // Read pixels from framebuffer using GL_RGB
    qglReadPixels(x, y, width, height, JXAC_GL_RGB, JXAC_GL_UNSIGNED_BYTE, buffer);
    
    // Check for errors
    if (qglGetError) {
        unsigned int err = qglGetError();
        if (err != JXAC_GL_NO_ERROR) {
            CG_Printf("^1JXAC ERROR: glReadPixels failed with error 0x%04X\n", err);
            return false;
        }
    }
    
    return true;
}

} // namespace OpenGL
} // namespace jxac
