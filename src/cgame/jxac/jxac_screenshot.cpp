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
        ctx->capacity = ctx->capacity * 2;
        ctx->buffer = (unsigned char*)realloc( ctx->buffer, ctx->capacity );
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
        Com_Printf( "JXAC Screenshot: Failed to allocate framebuffer buffer\n" );
        return NULL;
    }
    
    // Read framebuffer pixels (OpenGL style - bottom to top)
    // This would normally use glReadPixels or equivalent
    // For now, create a placeholder pattern for testing
    // TODO: Replace with actual framebuffer capture
    for ( int i = 0; i < bufferSize; i += 3 ) {
        // Create a simple gradient pattern for testing
        int pixel = i / 3;
        int x = pixel % (*width);
        int y = pixel / (*width);
        
        buffer[i + 0] = (unsigned char)((x * 255) / (*width));      // R
        buffer[i + 1] = (unsigned char)((y * 255) / (*height));     // G
        buffer[i + 2] = (unsigned char)(((x + y) * 128) / ((*width) + (*height))); // B
    }
    
    // Note: In a real implementation, you would use:
    // trap_R_ReadPixels( 0, 0, *width, *height, buffer );
    // or similar engine function to read the actual framebuffer
    
    return buffer;
}

///////////////////////////////////////////////////////////////////////////////

unsigned char* Screenshot::captureAndCompress( int* outSize, int quality ) {
    if ( !outSize ) {
        Com_Printf( "JXAC Screenshot: Invalid output size pointer\n" );
        return NULL;
    }
    
    *outSize = 0;
    
    // Capture framebuffer
    int width, height, channels;
    unsigned char* framebuffer = captureFramebuffer( &width, &height, &channels );
    
    if ( !framebuffer ) {
        return NULL;
    }
    
    Com_Printf( "JXAC Screenshot: Captured framebuffer %dx%d (%d channels)\n", 
                width, height, channels );
    
    // Compress to JPEG using stb_image_write with callback (in-memory)
    // This avoids file system issues
    JpegWriteContext ctx;
    ctx.capacity = width * height * channels;  // Start with raw size estimate
    ctx.buffer = (unsigned char*)malloc( ctx.capacity );
    ctx.size = 0;
    
    if ( !ctx.buffer ) {
        Com_Printf( "JXAC Screenshot: Failed to allocate JPEG buffer\n" );
        free( framebuffer );
        return NULL;
    }
    
    // Use callback-based JPEG writing (in-memory)
    int success = stbi_write_jpg_to_func( jpegWriteCallback, &ctx, width, height, channels, framebuffer, quality );
    
    free( framebuffer );
    
    if ( !success || ctx.size <= 0 ) {
        Com_Printf( "JXAC Screenshot: Failed to compress to JPEG\n" );
        free( ctx.buffer );
        return NULL;
    }
    
    *outSize = ctx.size;
    
    Com_Printf( "JXAC Screenshot: Compressed to JPEG (%d bytes, quality %d)\n", 
                ctx.size, quality );
    
    return ctx.buffer;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac
