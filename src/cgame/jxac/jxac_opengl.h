#ifndef JXAC_OPENGL_H
#define JXAC_OPENGL_H

// Direct OpenGL function loading for screenshot capture
// Bypasses engine syscall system and calls OpenGL directly
// Based on nitmod/StackOverflow approach

// Calling convention for OpenGL functions
// CRITICAL: On Windows 32-bit, OpenGL uses __stdcall (APIENTRY/WINAPI)
// On 64-bit and Linux, calling convention doesn't matter
#ifdef _WIN32
    #ifndef APIENTRY
        #define APIENTRY __stdcall
    #endif
    #define JXAC_GL_CALL APIENTRY
#else
    #define JXAC_GL_CALL
#endif

namespace jxac {
namespace OpenGL {

// OpenGL constants
#define JXAC_GL_FRONT               0x0404
#define JXAC_GL_BACK                0x0405
#define JXAC_GL_RGB                 0x1907
#define JXAC_GL_RGBA                0x1908
#define JXAC_GL_UNSIGNED_BYTE       0x1401
#define JXAC_GL_PACK_ALIGNMENT      0x0D05
#define JXAC_GL_FRAMEBUFFER         0x8D40
#define JXAC_GL_NO_ERROR            0

// Function pointer types with correct calling convention
// CRITICAL: Must use APIENTRY (__stdcall) on Windows 32-bit or it will crash!
typedef void (JXAC_GL_CALL *glReadPixels_t)(int x, int y, int width, int height, unsigned int format, unsigned int type, void *pixels);
typedef void (JXAC_GL_CALL *glReadBuffer_t)(unsigned int mode);
typedef void (JXAC_GL_CALL *glPixelStorei_t)(unsigned int pname, int param);
typedef unsigned int (JXAC_GL_CALL *glGetError_t)(void);
typedef void (JXAC_GL_CALL *glBindFramebuffer_t)(unsigned int target, unsigned int framebuffer);

// Initialize OpenGL function pointers
// Returns true if successful, false otherwise
bool init();

// Shutdown (unload library on Linux)
void shutdown();

// Check if initialized
bool isInitialized();

// Capture framebuffer to buffer
// Returns true on success, false on failure
// buffer must be pre-allocated with width*height*3 bytes (RGB output)
// On Android (OpenGL ES), internally captures RGBA and converts to RGB
bool captureFramebuffer(int x, int y, int width, int height, unsigned char* buffer);

} // namespace OpenGL
} // namespace jxac

#endif // JXAC_OPENGL_H
