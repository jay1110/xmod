#ifndef CGAME_JXAC_SCREENSHOT_H
#define CGAME_JXAC_SCREENSHOT_H

///////////////////////////////////////////////////////////////////////////////
// JXAC Client Screenshot Capture Module
// Handles screenshot capture and JPEG compression on client
///////////////////////////////////////////////////////////////////////////////

namespace jxac {

///////////////////////////////////////////////////////////////////////////////

class Screenshot {
public:
    // Capture screenshot from framebuffer and compress to JPEG
    // Returns pointer to JPEG data (caller must free) and size in outSize
    // quality: 1-100 (85 is recommended)
    // Returns NULL on failure
    static unsigned char* captureAndCompress( int* outSize, int quality = 85 );
    
    // Compress raw RGB/RGBA buffer to JPEG
    // Returns pointer to JPEG data (caller must free) and size in outSize
    // rawData: pointer to raw pixel data (RGB or RGBA)
    // width/height: image dimensions
    // channels: 3 for RGB, 4 for RGBA
    // quality: 1-100 (85 is recommended)
    // Returns NULL on failure
    static unsigned char* compressRawToJpeg( const unsigned char* rawData, int width, int height, 
                                              int channels, int quality, int* outSize );
    
private:
    // Platform-specific framebuffer capture
    static unsigned char* captureFramebuffer( int* width, int* height, int* channels );
};

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac

#endif // CGAME_JXAC_SCREENSHOT_H
