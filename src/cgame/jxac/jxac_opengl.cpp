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
    qglReadPixels = (glReadPixels_t)GetProcAddress(hOpenGL, "glReadPixels");
    qglReadBuffer = (glReadBuffer_t)GetProcAddress(hOpenGL, "glReadBuffer");
    qglPixelStorei = (glPixelStorei_t)GetProcAddress(hOpenGL, "glPixelStorei");
    qglGetError = (glGetError_t)GetProcAddress(hOpenGL, "glGetError");
    
    // glBindFramebuffer is an extension, need to get via wglGetProcAddress
    // Note: PROC is the correct return type per Windows API, but void* works the same on x64
    typedef PROC (WINAPI *wglGetProcAddress_t)(LPCSTR);
    wglGetProcAddress_t wglGetProcAddr = (wglGetProcAddress_t)GetProcAddress(hOpenGL, "wglGetProcAddress");
    if (wglGetProcAddr) {
        qglBindFramebuffer = (glBindFramebuffer_t)wglGetProcAddr("glBindFramebuffer");
        if (!qglBindFramebuffer) {
            qglBindFramebuffer = (glBindFramebuffer_t)wglGetProcAddr("glBindFramebufferEXT");
        }
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
    if (!qglReadPixels) {
        return false;
    }
    
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
}

} // namespace OpenGL
} // namespace jxac
