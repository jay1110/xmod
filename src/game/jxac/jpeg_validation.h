#ifndef GAME_JXAC_JPEG_VALIDATION_H
#define GAME_JXAC_JPEG_VALIDATION_H

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace jxac {
namespace jpeg {

// Bounded structural validation of standalone 8-bit Huffman JPEG screenshots.
// Supports baseline and progressive scans, without allocating or decoding pixels.
// Passing this check does not attest that the picture is genuine or decodable.
inline bool valid(const void* input, size_t size) {
    const size_t maxBytes = 2u * 1024u * 1024u;
    const size_t maxPixels = (64u * 1024u * 1024u) / 3u;
    if (!input || size < 4 || size > maxBytes) return false;
    const uint8_t* data = static_cast<const uint8_t*>(input);
    if (data[0] != 0xff || data[1] != 0xd8) return false;

    struct Component { unsigned id, quant; } components[4] = {};
    unsigned count = 0, quantTables = 0, dcTables = 0, acTables = 0;
    unsigned dcComponents = 0, coveredComponents = 0, scans = 0, markers = 0;
    unsigned restartInterval = 0;
    bool frame = false, progressive = false;
    size_t pos = 2;

    while (pos < size && ++markers <= 4096) {
        if (data[pos++] != 0xff) return false;
        while (pos < size && data[pos] == 0xff) ++pos;
        if (pos == size) return false;
        const unsigned marker = data[pos++];
        if (marker == 0xd9) {
            return pos == size && frame && scans && coveredComponents == (1u << count) - 1u;
        }
        if (marker == 0 || marker == 0xd8 || marker == 1 ||
            (marker >= 0xd0 && marker <= 0xd7) || size - pos < 2) return false;
        const size_t length = (static_cast<size_t>(data[pos]) << 8) | data[pos + 1];
        if (length < 2 || length > size - pos) return false;
        pos += 2;
        const size_t end = pos + length - 2;

        if (marker == 0xdb) { // Quantization tables.
            if (pos == end) return false;
            while (pos < end) {
                const unsigned table = data[pos++];
                if ((table >> 4) > 1 || (table & 15) > 3) return false;
                const size_t bytes = (table >> 4) ? 128 : 64;
                if (bytes > end - pos) return false;
                pos += bytes;
                quantTables |= 1u << (table & 15);
            }
        } else if (marker == 0xc4) { // Huffman tables and symbol counts.
            if (pos == end) return false;
            while (pos < end) {
                if (end - pos < 17) return false;
                const unsigned table = data[pos++];
                if ((table >> 4) > 1 || (table & 15) > 3) return false;
                unsigned symbols = 0;
                int slots = 1;
                for (unsigned i = 0; i < 16; ++i) {
                    const unsigned n = data[pos++];
                    symbols += n;
                    slots = slots * 2 - static_cast<int>(n);
                    if (slots < 0) return false;
                }
                if (!symbols || symbols > 256 || symbols > end - pos) return false;
                pos += symbols;
                if (table >> 4) acTables |= 1u << (table & 15);
                else dcTables |= 1u << (table & 15);
            }
        } else if (marker == 0xc0 || marker == 0xc2) { // Baseline/progressive SOF.
            if (frame || end - pos < 6 || data[pos] != 8) return false;
            const unsigned height = (data[pos + 1] << 8) | data[pos + 2];
            const unsigned width = (data[pos + 3] << 8) | data[pos + 4];
            count = data[pos + 5];
            if (!width || !height || width > maxPixels / height || !count || count > 4 ||
                end - pos != 6u + 3u * count) return false;
            pos += 6;
            for (unsigned i = 0; i < count; ++i) {
                components[i] = Component{ data[pos], data[pos + 2] };
                const unsigned horizontal = data[pos + 1] >> 4;
                const unsigned vertical = data[pos + 1] & 15;
                if (!horizontal || horizontal > 4 || !vertical || vertical > 4 || components[i].quant > 3)
                    return false;
                for (unsigned j = 0; j < i; ++j)
                    if (components[j].id == components[i].id) return false;
                pos += 3;
            }
            progressive = marker == 0xc2;
            frame = true;
        } else if (marker == 0xdd) { // Restart interval.
            if (end - pos != 2) return false;
            restartInterval = (data[pos] << 8) | data[pos + 1];
            pos = end;
        } else if (marker == 0xda) { // Start of scan followed by entropy bytes.
            if (!frame || end - pos < 4 || ++scans > 256) return false;
            const unsigned n = data[pos++];
            if (!n || n > count || end - pos != 2u * n + 3u) return false;
            const unsigned first = data[end - 3], last = data[end - 2];
            const unsigned high = data[end - 1] >> 4, low = data[end - 1] & 15;
            if (!progressive) {
                if (first != 0 || last != 63 || high || low) return false;
            } else if (first > last || last > 63 || (first == 0 && last != 0) ||
                (first != 0 && n != 1) || high > 13 || low > 13 || (high && high != low + 1)) {
                return false;
            }
            unsigned selected = 0;
            for (unsigned i = 0; i < n; ++i) {
                const unsigned id = data[pos++], tables = data[pos++];
                unsigned component = 0;
                while (component < count && components[component].id != id) ++component;
                if (component == count || (selected & (1u << component)) ||
                    !(quantTables & (1u << components[component].quant)) ||
                    (tables >> 4) > 3 || (tables & 15) > 3) return false;
                selected |= 1u << component;
                if ((!progressive || (first == 0 && high == 0)) && !(dcTables & (1u << (tables >> 4)))) return false;
                if ((!progressive || first != 0) && !(acTables & (1u << (tables & 15)))) return false;
                if (progressive && first != 0 && !(dcComponents & (1u << component))) return false;
            }
            if (progressive && first == 0 && high == 0) dcComponents |= selected;
            coveredComponents |= selected;
            pos = end;
            size_t entropyBytes = 0;
            unsigned nextRestart = 0;
            while (pos < size) {
                const uint8_t* next = static_cast<const uint8_t*>(std::memchr(data + pos, 0xff, size - pos));
                if (!next) return false; // Every complete scan must reach another marker.
                entropyBytes += static_cast<size_t>(next - (data + pos));
                pos = static_cast<size_t>(next - data);
                const size_t markerStart = pos++;
                while (pos < size && data[pos] == 0xff) ++pos;
                if (pos == size) return false;
                const unsigned code = data[pos++];
                if (code == 0) { ++entropyBytes; continue; } // Stuffed FF sample byte.
                if (code >= 0xd0 && code <= 0xd7) {
                    if (!restartInterval || !entropyBytes || code != 0xd0u + nextRestart) return false;
                    nextRestart = (nextRestart + 1u) & 7u;
                    continue;
                }
                if (!entropyBytes) return false;
                pos = markerStart;
                break;
            }
        } else if ((marker >= 0xe0 && marker <= 0xef) || marker == 0xfe) {
            pos = end; // APP/COM metadata; internal bytes are not outer markers.
        } else {
            return false; // Unsupported coding processes are not screenshot evidence.
        }
    }
    return false;
}

} // namespace jpeg
} // namespace jxac

#endif // GAME_JXAC_JPEG_VALIDATION_H
