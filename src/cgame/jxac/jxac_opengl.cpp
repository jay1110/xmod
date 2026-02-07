// Direct OpenGL function loading for screenshot capture
// Bypasses engine syscall system and calls OpenGL directly
// Based on nitmod/StackOverflow approach
//
// Windows: GetModuleHandle("opengl32") + GetProcAddress()
// Linux/aarch64: dlopen("libGL.so.1") + dlsym()
// macOS: dlopen OpenGL.framework + dlsym()
// Android: dlopen("libGLESv2.so") + dlsym()

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
    
    CG_Printf("JXAC OpenGL DEBUG: Initializing direct OpenGL access...\n");

#if defined(JXAC_PLATFORM_ANDROID)
    // Android: OpenGL ES screenshot capture is not yet implemented
    // Return false to gracefully skip screenshot instead of crashing
    CG_Printf("JXAC OpenGL DEBUG: Android platform detected\n");
    CG_Printf("^3JXAC: Screenshot capture not yet supported on Android (OpenGL ES)\n");
    return false;
#endif

#ifdef _WIN32
    // Windows (32-bit and 64-bit): Get handle to opengl32.dll (already loaded by engine)
    // Use GetModuleHandleA explicitly for ANSI string
    HMODULE hOpenGL = GetModuleHandleA("opengl32.dll");
    if (!hOpenGL) {
        CG_Printf("JXAC OpenGL DEBUG: FAILED - GetModuleHandleA(\"opengl32.dll\") returned NULL\n");
        return false;
    }
    CG_Printf("JXAC OpenGL DEBUG: Got opengl32.dll handle: %p\n", (void*)hOpenGL);
    
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
    
    CG_Printf("JXAC OpenGL DEBUG: Function pointers:\n");
    CG_Printf("  glReadPixels:      %p\n", (void*)qglReadPixels);
    CG_Printf("  glReadBuffer:      %p\n", (void*)qglReadBuffer);
    CG_Printf("  glPixelStorei:     %p\n", (void*)qglPixelStorei);
    CG_Printf("  glGetError:        %p\n", (void*)qglGetError);
    CG_Printf("  glBindFramebuffer: %p\n", (void*)qglBindFramebuffer);
    
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
    CG_Printf("JXAC OpenGL DEBUG: macOS platform detected, using OpenGL.framework\n");
#elif defined(JXAC_PLATFORM_ANDROID)
    // Android: Use OpenGL ES
    const char* libPaths[] = {
        "libGLESv3.so",
        "libGLESv2.so",
        NULL
    };
    CG_Printf("JXAC OpenGL DEBUG: Android platform detected, using OpenGL ES\n");
#else
    // Linux (x86, x86_64, aarch64): Use libGL
    const char* libPaths[] = {
        "libGL.so.1",
        "libGL.so",
        NULL
    };
    CG_Printf("JXAC OpenGL DEBUG: Linux platform detected, using libGL\n");
#endif
    
    // Try to get handle to already-loaded library
    for (int i = 0; libPaths[i] != NULL; i++) {
        glLibrary = dlopen(libPaths[i], RTLD_LAZY | RTLD_NOLOAD);
        if (glLibrary) {
            CG_Printf("JXAC OpenGL DEBUG: Got already-loaded %s handle: %p\n", libPaths[i], glLibrary);
            break;
        }
    }
    
    // If not already loaded, try loading it
    if (!glLibrary) {
        for (int i = 0; libPaths[i] != NULL; i++) {
            glLibrary = dlopen(libPaths[i], RTLD_LAZY);
            if (glLibrary) {
                CG_Printf("JXAC OpenGL DEBUG: Loaded %s handle: %p\n", libPaths[i], glLibrary);
                break;
            }
        }
    }
    
    if (!glLibrary) {
        CG_Printf("JXAC OpenGL DEBUG: FAILED - could not load OpenGL library: %s\n", dlerror());
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
    
    CG_Printf("JXAC OpenGL DEBUG: Function pointers:\n");
    CG_Printf("  glReadPixels:      %p\n", (void*)qglReadPixels);
    CG_Printf("  glReadBuffer:      %p\n", (void*)qglReadBuffer);
    CG_Printf("  glPixelStorei:     %p\n", (void*)qglPixelStorei);
    CG_Printf("  glGetError:        %p\n", (void*)qglGetError);
    CG_Printf("  glBindFramebuffer: %p\n", (void*)qglBindFramebuffer);
#endif
    
    // Check required functions
    if (!qglReadPixels) {
        CG_Printf("JXAC OpenGL DEBUG: FAILED - glReadPixels not found\n");
        return false;
    }
    
    // glReadBuffer and glPixelStorei are optional but recommended
    if (!qglReadBuffer) {
        CG_Printf("JXAC OpenGL DEBUG: WARNING - glReadBuffer not found (may affect capture)\n");
    }
    if (!qglPixelStorei) {
        CG_Printf("JXAC OpenGL DEBUG: WARNING - glPixelStorei not found (may affect alignment)\n");
    }
    
    isInit = true;
    CG_Printf("JXAC OpenGL DEBUG: Initialization successful\n");
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
    
    CG_Printf("JXAC OpenGL DEBUG: Shutdown complete\n");
}

///////////////////////////////////////////////////////////////////////////////

bool isInitialized() {
    return isInit;
}

///////////////////////////////////////////////////////////////////////////////

bool captureFramebuffer(int x, int y, int width, int height, unsigned char* buffer) {
    if (!isInit) {
        CG_Printf("JXAC OpenGL DEBUG: captureFramebuffer() FAILED - not initialized\n");
        if (!init()) {
            return false;
        }
    }
    
    if (!buffer) {
        CG_Printf("JXAC OpenGL DEBUG: captureFramebuffer() FAILED - buffer is NULL\n");
        return false;
    }
    
    CG_Printf("JXAC OpenGL DEBUG: captureFramebuffer(%d, %d, %d, %d, %p)\n",
             x, y, width, height, (void*)buffer);
    
    // Clear any existing GL errors
    if (qglGetError) {
        unsigned int err;
        while ((err = qglGetError()) != JXAC_GL_NO_ERROR) {
            CG_Printf("JXAC OpenGL DEBUG: Cleared pre-existing GL error: 0x%04X\n", err);
        }
    }
    
    // IMPORTANT: Bind default framebuffer (0) first
    // This ensures we're not reading from an FBO
    if (qglBindFramebuffer) {
        CG_Printf("JXAC OpenGL DEBUG: Calling glBindFramebuffer(GL_FRAMEBUFFER=0x%04X, 0)\n", JXAC_GL_FRAMEBUFFER);
        qglBindFramebuffer(JXAC_GL_FRAMEBUFFER, 0);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                CG_Printf("JXAC OpenGL DEBUG: glBindFramebuffer error: 0x%04X (may be expected if no FBO support)\n", err);
            }
        }
    } else {
        CG_Printf("JXAC OpenGL DEBUG: glBindFramebuffer not available, skipping\n");
    }
    
    // Set pixel store alignment to 1 (no padding between rows)
    // This is important for proper pixel reading
    if (qglPixelStorei) {
        CG_Printf("JXAC OpenGL DEBUG: Calling glPixelStorei(GL_PACK_ALIGNMENT, 1)\n");
        qglPixelStorei(JXAC_GL_PACK_ALIGNMENT, 1);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                CG_Printf("JXAC OpenGL DEBUG: glPixelStorei error: 0x%04X\n", err);
            }
        }
    }
    
    // Set up read buffer
    // On Linux/macOS/Android with modern display systems, GL_FRONT is often invalid
    // On Windows, GL_FRONT typically works better after SwapBuffers
    bool readBufferSuccess = false;
    
    if (qglReadBuffer) {
#ifdef _WIN32
        // Windows: Try GL_FRONT first (displayed buffer after SwapBuffers)
        CG_Printf("JXAC OpenGL DEBUG: Windows - trying GL_FRONT first\n");
        qglReadBuffer(JXAC_GL_FRONT);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                CG_Printf("JXAC OpenGL DEBUG: glReadBuffer(GL_FRONT) error: 0x%04X, trying GL_BACK\n", err);
                qglReadBuffer(JXAC_GL_BACK);
                err = qglGetError ? qglGetError() : JXAC_GL_NO_ERROR;
                readBufferSuccess = (err == JXAC_GL_NO_ERROR);
            } else {
                readBufferSuccess = true;
            }
        } else {
            readBufferSuccess = true;
        }
#else
        // Linux/macOS/Android: Try GL_BACK first
        // Modern display systems (compositors, Wayland, etc.) don't maintain GL_FRONT
        CG_Printf("JXAC OpenGL DEBUG: Unix - trying GL_BACK first\n");
        qglReadBuffer(JXAC_GL_BACK);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                CG_Printf("JXAC OpenGL DEBUG: glReadBuffer(GL_BACK) error: 0x%04X, trying GL_FRONT\n", err);
                qglReadBuffer(JXAC_GL_FRONT);
                err = qglGetError ? qglGetError() : JXAC_GL_NO_ERROR;
                readBufferSuccess = (err == JXAC_GL_NO_ERROR);
            } else {
                readBufferSuccess = true;
            }
        } else {
            readBufferSuccess = true;
        }
#endif
    }
    
    CG_Printf("JXAC OpenGL DEBUG: Read buffer setup %s\n", readBufferSuccess ? "succeeded" : "skipped/failed (trying anyway)");
    
    // Read pixels from framebuffer
    // On Android (OpenGL ES), we MUST use GL_RGBA - GL_RGB is not guaranteed to work
    // Then convert RGBA to RGB for the output buffer
#if defined(JXAC_PLATFORM_ANDROID)
    CG_Printf("JXAC OpenGL DEBUG: Android - using GL_RGBA (required by OpenGL ES)\n");
    
    // Allocate temporary RGBA buffer (use size_t to avoid overflow on large screens)
    size_t rgbaBufferSize = (size_t)width * (size_t)height * 4;
    
    // Sanity check - max ~64MB
    if (rgbaBufferSize > 64 * 1024 * 1024) {
        CG_Printf("JXAC OpenGL DEBUG: FAILED - RGBA buffer too large (%zu bytes)\n", rgbaBufferSize);
        return false;
    }
    
    unsigned char* rgbaBuffer = (unsigned char*)malloc(rgbaBufferSize);
    if (!rgbaBuffer) {
        CG_Printf("JXAC OpenGL DEBUG: FAILED - malloc failed for RGBA buffer (%zu bytes)\n", rgbaBufferSize);
        return false;
    }
    
    CG_Printf("JXAC OpenGL DEBUG: Calling glReadPixels with GL_RGBA\n");
    qglReadPixels(x, y, width, height, JXAC_GL_RGBA, JXAC_GL_UNSIGNED_BYTE, rgbaBuffer);
    
    // Check for errors
    if (qglGetError) {
        unsigned int err = qglGetError();
        if (err != JXAC_GL_NO_ERROR) {
            CG_Printf("JXAC OpenGL DEBUG: glReadPixels error: 0x%04X\n", err);
            free(rgbaBuffer);
            return false;
        }
    }
    
    // Convert RGBA to RGB
    CG_Printf("JXAC OpenGL DEBUG: Converting RGBA to RGB...\n");
    for (int i = 0; i < width * height; i++) {
        buffer[i * 3 + 0] = rgbaBuffer[i * 4 + 0];  // R
        buffer[i * 3 + 1] = rgbaBuffer[i * 4 + 1];  // G
        buffer[i * 3 + 2] = rgbaBuffer[i * 4 + 2];  // B
    }
    
    free(rgbaBuffer);
    CG_Printf("JXAC OpenGL DEBUG: RGBA to RGB conversion complete\n");
#else
    // Desktop OpenGL (Windows, Linux, macOS): GL_RGB works fine
    CG_Printf("JXAC OpenGL DEBUG: Calling glReadPixels with GL_RGB\n");
    qglReadPixels(x, y, width, height, JXAC_GL_RGB, JXAC_GL_UNSIGNED_BYTE, buffer);
    
    // Check for errors
    if (qglGetError) {
        unsigned int err = qglGetError();
        if (err != JXAC_GL_NO_ERROR) {
            CG_Printf("JXAC OpenGL DEBUG: glReadPixels error: 0x%04X\n", err);
            return false;
        }
    }
#endif
    
    // Check if we got any data (first pixel shouldn't be all zero typically)
    CG_Printf("JXAC OpenGL DEBUG: glReadPixels completed, first 8 bytes: %02X %02X %02X %02X %02X %02X %02X %02X\n",
             buffer[0], buffer[1], buffer[2], buffer[3],
             buffer[4], buffer[5], buffer[6], buffer[7]);
    
    // Also check middle of image
    int midOffset = (height / 2) * width * 3 + (width / 2) * 3;
    CG_Printf("JXAC OpenGL DEBUG: Middle pixel bytes: %02X %02X %02X\n",
             buffer[midOffset], buffer[midOffset+1], buffer[midOffset+2]);
    
    CG_Printf("JXAC OpenGL DEBUG: captureFramebuffer() SUCCESS\n");
    return true;
}

} // namespace OpenGL
} // namespace jxac
