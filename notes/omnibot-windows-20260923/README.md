# Windows Omni-Bot: Untersuchung vom 23.09.2026

Ausgangsstand der Untersuchung: xmod 196b652. Die folgende Analyse wurde vor der unten dokumentierten Behebung erstellt.

## Befund

Die Release-Workflows bauen qagame für Windows x86 und x64 mit MinGW:
- .github/workflows/build-release.yml:80,83
- .github/workflows/build-multiplatform.yml:89,92
- make/platform/mingw und make/platform/mingw64

Die Omni-Bot-Anbindung fehlt nicht im Build. Der Start erfolgt in
src/game/g_main.cpp:748. In src/omnibot/et/g_etbot_interface.cpp:5007
wird ein ETInterface erzeugt und über pfnInitialize an die Bot-DLL übergeben
(src/omnibot/common/BotLoadLibrary.cpp:291).
Diese Grenze enthält virtuelle C++-Methoden, unter anderem Rückgaben vom Typ
GameEntity (IEngineInterface.h:197,202). Ein vorhandener C-Export und eine
passende Interface-Version garantieren deshalb keine Compiler-ABI-Kompatibilität.

## Direkt geprüfter ABI-Unterschied

probe.cpp enthält die unveränderte GameEntity-Klassendefinition aus
src/omnibot/common/Omni-Bot_Types.h sowie einen minimalen virtuellen Aufruf
GetLocalGameEntity().AsInt(). Es ist ein isolierter Compilervergleich,
kein Server- oder Vollintegrationstest.

Kompiliert mit MSVC 19.44 (Toolset 14.44.35207) und MSYS2 GCC 15.2.0,
jeweils für x86 und x64. Alle vier Kompilierungen erfolgreich.

- MinGW x86/x64 erwartet die vier Byte Entity-Daten direkt in EAX.
- MSVC x86 übergibt zusätzlich die Adresse eines Rückgabepuffers auf dem Stack.
- MSVC x64 übergibt zusätzlich die Adresse eines Rückgabepuffers in RDX.
- MSVC dereferenziert nach dem Aufruf EAX/RAX als Adresse des Ergebnisses.

Die vier beigefügten ASM-Dateien dokumentieren diesen Unterschied. Bei einem
MSVC-Bot-Aufruf eines MinGW-Callbacks interpretiert der Aufrufer dadurch einen
Entity-Wert als Adresse; auf x86 stimmen zudem die Stack-Konventionen nicht
überein. Diese Kombination ist für die geprüften Methoden inkompatibel.
Statisches Linken von libstdc++ behebt diesen Unterschied nicht.

## Weitere Ladebedingungen

Der Windows-Loader erwartet omnibot_et.dll (x86) beziehungsweise
omnibot_et_x64.dll (x64), siehe BotLoadLibrary.cpp:235-245.
Er sucht im omnibot_path, dann in ./omni-bot und zuletzt über die Windows-
DLL-Suche. omnibot_path ist standardmäßig leer. Ein leeres, aber nicht null
Pfadargument erzeugt beim ersten Versuch einen Pfad mit führendem Backslash;
die nachfolgenden Suchversuche bleiben erhalten.

omnibot_enable ist beim Start zu aktivieren. Engine, qagame und Omni-Bot-DLL
müssen dieselbe Architektur verwenden. Die angeforderte ET-Schnittstellenversion
ist 17 (ET_VERSION_0_8).

## Schlussfolgerung und Abhilfe

Für eine MSVC-kompilierte Omni-Bot-DLL ist der MinGW-Build von qagame ein
konkret nachgewiesenes Kompatibilitätsproblem. Empfohlene Abhilfe: Windows-
qagame mit einer passenden MSVC-Toolchain bauen und mit der tatsächlich
verwendeten Omni-Bot-DLL prüfen; alternativ beide Seiten mit einer kompatiblen
Toolchain bauen. Ein Umbenennen der DLL löst den ABI-Unterschied nicht.

Die vorhandene game.vcxproj enthält Win32- und x64-Konfigurationen sowie
beide Omni-Bot-Quelldateien. Die Release-Workflows verwenden dieses Projekt
jedoch nicht. Seine vollständige Buildfähigkeit wurde hier nicht geprüft.

Grenze des Befunds: Im untersuchten xmod-Arbeitsordner lagen keine betroffenen
Windows-Binaries und keine Serverlogs. Deshalb ist nicht belegt, welche
Bot-DLL der Nutzer tatsächlich lädt und ob der konkrete Fehler bereits beim
Laden oder erst beim Callback eintritt. Es wurde keine vollständige
Windows-Server-Reproduktion durchgeführt.

Hintergrund zur Unterscheidung der Compiler-ABIs:
https://clang.llvm.org/docs/MSVCCompatibility.html

## Umgesetzte Behebung

- build-release.yml und build-multiplatform.yml bauen Windows mit MSVC auf
  windows-2022, jeweils Win32 und x64. Artefaktnamen und DLL-Dateinamen bleiben
  gleich; Uploadpfade zeigen auf die neuen nativen Builds.
- Der neue CMake-Build übernimmt die Quellen aus src/*.defs und baut sämtliche
  drei Module inklusive Lua (wie zuvor als C++), SQLite und Omni-Bot.
- Korrigiert wurden MSVC-Buildhindernisse: doppelte size_t/uint64-Überladung
  unter Windows x64, veralteter rint-Ersatz sowie std::byte-Namenskonflikt im
  x86-Inline-Assembler. Die Bedingungen für Linux bleiben unverändert.
- Der eigenständige Omni-Bot-Loader erhält keinen erzwungenen Game-PCH,
  weil dieser unter MSVC seine CRT-Formatierungsfunktionen per Makro sperrt.
- Ein nativer CTest lädt die drei fertigen DLLs und prüft dllEntry/vmMain.
  Er läuft im Workflow für beide Architekturen. Release-Pakete werden bei
  fehlgeschlagenen Plattform-Builds nicht mehr veröffentlicht.

### Validierung

MSVC 19.39.33523.0 / Windows SDK 10.0.26100.0:
- Release Win32: alle drei DLLs erfolgreich gebaut; CTest 1/1 bestanden.
- Release x64: alle drei DLLs erfolgreich gebaut; CTest 1/1 bestanden.
- Beide Workflow-YAML-Dateien geparst; alle sechs Uploadpfade existieren.
- Linux-32-/64-Bit-Jobdefinitionen beider Workflows gegen HEAD verglichen:
  vollständig identisch. Linux wurde lokal nicht erneut gebaut.
- git diff --check bestanden.

Builds: build/msvc-x86 und build/msvc-x64.
Logs: build/msvc-x86-build.log, build/msvc-x64-build.log,
build/msvc-x86-test.log und build/msvc-x64-test.log.
DLLs jeweils in <Buildverzeichnis>/<game|cgame|ui>/Release/.

Die Ladeprüfung ist kein vollständiger Omni-Bot-Spieltest. Dafür müssen die
resultierenden qagame-DLLs mit der tatsächlich eingesetzten Omni-Bot-Version
auf dem Server gestartet und Bots hinzugefügt werden. Ein GitHub-Actions-Lauf
wurde nicht ausgelöst; sämtliche Änderungen bleiben lokal und ungepusht.
