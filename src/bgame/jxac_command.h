#ifndef XMOD_JXAC_COMMAND_H
#define XMOD_JXAC_COMMAND_H

// ETLegacy sanitizes each game-command argument to 255 characters before
// dispatching it to qagame. Keep hex data in one compatible, shorter argument.
enum {
    XMOD_JXAC_COMMAND_CHUNK_BYTES = 127,
    XMOD_JXAC_COMMAND_HEX_SIZE = XMOD_JXAC_COMMAND_CHUNK_BYTES * 2 + 1,
    // Preserve the previous 300 * 450-byte UDP screenshot capacity.
    XMOD_JXAC_UDP_MAX_BYTES = 135000,
    XMOD_JXAC_UDP_QUEUE_CAPACITY =
        (XMOD_JXAC_UDP_MAX_BYTES + XMOD_JXAC_COMMAND_CHUNK_BYTES - 1) /
        XMOD_JXAC_COMMAND_CHUNK_BYTES
};

inline bool XmodJxacEncodeScreenshotChunk(const unsigned char* data, int size,
                                         char* output, int outputSize) {
    if (!data || !output || size <= 0 || size > XMOD_JXAC_COMMAND_CHUNK_BYTES ||
        outputSize < size * 2 + 1) return false;
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < size; ++i) {
        output[i * 2] = hex[data[i] >> 4];
        output[i * 2 + 1] = hex[data[i] & 15];
    }
    output[size * 2] = '\0';
    return true;
}

#endif
