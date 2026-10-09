#ifndef BGAME_JXAC_MEMORY_RULES_H
#define BGAME_JXAC_MEMORY_RULES_H

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace jxac {
namespace memoryrules {

inline constexpr char CCHOOK_TABLES_V1[] = "cchook_tables_v1";

// Only identifiers belong in the server binary. Do not copy the detection
// bytes into qagame: a local client can share its process with that module.
inline const char* knownRuleLabel(const char* id) {
    return id && std::strcmp(id, CCHOOK_TABLES_V1) == 0
        ? "CCHookReloaded memory signature" : NULL;
}

#ifdef XMOD_JXAC_MEMORY_MATCHER

struct Pattern {
    const uint8_t* bytes;
    size_t size;
    bool executable;
};

struct Rule {
    const char* id;
    const char* label;
    const Pattern* patterns;
    size_t count;
};

inline constexpr size_t MAX_PATTERN_BYTES = 64;
inline constexpr size_t NOT_FOUND = static_cast<size_t>(-1);

// Source-derived, compound fingerprint for the public CCHookReloaded source
// a353dc19aaac8edca4e3381c6e16dda7d2bc3971 (2026-05-22).
// offsets.h stores each of these two independent 16-word engine tables in
// global SOffsets objects. RetSpoof.asm contains the third, fixed code body.
// ALL THREE must occur in the SAME allocation; the stub must occupy a code
// candidate accepted by the Windows scanner's protection/DEP checks.
// Individual tables, strings, hooks or private executable memory are not hits.
// Also verified in the supplied 1.4.0 x86 binary (MD5
// 9564691ac5d9f99e19619dfbe95e2cbd); none of these bytes overlap relocations.
// Other compiled variants may differ from this deliberately narrow rule.
inline constexpr uint8_t cchEt260Table[] = {
    0xc0, 0x8b, 0x3e, 0x01, 0xe2, 0x8a, 0x44, 0x00, 0x60, 0x41, 0x41, 0x00,
    0xf0, 0x29, 0x02, 0x01, 0x14, 0xfb, 0x68, 0x01, 0xd8, 0x59, 0x83, 0x00,
    0x88, 0xea, 0x3e, 0x01, 0x08, 0x6b, 0x5d, 0x01, 0x1c, 0x6b, 0x5d, 0x01,
    0x14, 0x6b, 0x5d, 0x01, 0xc4, 0xf7, 0x65, 0x01, 0x94, 0xaa, 0x9d, 0x00,
    0x84, 0xa6, 0x78, 0x01, 0x88, 0xa6, 0x78, 0x01, 0x20, 0x94, 0x3a, 0x01,
    0xff, 0xff, 0xff, 0xff
};
inline constexpr uint8_t cchEtSteamTable[] = {
    0xc0, 0xd1, 0x46, 0x01, 0x48, 0xbf, 0x43, 0x00, 0x40, 0x44, 0x41, 0x00,
    0x38, 0x53, 0xa7, 0x00, 0x88, 0xd2, 0x46, 0x01, 0x38, 0x4c, 0x89, 0x00,
    0x48, 0xb0, 0x60, 0x01, 0x68, 0xc5, 0x56, 0x01, 0x7c, 0xc5, 0x56, 0x01,
    0x74, 0xc5, 0x56, 0x01, 0x24, 0x52, 0x5f, 0x01, 0x64, 0x3b, 0xa4, 0x00,
    0xe4, 0xd7, 0x7f, 0x01, 0xe8, 0xd7, 0x7f, 0x01, 0x80, 0x65, 0x20, 0x01,
    0xff, 0xff, 0xff, 0xff
};
inline constexpr uint8_t cchCall12Stub[] = {
    0x8b, 0x04, 0x24, 0x89, 0x44, 0x24, 0x40, 0x89, 0x74, 0x24, 0x3c, 0x89,
    0x7c, 0x24, 0x38, 0x89, 0x0c, 0x24, 0xff, 0xe2
};
static_assert(sizeof(cchEt260Table) == 64 && sizeof(cchEtSteamTable) == 64,
              "CCH offset tables must contain all sixteen 32-bit words");
static_assert(sizeof(cchCall12Stub) == 20, "CCH fixed x86 stub length changed");

inline constexpr Pattern cchPatterns[] = {
    { cchEt260Table, sizeof(cchEt260Table), false },
    { cchEtSteamTable, sizeof(cchEtSteamTable), false },
    { cchCall12Stub, sizeof(cchCall12Stub), true }
};

inline constexpr Rule rules[] = {
    { CCHOOK_TABLES_V1, "CCHookReloaded memory signature",
      cchPatterns, sizeof(cchPatterns) / sizeof(cchPatterns[0]) }
};
inline constexpr size_t RULE_COUNT = sizeof(rules) / sizeof(rules[0]);

inline bool matches(const Pattern& pattern, const uint8_t* data,
                    size_t available, bool executable) {
    return data && pattern.size && pattern.size <= available &&
        (!pattern.executable || executable) &&
        std::memcmp(data, pattern.bytes, pattern.size) == 0;
}

inline size_t findPattern(const Pattern& pattern, const uint8_t* data,
                          size_t available, bool executable) {
    if (!data || !pattern.size || pattern.size > available ||
        (pattern.executable && !executable)) return NOT_FOUND;
    for (size_t i = 0; i <= available - pattern.size; ++i) {
        if (data[i] == pattern.bytes[0] &&
            std::memcmp(data + i, pattern.bytes, pattern.size) == 0) return i;
    }
    return NOT_FOUND;
}

#endif // XMOD_JXAC_MEMORY_MATCHER

} // namespace memoryrules
} // namespace jxac

#endif // BGAME_JXAC_MEMORY_RULES_H
