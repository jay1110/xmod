#ifndef XMOD_FORCED_CVARS_H
#define XMOD_FORCED_CVARS_H

#include <map>
#include <string>
#include <cstdlib>
#include <cmath>
#include <bgame/numeric_text.h>

// Shared validation keeps configuration files and incoming rules consistent.
inline bool XmodCvarNameValid(const std::string& name) {
    if (name.empty() || name.size() >= 64) return false;
    for (unsigned char c : name)
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '.')) return false;
    return true;
}

inline bool XmodCvarNumber(const std::string& text, double& value) {
    return XmodParseFiniteDecimal(text.c_str(), value);
}

class XmodForcedCvars {
    struct Rule { std::string value, minimum, maximum; bool range; };
    std::map<std::string, Rule> rules;
public:
    void clear() { rules.clear(); }
    bool set(std::string name, const std::string& value, const std::string& maximum = "", bool range = false) {
        if (!XmodCvarNameValid(name) || value.size() >= 128 || maximum.size() >= 128) return false;
        for (char& c : name) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
        if (rules.size() >= 256 && rules.find(name) == rules.end()) return false;
        if (range) {
            double lo, hi;
            if (!XmodCvarNumber(value, lo) || !XmodCvarNumber(maximum, hi) || lo > hi) return false;
        }
        Rule rule = {value, value, maximum, range};
        rules[name] = rule;
        return true;
    }
    template<class Read, class Write>
    void enforce(Read read, Write write) const {
        for (const auto& item : rules) {
            const Rule& rule = item.second;
            const std::string current = read(item.first.c_str());
            if (!rule.range) {
                if (current != rule.value) write(item.first.c_str(), rule.value.c_str());
            } else {
                double value, lo, hi;
                XmodCvarNumber(rule.minimum, lo);
                XmodCvarNumber(rule.maximum, hi);
                if (!XmodCvarNumber(current, value) || value < lo)
                    write(item.first.c_str(), rule.minimum.c_str());
                else if (value > hi)
                    write(item.first.c_str(), rule.maximum.c_str());
            }
        }
    }
};

#endif
