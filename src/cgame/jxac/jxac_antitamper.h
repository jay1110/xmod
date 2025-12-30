#ifndef CGAME_JXAC_ANTITAMPER_H
#define CGAME_JXAC_ANTITAMPER_H

namespace jxac {

// Anti-tamper detection system
class AntiTamper {
public:
    // Initialize anti-tamper system
    static void init();
    
    // Check for tampering (called periodically)
    static void check();
    
    // Report tampering detected
    static void reportTamper( const char* details );
    
private:
    // Check code integrity
    static bool checkCodeIntegrity();
    
    // Check for debugger attachment
    static bool checkDebugger();
    
    // Check for known tamper tools
    static bool checkTamperTools();
    
    // Verify critical functions haven't been hooked
    static bool checkFunctionHooks();
};

} // namespace jxac

#endif // CGAME_JXAC_ANTITAMPER_H
