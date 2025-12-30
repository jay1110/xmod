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
    // Get current screen dimensions
    *width = cgs.glconfig.vidWidth;
    *height = cgs.glconfig.vidHeight;
    *channels = 3; // RGB
    
    // Allocate buffer for framebuffer data
    int bufferSize = (*width) * (*height) * (*channels);
    unsigned char* buffer = (unsigned char*)malloc( bufferSize );
    
    if ( !buffer ) {
        // Silent failure
        return NULL;
    }
    
    // Read framebuffer pixels (OpenGL style - bottom to top)
    // NOTE: This requires engine API support which is not currently available
    // The engine would need to expose a trap_R_ReadPixels() function
    // For now, create a placeholder pattern for testing
    // TODO: Once engine adds trap_R_ReadPixels, replace with:
    // trap_R_ReadPixels( 0, 0, *width, *height, buffer );
    // 
    // Then flip the image vertically (OpenGL reads bottom-to-top, JPEG expects top-to-bottom):
    // int rowSize = (*width) * (*channels);
    // unsigned char* tempRow = (unsigned char*)malloc( rowSize );
    // if ( tempRow ) {
    //     for ( int y = 0; y < (*height) / 2; y++ ) {
    //         unsigned char* row1 = buffer + (y * rowSize);
    //         unsigned char* row2 = buffer + (((*height) - 1 - y) * rowSize);
    //         memcpy( tempRow, row1, rowSize );
    //         memcpy( row1, row2, rowSize );
    //         memcpy( row2, tempRow, rowSize );
    //     }
    //     free( tempRow );
    // }
    
    for ( int i = 0; i < bufferSize; i += 3 ) {
        // Create a simple gradient pattern for testing
        int pixel = i / 3;
        int x = pixel % (*width);
        int y = pixel / (*width);
        
        buffer[i + 0] = (unsigned char)((x * 255) / (*width));      // R
        buffer[i + 1] = (unsigned char)((y * 255) / (*height));     // G
        buffer[i + 2] = (unsigned char)(((x + y) * 128) / ((*width) + (*height))); // B
    }
    
    return buffer;
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
