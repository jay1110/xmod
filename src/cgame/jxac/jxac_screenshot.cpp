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
            ctx->size = -1;
            return;
        }
        ctx->buffer = newBuffer;
        ctx->capacity = newCapacity;
    }
    
    memcpy( ctx->buffer + ctx->size, data, size );
    ctx->size += size;
}

///////////////////////////////////////////////////////////////////////////////

unsigned char* Screenshot::captureFramebuffer( int* width, int* height, int* channels ) {
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

unsigned char* Screenshot::captureAndCompress( int* outSize, int quality ) {
    if ( !outSize ) {
        return NULL;
    }
    *outSize = 0;
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

unsigned char* Screenshot::compressRawToJpeg( const unsigned char* rawData, int width, int height, 
                                               int channels, int quality, int* outSize ) {
    if ( !rawData || !outSize || width <= 0 || height <= 0 || channels < 3 ) {
        return NULL;
    }
    
    *outSize = 0;
    
    // Initialize output buffer context
    JpegWriteContext ctx;
    ctx.capacity = width * height;
    if ( ctx.capacity < 4096 ) ctx.capacity = 4096;
    ctx.buffer = (unsigned char*)malloc( ctx.capacity );
    ctx.size = 0;
    
    if ( !ctx.buffer ) {
        CG_Printf("^1JXAC ERROR: malloc failed for JPEG buffer\n");
        return NULL;
    }
    
    // Use callback-based JPEG writing (in-memory)
    int success = stbi_write_jpg_to_func( jpegWriteCallback, &ctx, width, height, channels, rawData, quality );
    
    if ( !success || ctx.size <= 0 ) {
        CG_Printf("^1JXAC ERROR: JPEG compression failed\n");
        if ( ctx.buffer ) {
            free( ctx.buffer );
        }
        return NULL;
    }
    
    *outSize = ctx.size;
    return ctx.buffer;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
