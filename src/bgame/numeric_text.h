#ifndef BGAME_NUMERIC_TEXT_H
#define BGAME_NUMERIC_TEXT_H

#include <cerrno>
#include <cstdlib>

// Validate text before converting: -ffast-math can optimize away isfinite(),
// including bit-based checks of a value already treated as floating point.
inline bool XmodParseFiniteDecimal(const char* text, double& value) {
    if (!text || !*text) return false;
    const char* p = text;
    if (*p == '+' || *p == '-') ++p;
    bool digits = false;
    while (*p >= '0' && *p <= '9') { digits = true; ++p; }
    if (*p == '.') {
        ++p;
        while (*p >= '0' && *p <= '9') { digits = true; ++p; }
    }
    if (!digits) return false;
    if (*p == 'e' || *p == 'E') {
        ++p;
        if (*p == '+' || *p == '-') ++p;
        const char* exponent = p;
        while (*p >= '0' && *p <= '9') ++p;
        if (p == exponent) return false;
    }
    if (*p) return false;

    errno = 0;
    char* end;
    const double parsed = std::strtod(text, &end);
    // Reject overflow AND underflow before using a possibly non-finite result.
    if (errno == ERANGE || end != p) return false;
    value = parsed;
    return true;
}

#endif
