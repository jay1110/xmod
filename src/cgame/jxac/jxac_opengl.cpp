// Direct OpenGL function loading for screenshot capture
// Bypasses engine syscall system and calls OpenGL directly
// Based on nitmod/StackOverflow approach
//
// Windows: GetModuleHandle("opengl32") + GetProcAddress()
// Linux: dlopen("libGL.so.1") + dlsym()

#include <bgame/impl.h>
#include "jxac_opengl.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace jxac {
namespace OpenGL {

// Function pointers
static glReadPixels_t   qglReadPixels = NULL;
static glReadBuffer_t   qglReadBuffer = NULL;
static glPixelStorei_t  qglPixelStorei = NULL;
static glGetError_t     qglGetError = NULL;

// Library handle (Linux only)
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

#ifdef _WIN32
    // Windows: Get handle to opengl32.dll (already loaded by engine)
    HMODULE hOpenGL = GetModuleHandle("opengl32");
    if (!hOpenGL) {
        CG_Printf("JXAC OpenGL DEBUG: FAILED - GetModuleHandle(\"opengl32\") returned NULL\n");
        return false;
    }
    CG_Printf("JXAC OpenGL DEBUG: Got opengl32.dll handle: %p\n", (void*)hOpenGL);
    
    // Get function pointers
    qglReadPixels = (glReadPixels_t)GetProcAddress(hOpenGL, "glReadPixels");
    qglReadBuffer = (glReadBuffer_t)GetProcAddress(hOpenGL, "glReadBuffer");
    qglPixelStorei = (glPixelStorei_t)GetProcAddress(hOpenGL, "glPixelStorei");
    qglGetError = (glGetError_t)GetProcAddress(hOpenGL, "glGetError");
    
    CG_Printf("JXAC OpenGL DEBUG: Function pointers:\n");
    CG_Printf("  glReadPixels:  %p\n", (void*)qglReadPixels);
    CG_Printf("  glReadBuffer:  %p\n", (void*)qglReadBuffer);
    CG_Printf("  glPixelStorei: %p\n", (void*)qglPixelStorei);
    CG_Printf("  glGetError:    %p\n", (void*)qglGetError);
    
#else
    // Linux: Load libGL.so.1
    glLibrary = dlopen("libGL.so.1", RTLD_LAZY | RTLD_NOLOAD);
    if (!glLibrary) {
        // Library not loaded yet, try loading it
        glLibrary = dlopen("libGL.so.1", RTLD_LAZY);
    }
    if (!glLibrary) {
        // Try alternative name
        glLibrary = dlopen("libGL.so", RTLD_LAZY);
    }
    if (!glLibrary) {
        CG_Printf("JXAC OpenGL DEBUG: FAILED - dlopen(\"libGL.so.1\") returned NULL: %s\n", dlerror());
        return false;
    }
    CG_Printf("JXAC OpenGL DEBUG: Got libGL.so handle: %p\n", glLibrary);
    
    // Get function pointers
    qglReadPixels = (glReadPixels_t)dlsym(glLibrary, "glReadPixels");
    qglReadBuffer = (glReadBuffer_t)dlsym(glLibrary, "glReadBuffer");
    qglPixelStorei = (glPixelStorei_t)dlsym(glLibrary, "glPixelStorei");
    qglGetError = (glGetError_t)dlsym(glLibrary, "glGetError");
    
    CG_Printf("JXAC OpenGL DEBUG: Function pointers:\n");
    CG_Printf("  glReadPixels:  %p\n", (void*)qglReadPixels);
    CG_Printf("  glReadBuffer:  %p\n", (void*)qglReadBuffer);
    CG_Printf("  glPixelStorei: %p\n", (void*)qglPixelStorei);
    CG_Printf("  glGetError:    %p\n", (void*)qglGetError);
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
    
    // Select back buffer for reading (IMPORTANT - this is what was missing!)
    if (qglReadBuffer) {
        CG_Printf("JXAC OpenGL DEBUG: Calling glReadBuffer(GL_BACK=0x%04X)\n", JXAC_GL_BACK);
        qglReadBuffer(JXAC_GL_BACK);
        
        if (qglGetError) {
            unsigned int err = qglGetError();
            if (err != JXAC_GL_NO_ERROR) {
                CG_Printf("JXAC OpenGL DEBUG: glReadBuffer error: 0x%04X\n", err);
            }
        }
    }
    
    // Read pixels from framebuffer
    CG_Printf("JXAC OpenGL DEBUG: Calling glReadPixels(%d, %d, %d, %d, GL_RGB=0x%04X, GL_UNSIGNED_BYTE=0x%04X, %p)\n",
             x, y, width, height, JXAC_GL_RGB, JXAC_GL_UNSIGNED_BYTE, (void*)buffer);
    
    qglReadPixels(x, y, width, height, JXAC_GL_RGB, JXAC_GL_UNSIGNED_BYTE, buffer);
    
    // Check for errors
    if (qglGetError) {
        unsigned int err = qglGetError();
        if (err != JXAC_GL_NO_ERROR) {
            CG_Printf("JXAC OpenGL DEBUG: glReadPixels error: 0x%04X\n", err);
            return false;
        }
    }
    
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
