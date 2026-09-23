#include <windows.h>
#include <cstdio>

// Run in a process of the target architecture. This checks the final DLLs,
// including their runtime dependencies and the engine's required exports.
int main(int argc, char **argv)
{
    if (argc != 4)
        return 2;
    for (int i = 1; i < argc; ++i) {
        HMODULE module = LoadLibraryA(argv[i]);
        if (!module) {
            std::fprintf(stderr, "LoadLibrary(%s) failed: %lu\n", argv[i], GetLastError());
            return 1;
        }
        bool valid = GetProcAddress(module, "dllEntry") && GetProcAddress(module, "vmMain");
        FreeLibrary(module);
        if (!valid) {
            std::fprintf(stderr, "Missing dllEntry/vmMain in %s\n", argv[i]);
            return 1;
        }
        std::printf("Loaded %s; dllEntry and vmMain present\n", argv[i]);
    }
    return 0;
}
