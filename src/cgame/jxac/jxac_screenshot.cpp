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
    // This function is deprecated - use compressRawToJpeg with trap_R_ReadPixels instead
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

unsigned char* Screenshot::captureAndCompress( int* outSize, int quality ) {
    if ( !outSize ) {
        return NULL;
    }
    
    *outSize = 0;
    
    // This function is deprecated - capture is now done in jxac_client.cpp
    // using trap_R_ReadPixels directly, then compressed via compressRawToJpeg
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

unsigned char* Screenshot::compressRawToJpeg( const unsigned char* rawData, int width, int height, 
                                               int channels, int quality, int* outSize ) {
    CG_Printf( "JXAC DEBUG: compressRawToJpeg() called - width=%d, height=%d, channels=%d, quality=%d\n",
              width, height, channels, quality );
    
    if ( !rawData || !outSize || width <= 0 || height <= 0 || channels < 3 ) {
        CG_Printf( "JXAC DEBUG: compressRawToJpeg() FAILED - invalid params (rawData=%p, width=%d, height=%d, channels=%d)\n",
                  (void*)rawData, width, height, channels );
        return NULL;
    }
    
    *outSize = 0;
    
    // Initialize output buffer context
    JpegWriteContext ctx;
    ctx.capacity = width * height;  // Start with reasonable estimate (JPEG is usually ~10% of raw)
    if ( ctx.capacity < 4096 ) ctx.capacity = 4096;  // Minimum 4KB
    ctx.buffer = (unsigned char*)malloc( ctx.capacity );
    ctx.size = 0;
    
    CG_Printf( "JXAC DEBUG: Allocated initial JPEG buffer: %d bytes\n", ctx.capacity );
    
    if ( !ctx.buffer ) {
        CG_Printf( "JXAC DEBUG: compressRawToJpeg() FAILED - malloc failed for %d bytes\n", ctx.capacity );
        return NULL;
    }
    
    CG_Printf( "JXAC DEBUG: Calling stbi_write_jpg_to_func()...\n" );
    
    // Use callback-based JPEG writing (in-memory)
    // stb_image_write expects RGB data (3 channels) or RGBA (4 channels)
    int success = stbi_write_jpg_to_func( jpegWriteCallback, &ctx, width, height, channels, rawData, quality );
    
    CG_Printf( "JXAC DEBUG: stbi_write_jpg_to_func() returned %d, ctx.size=%d\n", success, ctx.size );
    
    // Check for callback errors (size = -1 indicates realloc failure)
    if ( !success || ctx.size <= 0 ) {
        CG_Printf( "JXAC DEBUG: compressRawToJpeg() FAILED - stbi_write_jpg_to_func failed (success=%d, size=%d)\n",
                  success, ctx.size );
        if ( ctx.buffer ) {
            free( ctx.buffer );
        }
        return NULL;
    }
    
    CG_Printf( "JXAC DEBUG: compressRawToJpeg() SUCCESS - output size=%d bytes (compression ratio %.1f%%)\n",
              ctx.size, (ctx.size * 100.0f) / (width * height * channels) );
    
    *outSize = ctx.size;
    return ctx.buffer;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
