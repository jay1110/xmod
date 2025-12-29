#ifndef GAME_JXAC_SCREENSHOT_H
#define GAME_JXAC_SCREENSHOT_H

///////////////////////////////////////////////////////////////////////////////
// JXAC Screenshot Capture Module
// Handles screenshot capture and JPEG compression
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
    
    // Helper: Write JPEG data to file
    // Returns qtrue on success, qfalse on failure
    static qboolean writeToFile( const char* filename, const unsigned char* data, int size );
    
private:
    // Platform-specific framebuffer capture
    static unsigned char* captureFramebuffer( int* width, int* height, int* channels );
};

///////////////////////////////////////////////////////////////////////////////

} // namespace jxac

#endif // GAME_JXAC_SCREENSHOT_H
