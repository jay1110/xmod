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
#include <cstdio>   // for FILE, fprintf, fflush

// =============================================================================
// FILE-BASED DEBUG LOGGING
// Writes to jxac_debug.log immediately with fflush() so logs survive crashes
// =============================================================================
static FILE* debugLogFile = NULL;

static void JXAC_DebugLog(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    Q_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    // Open file if not yet open - use different paths for different platforms
    if (!debugLogFile) {
#ifdef JXAC_PLATFORM_ANDROID
        // Android: Try multiple paths
        debugLogFile = fopen("/sdcard/jxac_debug.log", "a");
        if (!debugLogFile) {
            debugLogFile = fopen("/storage/emulated/0/jxac_debug.log", "a");
        }
        if (!debugLogFile) {
            debugLogFile = fopen("jxac_debug.log", "a");
        }
#else
        debugLogFile = fopen("jxac_debug.log", "a");
#endif
    }
    
    // Write to file
    if (debugLogFile) {
        fprintf(debugLogFile, "%s", buf);
        fflush(debugLogFile);  // CRITICAL: flush immediately so logs survive crash
    }
    
    // Also print to console
    CG_Printf("%s", buf);
}

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
    
    JXAC_DebugLog("JXAC OpenGL DEBUG: Initializing direct OpenGL access...\n");

// NOTE: Android early-return was REMOVED because we now defer the capture to the GL thread.
// The capture happens in Client::frame() which is called from CG_DrawActiveFrame (GL thread).
// So the OpenGL context IS available when captureFramebuffer() is called.

#ifdef _WIN32
    // Windows (32-bit and 64-bit): Get handle to opengl32.dll (already loaded by engine)
    // Use GetModuleHandleA explicitly for ANSI string
    HMODULE hOpenGL = GetModuleHandleA("opengl32.dll");
    if (!hOpenGL) {
        JXAC_DebugLog("JXAC OpenGL DEBUG: FAILED - GetModuleHandleA(\"opengl32.dll\") returned NULL\n");
        return false;
    }
    JXAC_DebugLog("JXAC OpenGL DEBUG: Got opengl32.dll handle: %p\n", (void*)hOpenGL);
    
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
    
    JXAC_DebugLog("JXAC OpenGL DEBUG: Function pointers:\n");
    JXAC_DebugLog("  glReadPixels:      %p\n", (void*)qglReadPixels);
    JXAC_DebugLog("  glReadBuffer:      %p\n", (void*)qglReadBuffer);
    JXAC_DebugLog("  glPixelStorei:     %p\n", (void*)qglPixelStorei);
    JXAC_DebugLog("  glGetError:        %p\n", (void*)qglGetError);
    JXAC_DebugLog("  glBindFramebuffer: %p\n", (void*)qglBindFramebuffer);
    
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
    JXAC_DebugLog("JXAC OpenGL DEBUG: macOS platform detected, using OpenGL.framework\n");
#elif defined(JXAC_PLATFORM_ANDROID)
    // Android: Use OpenGL ES
    const char* libPaths[] = {
        "libGLESv3.so",
        "libGLESv2.so",
        NULL
    };
    JXAC_DebugLog("JXAC OpenGL DEBUG: Android platform detected, using OpenGL ES\n");
#else
    // Linux (x86, x86_64, aarch64): Use libGL
    const char* libPaths[] = {
        "libGL.so.1",
        "libGL.so",
        NULL
    };
    JXAC_DebugLog("JXAC OpenGL DEBUG: Linux platform detected, using libGL\n");
#endif
    
    // Try to get handle to already-loaded library
    for (int i = 0; libPaths[i] != NULL; i++) {
        glLibrary = dlopen(libPaths[i], RTLD_LAZY | RTLD_NOLOAD);
        if (glLibrary) {
            JXAC_DebugLog("JXAC OpenGL DEBUG: Got already-loaded %s handle: %p\n", libPaths[i], glLibrary);
            break;
        }
    }
    
    // If not already loaded, try loading it
    if (!glLibrary) {
        for (int i = 0; libPaths[i] != NULL; i++) {
            glLibrary = dlopen(libPaths[i], RTLD_LAZY);
            if (glLibrary) {
                JXAC_DebugLog("JXAC OpenGL DEBUG: Loaded %s handle: %p\n", libPaths[i], glLibrary);
                break;
            }
        }
    }
    
    if (!glLibrary) {
        JXAC_DebugLog("JXAC OpenGL DEBUG: FAILED - could not load OpenGL library: %s\n", dlerror());
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
    
    JXAC_DebugLog("JXAC OpenGL DEBUG: Function pointers:\n");
    JXAC_DebugLog("  glReadPixels:      %p\n", (void*)qglReadPixels);
    JXAC_DebugLog("  glReadBuffer:      %p\n", (void*)qglReadBuffer);
    JXAC_DebugLog("  glPixelStorei:     %p\n", (void*)qglPixelStorei);
    JXAC_DebugLog("  glGetError:        %p\n", (void*)qglGetError);
    JXAC_DebugLog("  glBindFramebuffer: %p\n", (void*)qglBindFramebuffer);
#endif
    
    // Check required functions
    if (!qglReadPixels) {
        JXAC_DebugLog("JXAC OpenGL DEBUG: FAILED - glReadPixels not found\n");
        return false;
    }
    
    // glReadBuffer and glPixelStorei are optional but recommended
    if (!qglReadBuffer) {
        JXAC_DebugLog("JXAC OpenGL DEBUG: WARNING - glReadBuffer not found (may affect capture)\n");
    }
    if (!qglPixelStorei) {
        JXAC_DebugLog("JXAC OpenGL DEBUG: WARNING - glPixelStorei not found (may affect alignment)\n");
    }
    
    isInit = true;
    JXAC_DebugLog("JXAC OpenGL DEBUG: Initialization successful\n");
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
    
    JXAC_DebugLog("JXAC OpenGL DEBUG: Shutdown complete\n");
}

///////////////////////////////////////////////////////////////////////////////

bool isInitialized() {
    return isInit;
}

///////////////////////////////////////////////////////////////////////////////

bool captureFramebuffer(int x, int y, int width, int height, unsigned char* buffer) {
    if (!isInit) {
        JXAC_DebugLog("JXAC OpenGL DEBUG: captureFramebuffer() FAILED - not initialized\n");
        if (!init()) {
            return false;
        }
    }
    
    if (!buffer) {
        JXAC_DebugLog("JXAC OpenGL DEBUG: captureFramebuffer() FAILED - buffer is NULL\n");
        return false;
    }
    
    JXAC_DebugLog("JXAC OpenGL DEBUG: captureFramebuffer(%d, %d, %d, %d, %p)\n",
             x, y, width, height, (void*)buffer);
    
    // Clear any existing GL errors
    if (qglGetError) {
        unsigned int err;
        while ((err = qglGetError()) != JXAC_GL_NO_ERROR) {
            JXAC_DebugLog("JXAC OpenGL DEBUG: Cleared pre-existing GL error: 0x%04X\n", err);
        }
    }
    
    // IMPORTANT: Bind default framebuffer (0) first
    // This ensures we're not reading from an FBO
    if (qglBindFramebuffer) {
        JXAC_DebugLog("JXAC OpenGL DEBUG: Calling glBindFramebuffer(GL_FRAMEBUFFER=0x%04X, 0)\n", JXAC_GL_FRAMEBUFFER);
        qglBindFramebuffer(JXAC_GL_FRAMEBUFFER, 0);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                JXAC_DebugLog("JXAC OpenGL DEBUG: glBindFramebuffer error: 0x%04X (may be expected if no FBO support)\n", err);
            }
        }
    } else {
        JXAC_DebugLog("JXAC OpenGL DEBUG: glBindFramebuffer not available, skipping\n");
    }
    
    // Set pixel store alignment to 1 (no padding between rows)
    // This is important for proper pixel reading
    if (qglPixelStorei) {
        JXAC_DebugLog("JXAC OpenGL DEBUG: Calling glPixelStorei(GL_PACK_ALIGNMENT, 1)\n");
        qglPixelStorei(JXAC_GL_PACK_ALIGNMENT, 1);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                JXAC_DebugLog("JXAC OpenGL DEBUG: glPixelStorei error: 0x%04X\n", err);
            }
        }
    }
    
    // Set up read buffer
    // IMPORTANT: glReadBuffer does NOT exist in OpenGL ES (Android)!
    // ET Legacy doesn't use glReadBuffer at all - they just call glReadPixels directly.
    // We only use glReadBuffer on desktop OpenGL where it exists.
#if defined(JXAC_PLATFORM_ANDROID)
    // Android/OpenGL ES: DO NOT call glReadBuffer - it doesn't exist!
    // Just read directly from the default framebuffer (which is already bound above)
    JXAC_DebugLog("JXAC OpenGL DEBUG: Android - skipping glReadBuffer (doesn't exist in OpenGL ES)\n");
#else
    // Desktop OpenGL: Set read buffer
    if (qglReadBuffer) {
#ifdef _WIN32
        // Windows: Try GL_FRONT first (displayed buffer after SwapBuffers)
        JXAC_DebugLog("JXAC OpenGL DEBUG: Windows - trying GL_FRONT first\n");
        qglReadBuffer(JXAC_GL_FRONT);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                JXAC_DebugLog("JXAC OpenGL DEBUG: glReadBuffer(GL_FRONT) error: 0x%04X, trying GL_BACK\n", err);
                qglReadBuffer(JXAC_GL_BACK);
            }
        }
#else
        // Linux/macOS: Try GL_BACK first (modern compositors don't maintain GL_FRONT)
        JXAC_DebugLog("JXAC OpenGL DEBUG: Linux/macOS - trying GL_BACK first\n");
        qglReadBuffer(JXAC_GL_BACK);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                JXAC_DebugLog("JXAC OpenGL DEBUG: glReadBuffer(GL_BACK) error: 0x%04X, trying GL_FRONT\n", err);
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
    JXAC_DebugLog("JXAC OpenGL DEBUG: Android - trying GL_RGB first (ET Legacy method)\n");
    
    // Clear any errors
    if (qglGetError) {
        while (qglGetError() != JXAC_GL_NO_ERROR) {}
    }
    
    qglReadPixels(x, y, width, height, JXAC_GL_RGB, JXAC_GL_UNSIGNED_BYTE, buffer);
    
    bool rgbSuccess = true;
    if (qglGetError) {
        unsigned int err = qglGetError();
        if (err != JXAC_GL_NO_ERROR) {
            JXAC_DebugLog("JXAC OpenGL DEBUG: glReadPixels(GL_RGB) error: 0x%04X, trying GL_RGBA\n", err);
            rgbSuccess = false;
        }
    }
    
    if (!rgbSuccess) {
        // GL_RGB failed, try GL_RGBA
        JXAC_DebugLog("JXAC OpenGL DEBUG: Falling back to GL_RGBA...\n");
        
        size_t rgbaBufferSize = (size_t)width * (size_t)height * 4;
        if (rgbaBufferSize > 64 * 1024 * 1024) {
            JXAC_DebugLog("JXAC OpenGL DEBUG: FAILED - RGBA buffer too large (%zu bytes)\n", rgbaBufferSize);
            return false;
        }
        
        unsigned char* rgbaBuffer = (unsigned char*)malloc(rgbaBufferSize);
        if (!rgbaBuffer) {
            JXAC_DebugLog("JXAC OpenGL DEBUG: FAILED - malloc failed for RGBA buffer\n");
            return false;
        }
        
        qglReadPixels(x, y, width, height, JXAC_GL_RGBA, JXAC_GL_UNSIGNED_BYTE, rgbaBuffer);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                JXAC_DebugLog("JXAC OpenGL DEBUG: glReadPixels(GL_RGBA) also failed: 0x%04X\n", err);
                free(rgbaBuffer);
                return false;
            }
        }
        
        // Convert RGBA to RGB
        JXAC_DebugLog("JXAC OpenGL DEBUG: Converting RGBA to RGB...\n");
        for (int i = 0; i < width * height; i++) {
            buffer[i * 3 + 0] = rgbaBuffer[i * 4 + 0];
            buffer[i * 3 + 1] = rgbaBuffer[i * 4 + 1];
            buffer[i * 3 + 2] = rgbaBuffer[i * 4 + 2];
        }
        free(rgbaBuffer);
    }
#else
    // Desktop OpenGL (Windows, Linux, macOS): GL_RGB works fine
    JXAC_DebugLog("JXAC OpenGL DEBUG: Calling glReadPixels with GL_RGB\n");
    qglReadPixels(x, y, width, height, JXAC_GL_RGB, JXAC_GL_UNSIGNED_BYTE, buffer);
    
    // Check for errors
    if (qglGetError) {
        unsigned int err = qglGetError();
        if (err != JXAC_GL_NO_ERROR) {
            JXAC_DebugLog("JXAC OpenGL DEBUG: glReadPixels error: 0x%04X\n", err);
            return false;
        }
    }
#endif
    
    // Check if we got any data (first pixel shouldn't be all zero typically)
    JXAC_DebugLog("JXAC OpenGL DEBUG: glReadPixels completed, first 8 bytes: %02X %02X %02X %02X %02X %02X %02X %02X\n",
             buffer[0], buffer[1], buffer[2], buffer[3],
             buffer[4], buffer[5], buffer[6], buffer[7]);
    
    // Also check middle of image
    int midOffset = (height / 2) * width * 3 + (width / 2) * 3;
    JXAC_DebugLog("JXAC OpenGL DEBUG: Middle pixel bytes: %02X %02X %02X\n",
             buffer[midOffset], buffer[midOffset+1], buffer[midOffset+2]);
    
    JXAC_DebugLog("JXAC OpenGL DEBUG: captureFramebuffer() SUCCESS\n");
    return true;
}

} // namespace OpenGL
} // namespace jxac
