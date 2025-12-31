#include <bgame/impl.h>
#include <bgame/jxac_common.h>
#include <cgame/jxac/jxac_screenshot.h>

// Include stb_image_write for JPEG compression
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC(sz)        malloc(sz)
#define STBIW_REALLOC(p,newsz)  realloc(p,newsz)
#define STBIW_FREE(p)           free(p)
#include <base/stb_image_write.h>

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

// Callback context for in-memory JPEG writing
typedef struct {
    unsigned char* buffer;
    int size;
    int capacity;
} JpegWriteContext;

// Callback function for stbi_write_jpg_to_func
static void jpegWriteCallback( void* context, void* data, int size ) {
    JpegWriteContext* ctx = (JpegWriteContext*)context;
    
    // Expand buffer if needed
    while ( ctx->size + size > ctx->capacity ) {
        int newCapacity = ctx->capacity * 2;
        unsigned char* newBuffer = (unsigned char*)realloc( ctx->buffer, newCapacity );
        if ( !newBuffer ) {
            // realloc failed - mark size as -1 to indicate error
            // Caller will check for this and free the original buffer
            ctx->size = -1;
            return;
        }
        ctx->buffer = newBuffer;
        ctx->capacity = newCapacity;
    }
    
    // Copy data to buffer
    memcpy( ctx->buffer + ctx->size, data, size );
    ctx->size += size;
}

///////////////////////////////////////////////////////////////////////////////

unsigned char* Screenshot::captureFramebuffer( int* width, int* height, int* channels ) {
    // TODO: Implement screenshot capture that works with stock ET engine
    // FIXME: The trap_R_ReadPixels() function calls the CG_R_READPIXELS syscall,
    // which does not exist in the stock Wolfenstein: Enemy Territory engine.
    // Calling this function causes a client crash.
    //
    // Possible solutions:
    // 1. Require a custom ET engine build that implements CG_R_READPIXELS
    // 2. Use engine's screenshot command and read from disk (async, complex)
    // 3. Disable screenshot functionality until engine support is available
    //
    // For now, we return NULL to prevent crashes. Screenshot requests will
    // fail gracefully instead of crashing the client.
    
    // Silent failure - screenshot capture not available without engine support
    return NULL;
    
    /* DISABLED - Causes crash on stock ET engine
    // Get current screen dimensions
    *width = cgs.glconfig.vidWidth;
    *height = cgs.glconfig.vidHeight;
    *channels = 3; // RGB
    
    // Allocate buffer for framebuffer data (RGBA from OpenGL, we'll convert to RGB)
    int rgbaBufferSize = (*width) * (*height) * 4; // RGBA
    unsigned char* rgbaBuffer = (unsigned char*)malloc( rgbaBufferSize );
    
    if ( !rgbaBuffer ) {
        // Silent failure
        return NULL;
    }
    
    // Read framebuffer pixels using engine API
    // OpenGL reads pixels from bottom to top, so we'll need to flip
    trap_R_ReadPixels( 0, 0, *width, *height, rgbaBuffer );
    
    // Allocate RGB buffer (3 channels instead of 4)
    int rgbBufferSize = (*width) * (*height) * (*channels);
    unsigned char* rgbBuffer = (unsigned char*)malloc( rgbBufferSize );
    
    if ( !rgbBuffer ) {
        free( rgbaBuffer );
        return NULL;
    }
    
    // Convert RGBA to RGB and flip vertically
    // OpenGL reads bottom-to-top, JPEG expects top-to-bottom
    for ( int y = 0; y < *height; y++ ) {
        for ( int x = 0; x < *width; x++ ) {
            // Source: bottom-to-top (OpenGL)
            int srcY = (*height) - 1 - y;
            int srcIdx = (srcY * (*width) + x) * 4; // RGBA
            
            // Destination: top-to-bottom (JPEG)
            int dstIdx = (y * (*width) + x) * 3; // RGB
            
            // Copy RGB, skip A
            rgbBuffer[dstIdx + 0] = rgbaBuffer[srcIdx + 0]; // R
            rgbBuffer[dstIdx + 1] = rgbaBuffer[srcIdx + 1]; // G
            rgbBuffer[dstIdx + 2] = rgbaBuffer[srcIdx + 2]; // B
        }
    }
    
    free( rgbaBuffer );
    return rgbBuffer;
    */
}

///////////////////////////////////////////////////////////////////////////////

unsigned char* Screenshot::captureAndCompress( int* outSize, int quality ) {
    if ( !outSize ) {
        // Silent failure
        return NULL;
    }
    
    *outSize = 0;
    
    // Capture framebuffer
    int width, height, channels;
    unsigned char* framebuffer = captureFramebuffer( &width, &height, &channels );
    
    if ( !framebuffer ) {
        return NULL;
    }
    
    // Silent operation - no console output
    
    // Compress to JPEG using stb_image_write with callback (in-memory)
    // This avoids file system issues
    JpegWriteContext ctx;
    ctx.capacity = width * height * channels;  // Start with raw size estimate
    ctx.buffer = (unsigned char*)malloc( ctx.capacity );
    ctx.size = 0;
    
    if ( !ctx.buffer ) {
        // Silent failure
        free( framebuffer );
        return NULL;
    }
    
    // Use callback-based JPEG writing (in-memory)
    int success = stbi_write_jpg_to_func( jpegWriteCallback, &ctx, width, height, channels, framebuffer, quality );
    
    free( framebuffer );
    
    // Check for callback errors (size = -1 indicates realloc failure)
    if ( !success || ctx.size <= 0 ) {
        // Silent failure - free buffer on error
        if ( ctx.buffer ) {
            free( ctx.buffer );
        }
        return NULL;
    }
    
    *outSize = ctx.size;
    
    // Silent success - no console output
    
    return ctx.buffer;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
