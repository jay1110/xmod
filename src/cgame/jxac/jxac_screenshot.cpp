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
    // This function is no longer used - we use file-based capture instead
    // See captureAndCompress() for the new implementation
    return NULL;
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
