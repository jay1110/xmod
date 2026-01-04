# Copilot Instructions für xmod

## Projekt-Übersicht

xmod ist ein Fork von jaymod für Enemy Territory. Das Projekt unterstützt **mehrere Plattformen und Architekturen**, was bei jeder Code-Änderung berücksichtigt werden muss.

## Unterstützte Build-Targets

### Architekturen
- **32-bit (x86)**
- **64-bit (x86_64/AMD64)**

### Betriebssysteme
- **Windows** (32-bit und 64-bit)
- **Linux** (32-bit und 64-bit)

### Module
- **cgame** - Client-seitiges Spielmodul (läuft auf dem Spieler-PC)
- **ui** - User Interface Modul (läuft auf dem Spieler-PC)
- **qagame** - Server-seitiges Spielmodul (läuft auf dem Server)

## Wichtige Regeln für Cross-Platform Kompatibilität

### 1. Header-Includes

**FALSCH:**
```cpp
#include <unistd.h>  // Existiert nicht auf Windows!
```

**RICHTIG:**
```cpp
#ifdef _WIN32
#include <process.h>  // Windows: getpid(), etc.
#include <windows.h>  // Windows API
#else
#include <unistd.h>   // Linux/Unix: getpid(), close(), etc.
#endif
```

### 2. Datentypen für Pointer und Größen

**FALSCH:**
```cpp
int ptr = (int)somePointer;  // Bricht auf 64-bit!
unsigned long size;          // Unterschiedliche Größe auf Windows vs Linux
```

**RICHTIG:**
```cpp
#include <stdint.h>
intptr_t ptr = (intptr_t)somePointer;  // Pointer-sichere Ganzzahl
size_t size;                            // Plattform-unabhängige Größe
```

### 3. Pfad-Trennzeichen

**FALSCH:**
```cpp
const char* path = "config\\settings.cfg";  // Nur Windows
```

**RICHTIG:**
```cpp
#ifdef _WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif
// Oder besser: Verwende "/" überall, da ET-Engine das akzeptiert
```

### 4. Netzwerk-Code

```cpp
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
// closesocket() statt close()
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// close() für Sockets
#endif
```

### 5. Struct Alignment

Bei Strukturen die zwischen Client und Server ausgetauscht werden:
```cpp
#pragma pack(push, 1)  // Konsistentes Alignment
typedef struct {
    int32_t value;     // Explizite Größe
    // ...
} networkStruct_t;
#pragma pack(pop)
```

### 6. Printf Format-Spezifizierer

**FALSCH:**
```cpp
printf("%d", sizeof(buffer));     // size_t ist nicht int
printf("%ld", (long)timeValue);   // Inkonsistent
```

**RICHTIG:**
```cpp
printf("%zu", sizeof(buffer));              // size_t
printf("%" PRId64, (int64_t)timeValue);     // 64-bit Integer
printf("%d", (int)smallValue);              // Expliziter Cast wenn sicher
```

## Verzeichnisstruktur

```
src/
├── bgame/      # Shared code zwischen Client und Server
├── cgame/      # Client-seitiger Code
│   ├── linux/  # Linux-spezifischer Client-Code
│   └── win32/  # Windows-spezifischer Client-Code
├── game/       # Server-seitiger Code
└── ui/         # UI Code
```

## Checkliste für Code-Reviews

Bei jeder Änderung prüfen:

- [ ] Kompiliert auf Windows 32-bit?
- [ ] Kompiliert auf Windows 64-bit?
- [ ] Kompiliert auf Linux 32-bit?
- [ ] Kompiliert auf Linux 64-bit?
- [ ] Sind alle Platform-spezifischen Includes korrekt mit #ifdef geschützt?
- [ ] Werden intptr_t/size_t statt int/long für Pointer-Arithmetik verwendet?
- [ ] Funktioniert die Client-Server Kommunikation zwischen verschiedenen Architekturen?

## Bekannte Fallstricke

1. **long Typ**: Auf Windows ist long immer 32-bit, auf Linux 64-bit ist long 64-bit
2. **time_t**: Kann 32-bit oder 64-bit sein, verwende explizite Casts
3. **Winsock vs BSD Sockets**: Komplett unterschiedliche APIs für Initialisierung und Cleanup
4. **DLL vs SO**: Windows verwendet .dll, Linux verwendet .so - beachte bei dynamischem Laden

## SQLite Datenbank

Die xmod.db Datenbank wird im Mod-Verzeichnis gespeichert und muss von 32-bit und 64-bit Servern/Clients lesbar sein. SQLite handhabt dies automatisch, aber:
- Keine plattformspezifischen Datentypen in der DB speichern
- Timestamps als Unix-Zeit (Integer) speichern