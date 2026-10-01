#include <bgame/impl.h>
#include <bgame/forced_cvars.h>

static XmodForcedCvars forcedCvars;

void CG_ClearForcedCvars() {
    forcedCvars.clear();
}

void CG_UpdateForcedCvars() {
    forcedCvars.enforce([](const char* name) {
        char value[MAX_STRING_CHARS];
        trap_Cvar_VariableStringBuffer(name, value, sizeof(value));
        return std::string(value);
    }, [](const char* name, const char* value) { trap_Cvar_Set(name, value); });
}

void CG_ReceiveForcedCvar(bool range) {
    // CG_Argv uses one static buffer, so copy each argument before reading another.
    char name[64], value[128], maximum[128];
    if (trap_Argc() != (range ? 4 : 3)) return;
    trap_Argv(1, name, sizeof(name));
    trap_Argv(2, value, sizeof(value));
    trap_Argv(3, maximum, sizeof(maximum));
    if (forcedCvars.set(name, value, maximum, range)) CG_UpdateForcedCvars();
}
