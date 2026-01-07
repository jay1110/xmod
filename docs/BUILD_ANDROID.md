# Kompilieren von xmod für Android

Diese Anleitung beschreibt, wie Sie die Android-Binaries von xmod kompilieren können. Die Anleitung richtet sich an Entwickler, die unter **Ubuntu 24.04** mit **CLion** als IDE arbeiten.

## Inhaltsverzeichnis

- [Übersicht](#übersicht)
- [Voraussetzungen](#voraussetzungen)
- [Android NDK Installation](#android-ndk-installation)
- [Umgebungsvariablen konfigurieren](#umgebungsvariablen-konfigurieren)
- [Build-Befehle](#build-befehle)
- [CLion IDE Konfiguration](#clion-ide-konfiguration)
- [Output-Dateien](#output-dateien)
- [Troubleshooting](#troubleshooting)

## Übersicht

Das xmod Build-System unterstützt die Kompilierung von Android-Binaries für Enemy Territory Android-Clients. Es werden drei Android-Architekturen unterstützt:

- **ARM64-v8a** (64-bit ARM) - Die primäre Architektur für moderne Android-Geräte
- **x86_64** (64-bit Intel/AMD) - Für Android-Emulatoren und x86-basierte Geräte
- **x86** (32-bit Intel/AMD) - Für ältere x86-basierte Android-Geräte

Für jede Architektur werden drei Shared Libraries (.so Dateien) gebaut:
- **libqagame** - Server-seitiges Spielmodul
- **libcgame** - Client-seitiges Spielmodul
- **libui** - User Interface Modul

Das Build-System ist bereits vollständig konfiguriert und erfordert nur die Installation des Android NDK.

## Voraussetzungen

### Betriebssystem

- **Ubuntu 24.04 LTS** (empfohlen)
- Andere Linux-Distributionen funktionieren ebenfalls, benötigen aber möglicherweise angepasste Paket-Installationsbefehle

### Erforderliche Pakete

Installieren Sie die notwendigen Build-Tools:

```bash
sudo apt update
sudo apt install build-essential m4 python3 git wget unzip
```

**Paket-Beschreibungen:**
- `build-essential` - GCC/G++ Compiler und Make
- `m4` - Makro-Prozessor (vom Build-System verwendet)
- `python3` - Python 3.x für Build-Skripte
- `git` - Versionskontrolle
- `wget` - Download-Tool für NDK
- `unzip` - Zum Entpacken des NDK-Archivs

### Android NDK

Das **Android NDK (Native Development Kit)** ist erforderlich, um native C/C++ Code für Android zu kompilieren.

**Empfohlene Version:** NDK r26d oder neuer

## Android NDK Installation

### Schritt 1: Verzeichnisstruktur erstellen

Erstellen Sie das Standard-Verzeichnis für das Android SDK:

```bash
mkdir -p ~/Android/Sdk/ndk
cd ~/Android/Sdk/ndk
```

### Schritt 2: NDK herunterladen

Laden Sie das Android NDK r26d herunter:

```bash
wget https://dl.google.com/android/repository/android-ndk-r26d-linux.zip
```

**Hinweis:** Die Download-Größe beträgt ca. 1 GB. Der Download kann einige Minuten dauern.

### Schritt 3: NDK entpacken

```bash
unzip android-ndk-r26d-linux.zip
```

Dies erstellt das Verzeichnis `android-ndk-r26d` mit der folgenden Struktur:

```
~/Android/Sdk/ndk/android-ndk-r26d/
├── toolchains/
│   └── llvm/
│       └── prebuilt/
│           └── linux-x86_64/
│               ├── bin/           # Compiler und Tools
│               ├── sysroot/       # Android Headers und Libraries
│               └── ...
├── sources/
├── build/
└── ...
```

### Schritt 4: ZIP-Datei löschen (optional)

```bash
rm android-ndk-r26d-linux.zip
```

### Schritt 5: Installation überprüfen

Überprüfen Sie, ob das NDK korrekt installiert ist:

```bash
ls -la ~/Android/Sdk/ndk/android-ndk-r26d/toolchains/llvm/prebuilt/linux-x86_64/bin/
```

Sie sollten verschiedene Compiler-Binaries wie `aarch64-linux-android*-clang++` sehen.

## Umgebungsvariablen konfigurieren

Das Build-System benötigt die Umgebungsvariablen `ANDROID_NDK_HOME` und `NDK_ROOT`, um den NDK-Pfad zu finden.

### Option 1: Permanente Konfiguration in ~/.bashrc

Öffnen Sie `~/.bashrc` in einem Editor:

```bash
nano ~/.bashrc
```

Fügen Sie am Ende der Datei folgende Zeilen hinzu:

```bash
# Android NDK
export ANDROID_NDK_HOME="$HOME/Android/Sdk/ndk/android-ndk-r26d"
export NDK_ROOT="$ANDROID_NDK_HOME"
```

Speichern Sie die Datei und laden Sie die Konfiguration neu:

```bash
source ~/.bashrc
```

### Option 2: Permanente Konfiguration in ~/.profile

Alternativ können Sie die Variablen in `~/.profile` setzen (gilt für alle Shells):

```bash
nano ~/.profile
```

Fügen Sie hinzu:

```bash
# Android NDK
export ANDROID_NDK_HOME="$HOME/Android/Sdk/ndk/android-ndk-r26d"
export NDK_ROOT="$ANDROID_NDK_HOME"
```

Speichern und neu laden:

```bash
source ~/.profile
```

### Konfiguration überprüfen

Überprüfen Sie, ob die Variablen korrekt gesetzt sind:

```bash
echo $ANDROID_NDK_HOME
echo $NDK_ROOT
```

Beide Befehle sollten `/home/<username>/Android/Sdk/ndk/android-ndk-r26d` ausgeben.

## Build-Befehle

### Übersicht: Android-Plattformen

Das xmod Build-System unterstützt folgende Android-Plattformen:

| PLATFORM | Architektur | Output-Datei (Beispiel: qagame) |
|----------|-------------|--------------------------------|
| `android-arm64` | ARM64-v8a (64-bit) | `libqagame.mp.android.arm64-v8a.so` |
| `android-x86_64` | x86_64 (64-bit) | `libqagame.mp.android.x86_64.so` |
| `android-x86` | x86 (32-bit) | `libqagame.mp.android.i386.so` |

### Build für ARM64 (empfohlen für moderne Geräte)

Wechseln Sie in das Repository-Verzeichnis:

```bash
cd /pfad/zu/xmod
```

**Release Build:**
```bash
PLATFORM=android-arm64 make release
```

**Debug Build:**
```bash
PLATFORM=android-arm64 make debug
```

**Clean Build:**
```bash
PLATFORM=android-arm64 make clean
```

### Build für x86_64

**Release Build:**
```bash
PLATFORM=android-x86_64 make release
```

**Debug Build:**
```bash
PLATFORM=android-x86_64 make debug
```

### Build für x86 (32-bit)

**Release Build:**
```bash
PLATFORM=android-x86 make release
```

**Debug Build:**
```bash
PLATFORM=android-x86 make debug
```

### Alle Android-Plattformen bauen

Um alle drei Android-Architekturen zu kompilieren:

```bash
# Release Builds
PLATFORM=android-arm64 make release
PLATFORM=android-x86_64 make release
PLATFORM=android-x86 make release
```

### Parallele Kompilierung (schneller)

Nutzen Sie die `-j` Option, um mehrere CPU-Kerne zu verwenden:

```bash
# Nutzt alle verfügbaren CPU-Kerne
PLATFORM=android-arm64 make release -j$(nproc)

# Nutzt 4 CPU-Kerne
PLATFORM=android-arm64 make release -j4
```

### Beispiel: Kompletter Build-Workflow

```bash
cd ~/xmod

# Clean vorherige Builds
PLATFORM=android-arm64 make clean
PLATFORM=android-x86_64 make clean
PLATFORM=android-x86 make clean

# Release Builds erstellen (parallel)
PLATFORM=android-arm64 make release -j$(nproc)
PLATFORM=android-x86_64 make release -j$(nproc)
PLATFORM=android-x86 make release -j$(nproc)
```

## CLion IDE Konfiguration

**CLion** ist eine leistungsstarke C/C++ IDE von JetBrains, die Makefile-Projekte unterstützt.

### Projekt öffnen

1. Starten Sie CLion
2. **File → Open**
3. Wählen Sie das xmod Repository-Verzeichnis
4. CLion erkennt automatisch das Makefile-Projekt

### Umgebungsvariablen in CLion setzen

CLion benötigt die NDK-Umgebungsvariablen, um das Projekt korrekt zu bauen.

**Methode 1: Globale Environment Variables**

1. **File → Settings** (oder **Ctrl+Alt+S**)
2. **Build, Execution, Deployment → Makefile**
3. Im Bereich **Environment** klicken Sie auf das **+** Symbol
4. Fügen Sie hinzu:
   - Name: `ANDROID_NDK_HOME`
   - Value: `/home/<username>/Android/Sdk/ndk/android-ndk-r26d`
5. Fügen Sie hinzu:
   - Name: `NDK_ROOT`
   - Value: `/home/<username>/Android/Sdk/ndk/android-ndk-r26d`
6. Klicken Sie **OK**

**Methode 2: Per Build Configuration**

Umgebungsvariablen können auch pro Build-Konfiguration gesetzt werden (siehe nächster Abschnitt).

### Run Configurations erstellen

Erstellen Sie separate Run Configurations für jede Android-Plattform:

#### Configuration 1: Android ARM64 Release

1. **Run → Edit Configurations**
2. Klicken Sie auf das **+** Symbol
3. Wählen Sie **Makefile**
4. **Name:** `Android ARM64 Release`
5. **Target:** `release`
6. **Environment variables:** 
   ```
   PLATFORM=android-arm64
   ANDROID_NDK_HOME=/home/<username>/Android/Sdk/ndk/android-ndk-r26d
   NDK_ROOT=/home/<username>/Android/Sdk/ndk/android-ndk-r26d
   ```
7. **Working directory:** Wählen Sie das Repository-Root
8. Klicken Sie **OK**

#### Configuration 2: Android x86_64 Release

1. **Run → Edit Configurations**
2. Klicken Sie auf das **+** Symbol
3. Wählen Sie **Makefile**
4. **Name:** `Android x86_64 Release`
5. **Target:** `release`
6. **Environment variables:**
   ```
   PLATFORM=android-x86_64
   ANDROID_NDK_HOME=/home/<username>/Android/Sdk/ndk/android-ndk-r26d
   NDK_ROOT=/home/<username>/Android/Sdk/ndk/android-ndk-r26d
   ```
7. **Working directory:** Wählen Sie das Repository-Root
8. Klicken Sie **OK**

#### Configuration 3: Android x86 Release

1. **Run → Edit Configurations**
2. Klicken Sie auf das **+** Symbol
3. Wählen Sie **Makefile**
4. **Name:** `Android x86 Release`
5. **Target:** `release`
6. **Environment variables:**
   ```
   PLATFORM=android-x86
   ANDROID_NDK_HOME=/home/<username>/Android/Sdk/ndk/android-ndk-r26d
   NDK_ROOT=/home/<username>/Android/Sdk/ndk/android-ndk-r26d
   ```
7. **Working directory:** Wählen Sie das Repository-Root
8. Klicken Sie **OK**

#### Configuration 4: Android ARM64 Debug (optional)

Erstellen Sie analog eine Debug-Configuration:
- **Name:** `Android ARM64 Debug`
- **Target:** `debug`
- **Environment variables:** (wie oben für ARM64)

### Bauen in CLion

1. Wählen Sie die gewünschte Configuration aus dem Dropdown (z.B. `Android ARM64 Release`)
2. Klicken Sie auf **Build** (oder drücken Sie **Ctrl+F9**)
3. Die Build-Ausgabe erscheint im **Messages** Fenster

### Tipps für effizientes Arbeiten

**Code Navigation:**
- CLion bietet Syntax-Highlighting, Code-Completion und Navigation für C/C++ Code
- **Ctrl+Click** auf Funktionen/Variablen springt zur Definition

**Build Output:**
- Das **Messages** Fenster zeigt Compiler-Warnungen und Fehler
- Klicken Sie auf Fehler, um zur entsprechenden Code-Zeile zu springen

**Terminal in CLion:**
- **View → Tool Windows → Terminal** öffnet ein eingebettetes Terminal
- Hier können Sie auch manuell `make` Befehle ausführen

**Mehrere Plattformen parallel bauen:**
- Sie können mehrere Build-Konfigurationen nacheinander ausführen
- Oder verwenden Sie das Terminal in CLion für parallele Builds

## Output-Dateien

### Build-Verzeichnisse

Kompilierte Dateien werden in plattformspezifischen Build-Verzeichnissen abgelegt:

```
xmod/
├── build.android-arm64/
│   ├── libqagame.mp.android.arm64-v8a.so
│   ├── libcgame.mp.android.arm64-v8a.so
│   └── libui.mp.android.arm64-v8a.so
├── build.android-x86_64/
│   ├── libqagame.mp.android.x86_64.so
│   ├── libcgame.mp.android.x86_64.so
│   └── libui.mp.android.x86_64.so
└── build.android-x86/
    ├── libqagame.mp.android.i386.so
    ├── libcgame.mp.android.i386.so
    └── libui.mp.android.i386.so
```

### Datei-Übersicht

| Modul | Beschreibung | Verwendung |
|-------|--------------|------------|
| **libqagame** | Server-seitiges Spielmodul | Wird vom Server geladen |
| **libcgame** | Client-seitiges Spielmodul | Wird vom Android-Client geladen |
| **libui** | User Interface Modul | Wird vom Android-Client geladen (Menüs, HUD) |

### Dateien für Android-Clients

Für einen Android Enemy Territory Client benötigen Sie:
- `libcgame.mp.android.<arch>.so` - Client-Logik
- `libui.mp.android.<arch>.so` - User Interface

Die Server-Datei (`libqagame`) wird **nicht** auf Android-Clients benötigt.

### APK Integration (für Android-Entwickler)

Um die kompilierten .so Dateien in eine Android-App zu integrieren:

1. Kopieren Sie die .so Dateien in das Android Studio Projekt:
   ```
   app/src/main/jniLibs/
   ├── arm64-v8a/
   │   ├── libcgame.mp.android.arm64-v8a.so
   │   └── libui.mp.android.arm64-v8a.so
   ├── x86_64/
   │   ├── libcgame.mp.android.x86_64.so
   │   └── libui.mp.android.x86_64.so
   └── x86/
       ├── libcgame.mp.android.i386.so
       └── libui.mp.android.i386.so
   ```

2. Android Studio inkludiert die Libraries automatisch in die APK

## Troubleshooting

### Fehler: "NDK not found"

**Fehlermeldung:**
```
make: *** No rule to make target '...'.  Stop.
```
oder
```
clang++: command not found
```

**Lösung:**

1. Überprüfen Sie, ob NDK installiert ist:
   ```bash
   ls ~/Android/Sdk/ndk/android-ndk-r26d/
   ```

2. Überprüfen Sie die Umgebungsvariablen:
   ```bash
   echo $ANDROID_NDK_HOME
   echo $NDK_ROOT
   ```

3. Falls leer, setzen Sie die Variablen erneut:
   ```bash
   export ANDROID_NDK_HOME="$HOME/Android/Sdk/ndk/android-ndk-r26d"
   export NDK_ROOT="$ANDROID_NDK_HOME"
   ```

4. In CLion: Überprüfen Sie die Environment Variables in den Build-Konfigurationen

### Fehler: "Toolchain not found"

**Fehlermeldung:**
```
/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang++: No such file or directory
```

**Ursache:** Das NDK ist nicht vollständig oder an einem anderen Ort installiert.

**Lösung:**

1. Überprüfen Sie den Toolchain-Pfad:
   ```bash
   ls $ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/
   ```

2. Falls das Verzeichnis nicht existiert:
   - Löschen Sie das unvollständige NDK
   - Laden Sie es erneut herunter
   - Entpacken Sie es vollständig

3. Stellen Sie sicher, dass Sie die **Linux**-Version des NDK heruntergeladen haben (nicht Darwin/macOS oder Windows)

### Fehler: "Cannot find -llog"

**Fehlermeldung:**
```
ld: error: cannot find -llog
```

**Ursache:** Die Android-System-Libraries fehlen oder der Sysroot-Pfad ist falsch.

**Lösung:**

1. Überprüfen Sie, ob das NDK die Android-Libraries enthält:
   ```bash
   ls $ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/
   ```

2. Stellen Sie sicher, dass Sie NDK r21 oder neuer verwenden (empfohlen: r26d)

3. Re-installieren Sie das NDK falls notwendig

### Fehler: "Missing m4 or python3"

**Fehlermeldung:**
```
m4: command not found
```
oder
```
python3: command not found
```

**Lösung:**

Installieren Sie die fehlenden Pakete:
```bash
sudo apt install m4 python3
```

### Warnung: "Permission denied" beim Bauen

**Fehlermeldung:**
```
bash: ./some-script.sh: Permission denied
```

**Lösung:**

Setzen Sie die Ausführungsrechte:
```bash
chmod +x build-all.sh
chmod +x migrate_userdb.sh
```

### Build ist sehr langsam

**Lösung:**

Nutzen Sie parallele Kompilierung mit der `-j` Option:
```bash
# Nutzt alle CPU-Kerne
PLATFORM=android-arm64 make release -j$(nproc)
```

Auf einem 8-Kern System kann dies den Build um den Faktor 5-8x beschleunigen.

### CLion erkennt Makefile nicht

**Lösung:**

1. **File → Reload CMake Project** (falls CMake aktiviert ist)
2. Oder: **File → Settings → Build, Execution, Deployment → Makefile**
3. Stellen Sie sicher, dass der **Makefile path** korrekt auf `GNUmakefile` zeigt
4. Klicken Sie **Reload Makefile Project**

### Alte NDK-Version

Wenn Sie eine ältere NDK-Version verwenden (z.B. r21), kann es zu Kompatibilitätsproblemen kommen.

**Lösung:**

Aktualisieren Sie auf NDK r26d oder neuer:

```bash
cd ~/Android/Sdk/ndk
wget https://dl.google.com/android/repository/android-ndk-r26d-linux.zip
unzip android-ndk-r26d-linux.zip
```

Aktualisieren Sie die Umgebungsvariablen entsprechend.

## Weiterführende Dokumentation

- [BUILD.md](../BUILD.md) - Allgemeine Build-Anleitung für alle Plattformen
- [BUILD_VS2022.md](BUILD_VS2022.md) - Windows Build mit Visual Studio 2022
- [README.md](../README.md) - Projekt-Übersicht und Features
- [notes/BuildSystem.txt](../notes/BuildSystem.txt) - Detaillierte Build-System Dokumentation

## Support

Bei Problemen:

1. Überprüfen Sie diese Dokumentation
2. Suchen Sie in den [GitHub Issues](https://github.com/jay1110/xmod/issues)
3. Erstellen Sie ein neues Issue mit:
   - Ihrer Ubuntu-Version
   - NDK-Version (`ls ~/Android/Sdk/ndk/`)
   - Vollständiger Fehlerausgabe
   - Schritten zur Reproduktion des Problems
