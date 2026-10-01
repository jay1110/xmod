# macOS-Builds von Ubuntu aus erstellen

Dieses Dokument beschreibt verschiedene Wege, um xmod für macOS von einem Ubuntu 24.04 System aus zu kompilieren. Da macOS spezielle Binaries benötigt (`.dylib` und `.bundle` Dateien), gibt es mehrere Ansätze mit unterschiedlichen Schwierigkeitsgraden.

## Inhaltsverzeichnis

- [1. Übersicht](#1-übersicht)
- [2. Weg 1: GitHub Actions (EMPFOHLEN)](#2-weg-1-github-actions-empfohlen)
- [3. Weg 2: OSXCross Toolchain](#3-weg-2-osxcross-toolchain)
- [4. Weg 3: Cloud-basierter Mac](#4-weg-3-cloud-basierter-mac)
- [5. Weg 4: Nativer macOS Build](#5-weg-4-nativer-macos-build)
- [6. Vergleichstabelle](#6-vergleichstabelle)
- [7. Output-Dateien](#7-output-dateien)
- [8. Integration in bestehenden Workflow](#8-integration-in-bestehenden-workflow)
- [9. Troubleshooting](#9-troubleshooting)
- [10. Weiterführende Links](#10-weiterführende-links)

## 1. Übersicht

### Was wird gebaut?

Für macOS werden folgende Module erstellt:

- **cgame** - Client-seitiges Spielmodul (`.dylib` oder `.bundle`)
- **ui** - User Interface Modul (`.dylib` oder `.bundle`)
- **qagame** - Server-seitiges Spielmodul (`.dylib` oder `.bundle`)

### ⚠️ Wichtiger Hinweis

Cross-Compilation für macOS ist **komplex und hat rechtliche Einschränkungen**:

- Apple's Xcode SDK darf nur mit einem Apple Developer Account heruntergeladen werden
- Die Verwendung des SDK unterliegt Apples Lizenzbedingungen
- Code-Signing ist für Distribution erforderlich (nur auf echten Macs möglich)

### Methoden-Übersicht

| Methode | Schwierigkeit | Zuverlässigkeit | Kosten | Empfehlung |
|---------|---------------|-----------------|--------|------------|
| **GitHub Actions** | ⭐ Niedrig | ⭐⭐⭐⭐⭐ Sehr hoch | Kostenlos | 🏆 **EMPFOHLEN** |
| **OSXCross** | ⭐⭐⭐⭐⭐ Sehr hoch | ⭐⭐ Mittel | Kostenlos | ⚠️ Nur für Experten |
| **Cloud Mac** | ⭐⭐ Niedrig | ⭐⭐⭐⭐ Hoch | 💰 $20-30/Monat | Bei Budget |
| **Nativer Mac** | ⭐ Niedrig | ⭐⭐⭐⭐⭐ Sehr hoch | Kostenlos* | Bei vorhandener Hardware |

## 2. Weg 1: GitHub Actions (EMPFOHLEN)

**Schwierigkeitsgrad:** ⭐ Niedrig  
**Zuverlässigkeit:** ⭐⭐⭐⭐⭐ Sehr hoch  
**Kosten:** ✅ Kostenlos  
**Für Produktion geeignet:** ✅ Ja

### Warum GitHub Actions?

GitHub Actions bietet **native macOS Runner** mit vorinstalliertem Xcode. Das ist der einfachste und zuverlässigste Weg, macOS-Binaries zu erstellen, ohne selbst einen Mac zu besitzen.

**Vorteile:**
- ✅ Xcode und alle Tools bereits installiert
- ✅ Keine lokale Konfiguration nötig
- ✅ Automatische Builds bei jedem Push/Release
- ✅ Kostenlos für öffentliche Repositories
- ✅ Professionelle Build-Infrastruktur

### Schritt-für-Schritt Anleitung

#### 2.1 Workflow-Datei erweitern

Öffne die Datei `.github/workflows/build-release.yml` und füge einen neuen Job hinzu:

```yaml
  build-macos:
    runs-on: macos-latest  # oder macos-13 (Intel) / macos-14 (ARM)
    steps:
      - uses: actions/checkout@v4
      
      - name: Install dependencies
        run: |
          # Xcode Command Line Tools sind bereits installiert
          # Nur zusätzliche Tools falls nötig
          brew install python3 zip m4
      
      - name: Ensure scripts are executable
        run: chmod +x project/info.py
      
      - name: Build macOS 64-bit (Intel)
        run: PLATFORM=osx64 make release
      
      - name: Upload macOS 64-bit artifacts
        uses: actions/upload-artifact@v4
        with:
          name: xmod-macos64
          path: |
            build.osx64-release/game/qagame.mp.x86_64.dylib
            build.osx64-release/cgame/cgame.mp.x86_64.dylib
            build.osx64-release/ui/ui.mp.x86_64.dylib
```

#### 2.2 Integration in Release-Job

Erweitere den `create-release` Job um macOS-Artefakte:

```yaml
  create-release:
    needs: [build-linux64, build-linux32, build-windows, build-macos, build-pak-data]
    runs-on: ubuntu-latest
    if: always()
    steps:
      - uses: actions/checkout@v4
      
      - name: Download all artifacts
        uses: actions/download-artifact@v4
        with:
          path: artifacts
      
      - name: Create unified release package
        run: |
          mkdir -p release/xmod
          mkdir -p pak-combined
          
          # ... (bestehender Code) ...
          
          # macOS cgame/ui zu pak-combined hinzufügen
          find artifacts/xmod-macos64 -name "cgame*.dylib" -exec cp {} pak-combined/ \; 2>/dev/null || true
          find artifacts/xmod-macos64 -name "ui*.dylib" -exec cp {} pak-combined/ \; 2>/dev/null || true
          
          # ... pk3 erstellen ...
          
          # macOS qagame zur release folder
          find artifacts/xmod-macos64 -name "qagame*.dylib" -exec cp {} release/xmod/ \; 2>/dev/null || true
          
          # ... (restlicher Code) ...
```

#### 2.3 Workflow triggern

**Option A: Manueller Trigger**
1. Gehe zu GitHub → Actions
2. Wähle "Build and Release" Workflow
3. Klicke "Run workflow"
4. Warte auf Completion (~5-10 Minuten)

**Option B: Automatischer Trigger**
- Push einen Tag: `git tag v1.0.0 && git push --tags`
- Der Workflow startet automatisch

#### 2.4 Binaries herunterladen

Nach erfolgreichem Build:
1. Gehe zu GitHub → Actions → Dein Workflow Run
2. Scrolle zu "Artifacts"
3. Lade `xmod-2.0.4.zip` herunter

### Unterschiedliche macOS Runner

GitHub bietet verschiedene macOS-Runner:

| Runner | Architektur | Xcode Version | macOS Version |
|--------|-------------|---------------|---------------|
| `macos-latest` | ARM64 (M1/M2) | Neueste | macOS 14+ |
| `macos-14` | ARM64 (M1/M2) | 15.x | macOS 14 Sonoma |
| `macos-13` | Intel (x86_64) | 14.x | macOS 13 Ventura |
| `macos-12` | Intel (x86_64) | 13.x | macOS 12 Monterey |

**Empfehlung:** Verwende `macos-13` für Intel-Builds (x86_64), da die meisten macOS-Server noch auf Intel laufen.

### Kompletter YAML-Job für beide Architekturen

```yaml
  build-macos-intel:
    runs-on: macos-13  # Intel
    steps:
      - uses: actions/checkout@v4
      
      - name: Install dependencies
        run: brew install python3 zip m4
      
      - name: Ensure scripts are executable
        run: chmod +x project/info.py
      
      - name: Build macOS Intel (x86_64)
        run: PLATFORM=osx64 make release
      
      - name: Upload Intel artifacts
        uses: actions/upload-artifact@v4
        with:
          name: xmod-macos-intel
          path: build.osx64-release/**/*.dylib

  build-macos-arm:
    runs-on: macos-14  # Apple Silicon
    steps:
      - uses: actions/checkout@v4
      
      - name: Install dependencies
        run: brew install python3 zip m4
      
      - name: Ensure scripts are executable
        run: chmod +x project/info.py
      
      # Hinweis: Benötigt osx-arm64 Platform-File
      - name: Build macOS ARM (arm64)
        run: |
          if [ -f "make/platform/osx-arm64" ]; then
            PLATFORM=osx-arm64 make release
          else
            echo "::warning::ARM64 platform not yet configured"
          fi
      
      - name: Upload ARM artifacts
        uses: actions/upload-artifact@v4
        with:
          name: xmod-macos-arm
          path: build.osx-arm64-release/**/*.dylib
        continue-on-error: true
```

## 3. Weg 2: OSXCross Toolchain

**Schwierigkeitsgrad:** ⭐⭐⭐⭐⭐ Sehr hoch  
**Zuverlässigkeit:** ⭐⭐ Mittel  
**Kosten:** Kostenlos  
**Für Produktion:** ⚠️ **NICHT EMPFOHLEN** (Experimentell)

### ⚠️ Wichtige Warnungen

- **Komplex:** Installation dauert mehrere Stunden
- **Rechtlich:** Xcode SDK nur mit Apple Developer Account legal
- **Fehleranfällig:** Viele Kompatibilitätsprobleme
- **Kein Code-Signing:** Binaries funktionieren evtl. nicht auf allen Systemen
- **Nur für Tests:** Nicht für Production-Releases verwenden

### 3.1 Voraussetzungen

```bash
# Ubuntu 24.04
sudo apt-get update
sudo apt-get install -y \
    clang \
    cmake \
    libxml2-dev \
    libssl-dev \
    zlib1g-dev \
    git \
    build-essential \
    patch \
    python3 \
    liblzma-dev \
    libz-dev \
    libbz2-dev
```

### 3.2 Xcode SDK beschaffen

**⚠️ ACHTUNG:** Das Xcode SDK unterliegt Apples Lizenzbedingungen!

**Option A: Mit Apple Developer Account (LEGAL)**

1. Melde dich bei https://developer.apple.com an
2. Lade Xcode herunter (z.B. Xcode 14.3)
3. Extrahiere das SDK:

```bash
# Lade Xcode_14.3.xip herunter
# Extrahiere mit OSXCross Tool
./tools/gen_sdk_package_pbzx.sh /pfad/zu/Xcode_14.3.xip
```

**Option B: Bereits extrahiertes SDK (falls vorhanden)**

```bash
# SDK muss im tarballs/ Verzeichnis liegen
cp /pfad/zu/MacOSX14.0.sdk.tar.xz osxcross/tarballs/
```

### 3.3 OSXCross Installation

```bash
# 1. Repository klonen
cd ~
git clone https://github.com/tpoechtrager/osxcross
cd osxcross

# 2. SDK vorbereiten (siehe 3.2)
# Stelle sicher dass SDK in tarballs/ liegt
ls -lh tarballs/

# 3. Toolchain bauen (dauert 30-60 Minuten!)
UNATTENDED=1 ./build.sh

# 4. Toolchain testen
export PATH="$PWD/target/bin:$PATH"
export OSXCROSS_ROOT="$PWD"

# Test ob Compiler funktioniert
x86_64-apple-darwin21-clang --version
```

### 3.4 Platform-Datei erstellen

Erstelle eine neue Datei `make/platform/osx64-cross` basierend auf `make/platform/osx64`:

```text
# OSXCross Cross-Compilation Configuration
OSXCROSS_ROOT ?= $(HOME)/osxcross
OSXCROSS_TARGET_DIR = $(OSXCROSS_ROOT)/target

###############################################################################

IDMODULE.prefix  =
IDMODULE.suffix  = .mp.x86_64.dylib
STATICLIB.suffix = .a

###############################################################################

DYNLOAD.l  =
MATH.l     =
IPHLPAPI.l =
ADVAPI.l   =

###############################################################################

M4  = m4
TAR = tar
AR  = $(OSXCROSS_TARGET_DIR)/bin/x86_64-apple-darwin21-ar
RANLIB = $(OSXCROSS_TARGET_DIR)/bin/x86_64-apple-darwin21-ranlib

###############################################################################

CXX = $(OSXCROSS_TARGET_DIR)/bin/x86_64-apple-darwin21-clang++

CXX.pch.ext  = gch
CXX.pch.arch = $(CXX.arch)

CXX.opt.D     = $(call fnPrefix,-D,$(CXX.D) $($(CXX.inherit).CXX.D))
CXX.opt.I     = $(call fnPrefix,-I,$($(CXX.inherit).CXX.I<) $(CXX.I) $($(CXX.inherit).CXX.I))
CXX.opt.L     = $(call fnPrefix,-L,$(CXX.L) $($(CXX.inherit).CXX.L))
CXX.opt.R     = $(call fnPrefix,-Xlinker -R,$(CXX.R) $($(CXX.inherit).CXX.R))
CXX.opt.U     = $(call fnPrefix,-U,$(CXX.U) $($(CXX.inherit).CXX.U))
CXX.opt.arch  = $(call fnPrefix,-arch ,$(CXX.arch) $($(CXX.arch).CXX.arch))
CXX.opt.l     = $(call fnPrefix,-l,$(CXX.l) $($(CXX.inherit).CXX.l))
CXX.opt.fwork = $(call fnPrefix,-framework ,$(CXX.fwork) $($(CXX.inherit).CXX.fwork))

CXX.opt.ML   = -fmessage-length=0
CXX.opt.NSA  = -fno-strict-aliasing
CXX.opt.O    = -O3 -ffast-math
CXX.opt.PIC  = -fPIC
CXX.opt.W    = -w
CXX.opt.g    = -g
CXX.opt.pipe = -pipe
CXX.opt.std  = -fno-exceptions -fno-rtti
CXX.opt.vis  = -fvisibility=hidden

CXX.ldopts.so = -bundle -undefined dynamic_lookup

###############################################################################

CXX.D     =
CXX.I     = $(BUILD/) $(PROJECT/)src $(PROJECT/)pak
CXX.L     =
CXX.R     =
CXX.U     =
CXX.arch  = x86_64
CXX.fwork =
CXX.l     =

CXX.ML   = 1
CXX.NSA  = 1
CXX.O    =
CXX.PIC  = 1
CXX.W    = 1
CXX.g    =
CXX.pipe = 1
CXX.std  = 1
CXX.vis  =

###############################################################################

CXX.fnStrip =

###############################################################################

CXX.fnCompile = $(call print.COMMAND.normal,$(CXX),$(2),$(strip \
$(CXX) \
$(CXX.opt.arch) \
    $(foreach i,pipe W ML std NSA vis PIC O g,$(foreach j,$(CXX.$(i)),$(CXX.opt.$(i)))) \
$(CXX.opt.D) \
$(CXX.opt.I) \
$(CXX.opt.U) \
-c $(1) -o $(2) \
))

CXX.fnCompilePch = $(CXX.fnCompile)

###############################################################################

CXX.fnLinkSo = $(call print.LINK,$(CXX),$(1),$(2),$(strip \
$(CXX) \
$(CXX.opt.arch) \
$(foreach i,pipe W ML std NSA vis PIC O g,$(foreach j,$(CXX.$(i)),$(CXX.opt.$(i)))) \
$(CXX.opt.D) \
$(CXX.opt.I) \
$(CXX.opt.U) \
$(CXX.fnLinkSo.<<) $($(CXX.inherit).CXX.fnLinkSo.<<) \
-o $(1).binary $(2) \
$(CXX.ldopts.so) \
$(CXX.opt.R) \
$(CXX.opt.L) \
$(CXX.opt.l) \
$(CXX.opt.fwork) \
$(CXX.fnLinkSo.>>) $($(CXX.inherit).CXX.fnLinkSo.>>) \
))

###############################################################################

CXX.fnRanlib = $(call print.COMMAND.normal,$(RANLIB),$(1),$(RANLIB) $(1))

###############################################################################

SPECIALS.name  = osx64-cross
SPECIALS.build = build
SPECIALS.base  = $(PROJECT.packageBase)-$(SPECIALS.name)
SPECIALS.base/ = $(BUILD/)$(SPECIALS.base)/

world:: default debug release
```

### 3.5 Build ausführen

```bash
# Environment setzen
export OSXCROSS_ROOT="$HOME/osxcross"
export PATH="$OSXCROSS_ROOT/target/bin:$PATH"

# Build starten
cd /pfad/zu/xmod
make PLATFORM=osx64-cross release

# Output in:
# build.osx64-cross-release/game/qagame.mp.x86_64.dylib
# build.osx64-cross-release/cgame/cgame.mp.x86_64.dylib
# build.osx64-cross-release/ui/ui.mp.x86_64.dylib
```

### 3.6 Bekannte Probleme und Lösungen

#### Problem: "SDK not found"

```bash
# Lösung: Prüfe ob SDK im tarballs/ Verzeichnis liegt
ls -lh ~/osxcross/tarballs/
# Sollte MacOSXXX.sdk.tar.xz zeigen

# Falls nicht, SDK erneut kopieren
cp /pfad/zu/MacOSX*.sdk.tar.xz ~/osxcross/tarballs/
cd ~/osxcross
UNATTENDED=1 ./build.sh
```

#### Problem: "Compiler not found"

```bash
# Lösung: PATH nicht korrekt gesetzt
export PATH="$HOME/osxcross/target/bin:$PATH"

# Test
which x86_64-apple-darwin21-clang
```

#### Problem: "Framework not found"

OSXCross hat eingeschränkten Zugriff auf macOS Frameworks. Einige Features könnten fehlen.

**Lösung:** Verwende GitHub Actions stattdessen.

#### Problem: Bundle-Format

OSXCross erstellt `.dylib` Dateien statt `.bundle`. Das ist meist OK, kann aber bei älteren ET-Versionen Probleme verursachen.

**Lösung:** 
- Verwende `.dylib` Format (funktioniert in modernen ET-Versionen)
- Oder verwende einen echten Mac für `.bundle` Format

#### Problem: Code-Signing

Cross-kompilierte Binaries sind nicht signiert und werden von modernen macOS-Versionen blockiert.

**Lösung:**
- Benutzer müssen Binaries manuell freigeben (`xattr -d com.apple.quarantine`)
- Für Distribution: Code-Signing nur auf echtem Mac möglich

### 3.7 Warum OSXCross schwierig ist

- **SDK-Lizenz:** Legal nur mit Apple Developer Account
- **Fehlende Tools:** Keine nativen macOS-Build-Tools
- **Framework-Probleme:** Nicht alle macOS-Frameworks verfügbar
- **Code-Signing:** Unmöglich ohne echten Mac
- **Wartung:** OSXCross muss bei neuen macOS-Versionen aktualisiert werden
- **Debugging:** Sehr schwierig ohne macOS-System

**Fazit:** Verwende OSXCross nur für Tests, nicht für Production!


## 4. Weg 3: Cloud-basierter Mac

**Schwierigkeitsgrad:** ⭐⭐ Niedrig  
**Zuverlässigkeit:** ⭐⭐⭐⭐ Hoch  
**Kosten:** 💰 $20-30/Monat  
**Für Produktion:** ✅ Ja

### Übersicht Cloud-Anbieter

#### MacStadium
- **Website:** https://www.macstadium.com
- **Kosten:** Ab $99/Monat (Dedicated Mac mini)
- **Vorteile:** Echte Mac-Hardware, volle Kontrolle
- **Nachteile:** Relativ teuer

#### MacinCloud
- **Website:** https://www.macincloud.com
- **Kosten:** Ab $20/Monat (Shared), $79/Monat (Dedicated)
- **Vorteile:** Günstigste Option
- **Nachteile:** Shared-Pläne können langsam sein

#### AWS EC2 Mac Instances
- **Website:** https://aws.amazon.com/ec2/instance-types/mac/
- **Kosten:** ~$25/Tag (24h minimum), ~$600/Monat
- **Vorteile:** Integration in AWS-Infrastruktur
- **Nachteile:** Teuer, 24h Mindestlaufzeit

### Setup (Beispiel: MacinCloud)

#### 4.1 Account erstellen

1. Gehe zu https://www.macincloud.com
2. Wähle einen Plan (z.B. "Mac Plan" für $20/Monat)
3. Erstelle Account und buche Server

#### 4.2 SSH-Zugriff einrichten

```bash
# SSH-Verbindung testen
ssh username@your-mac.macincloud.com

# Falls Passwort-Login:
# Gib dein Passwort ein

# Besser: SSH-Key einrichten
ssh-copy-id username@your-mac.macincloud.com
```

#### 4.3 Xcode installieren (falls nicht vorhanden)

```bash
# Auf dem Mac:
xcode-select --install

# Oder Xcode aus App Store
# (nur mit Apple-ID möglich)
```

#### 4.4 Repository klonen und bauen

```bash
# Auf dem Cloud-Mac via SSH:
ssh username@your-mac.macincloud.com

# Git installieren (falls nötig)
brew install git

# Repository klonen
git clone https://github.com/jay1110/xmod
cd xmod

# Build für Intel (x86_64)
make PLATFORM=osx64 release

# Binaries liegen in:
# build.osx64-release/
```

#### 4.5 Dateien herunterladen

**Option A: SCP**
```bash
# Von deinem Ubuntu-System aus:
scp -r username@your-mac.macincloud.com:~/xmod/build.osx64-release/*.dylib ./
```

**Option B: rsync**
```bash
# Von deinem Ubuntu-System aus:
rsync -avz username@your-mac.macincloud.com:~/xmod/build.osx64-release/ ./macos-builds/
```

**Option C: Git**
```bash
# Auf dem Mac:
git add build.osx64-release/
git commit -m "macOS builds"
git push

# Auf Ubuntu:
git pull
```

### 4.6 Automatisierung

Erstelle ein Build-Script für automatische Builds:

```bash
#!/bin/bash
# build-macos-remote.sh

MAC_HOST="username@your-mac.macincloud.com"
REPO_PATH="~/xmod"

echo "Building on remote Mac..."
ssh $MAC_HOST "cd $REPO_PATH && git pull && make PLATFORM=osx64 clean && make PLATFORM=osx64 release"

echo "Downloading binaries..."
mkdir -p macos-builds
scp -r $MAC_HOST:$REPO_PATH/build.osx64-release/*.dylib ./macos-builds/

echo "Done! Binaries in ./macos-builds/"
```

Ausführen:
```bash
chmod +x build-macos-remote.sh
./build-macos-remote.sh
```

## 5. Weg 4: Nativer macOS Build

**Schwierigkeitsgrad:** ⭐ Sehr niedrig  
**Zuverlässigkeit:** ⭐⭐⭐⭐⭐ Sehr hoch  
**Kosten:** Kostenlos (Hardware vorhanden)  
**Für Produktion:** ✅ Ja

Falls du Zugriff auf einen echten Mac hast (eigener Mac, Freund, Firma):

### 5.1 Xcode Command Line Tools installieren

```bash
# Terminal öffnen
xcode-select --install

# Warten bis Installation abgeschlossen ist
```

### 5.2 Homebrew installieren (optional, aber empfohlen)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 5.3 Repository klonen

```bash
git clone https://github.com/jay1110/xmod
cd xmod
```

### 5.4 Build ausführen

```bash
# Für Intel Macs (x86_64):
make PLATFORM=osx64 release

# Für Apple Silicon Macs (ARM64):
# Hinweis: Benötigt make/platform/osx-arm64 File
make PLATFORM=osx-arm64 release

# Oder Legacy 32-bit (sehr alte Macs):
make PLATFORM=osx release
```

### 5.5 Output-Dateien

**Intel (osx64):**
```
build.osx64-release/game/qagame.mp.x86_64.dylib
build.osx64-release/cgame/cgame.mp.x86_64.dylib
build.osx64-release/ui/ui.mp.x86_64.dylib
```

**Apple Silicon (osx-arm64):**
```
build.osx-arm64-release/game/qagame.mp.arm64.dylib
build.osx-arm64-release/cgame/cgame.mp.arm64.dylib
build.osx-arm64-release/ui/ui.mp.arm64.dylib
```

### 5.6 Code-Signing (für Distribution)

Für öffentliche Distribution sollten Binaries signiert werden:

```bash
# Voraussetzung: Apple Developer Certificate
codesign --sign "Developer ID Application: Dein Name" qagame.mp.x86_64.dylib

# Oder ad-hoc signing (nur für lokale Tests):
codesign -s - qagame.mp.x86_64.dylib
```


## 6. Vergleichstabelle

| Methode | Aufwand | Kosten | Zuverlässigkeit | Code-Signing | Für Produktion | Empfehlung |
|---------|---------|--------|-----------------|--------------|----------------|------------|
| **GitHub Actions** | ⭐ Sehr niedrig | ✅ Kostenlos | ⭐⭐⭐⭐⭐ Sehr hoch | ❌ Nein* | ✅ Ja | 🏆 **EMPFOHLEN** |
| **OSXCross** | ⭐⭐⭐⭐⭐ Sehr hoch | ✅ Kostenlos | ⭐⭐ Mittel | ❌ Nein | ⚠️ Experimentell | Nur für Tests |
| **Cloud Mac** | ⭐⭐ Niedrig | 💰 $20-30/Monat | ⭐⭐⭐⭐ Hoch | ✅ Ja | ✅ Ja | Bei Budget |
| **Nativer Mac** | ⭐ Sehr niedrig | ✅ Kostenlos** | ⭐⭐⭐⭐⭐ Sehr hoch | ✅ Ja | ✅ Ja | Bei Hardware |

**Legende:**
- ⭐ = Schwierigkeitsgrad (weniger = einfacher)
- ✅ = Verfügbar/Ja
- ❌ = Nicht verfügbar/Nein
- ⚠️ = Eingeschränkt
- 💰 = Kostenpflichtig
- 🏆 = Beste Wahl

\* Code-Signing in GitHub Actions ist mit Secrets möglich, aber komplex  
\** Kostenlos wenn Mac bereits vorhanden

### Empfehlung nach Anwendungsfall

| Anwendungsfall | Beste Methode |
|----------------|---------------|
| **Open Source Projekt** | GitHub Actions |
| **Automatische Releases** | GitHub Actions |
| **Einmalige Builds** | Cloud Mac oder Freund mit Mac |
| **Regelmäßige Development Builds** | GitHub Actions oder Cloud Mac |
| **Signierte Binaries für Distribution** | Nativer Mac oder Cloud Mac |
| **Experimentieren/Lernen** | OSXCross (mit Vorsicht) |
| **Du hast bereits einen Mac** | Nativer Build |

## 7. Output-Dateien

### Datei-Formate

macOS verwendet verschiedene Formate je nach Build-Konfiguration:

#### Format 1: .dylib (Empfohlen)

**Platform:** `osx64`, `osx-arm64`

```
qagame.mp.x86_64.dylib
cgame.mp.x86_64.dylib
ui.mp.x86_64.dylib
```

**Vorteile:**
- Standard macOS dynamic library Format
- Einfacher zu erstellen
- Funktioniert in modernen ET-Versionen

#### Format 2: .bundle (Legacy)

**Platform:** `osx` (32-bit)

```
qagame_mac
cgame_mac
ui_mac
```

Diese werden als Bundle-Verzeichnisse erstellt:
```
qagame_mac.bundle/
  Contents/
    Info.plist
    MacOS/
      qagame_mac
```

Dann zu ZIP komprimiert: `qagame_mac` (Bundle als ZIP-Archiv)

**Hinweis:** Bundle-Format wird von älteren ET-Versionen erwartet, ist aber komplexer zu erstellen.

### Build-Verzeichnisse

Je nach Platform und Variant:

```
build.osx64/                    # Default build
build.osx64-debug/              # Debug build  
build.osx64-release/            # Release build
  ├── game/
  │   └── qagame.mp.x86_64.dylib
  ├── cgame/
  │   └── cgame.mp.x86_64.dylib
  └── ui/
      └── ui.mp.x86_64.dylib
```

### Installation im ET-Server

```
etmain/
└── xmod/
    ├── xmod-2.0.4.pk3           # Enthält cgame + ui
    └── qagame.mp.x86_64.dylib   # Server-Modul
```

**Wichtig:** cgame und ui kommen normalerweise in die `.pk3` Datei, qagame bleibt separat für den Server.

## 8. Integration in bestehenden Workflow

### Einfache Integration

Füge diesen Job zur `.github/workflows/build-release.yml` hinzu:

```yaml
  build-macos:
    runs-on: macos-13
    steps:
      - uses: actions/checkout@v4
      - name: Install dependencies
        run: brew install python3 zip m4
      - name: Ensure scripts are executable
        run: chmod +x project/info.py
      - name: Build macOS 64-bit
        run: PLATFORM=osx64 make release
      - name: Upload artifacts
        uses: actions/upload-artifact@v4
        with:
          name: xmod-macos64
          path: build.osx64-release/**/*.dylib
```

Und erweitere den `create-release` Job dependencies:

```yaml
  create-release:
    needs: [build-linux64, build-linux32, build-windows, build-macos, build-pak-data]
    # ... rest of the job
```

Füge macOS-Artefakte zum Release-Paket hinzu:

```yaml
          # In der create-release run section:
          # macOS cgame/ui zu pak-combined
          find artifacts/xmod-macos64 -name "cgame*.dylib" -exec cp {} pak-combined/ \; 2>/dev/null || true
          find artifacts/xmod-macos64 -name "ui*.dylib" -exec cp {} pak-combined/ \; 2>/dev/null || true
          
          # macOS qagame zur release folder
          find artifacts/xmod-macos64 -name "qagame*.dylib" -exec cp {} release/xmod/ \; 2>/dev/null || true
```


## 9. Troubleshooting

### GitHub Actions Probleme

#### Problem: "make: command not found"

```yaml
# Lösung: make ist standardmäßig installiert auf macOS runners
# Falls nicht, installiere es:
- name: Install make
  run: brew install make
```

#### Problem: "Python not found"

```yaml
# Lösung: Python 3 ist bereits installiert, aber manchmal als python3
- name: Fix Python symlink
  run: |
    ln -sf $(which python3) /usr/local/bin/python
    python --version
```

#### Problem: "Platform file not found"

```
Error: make/platform/osx64: No such file or directory
```

**Lösung:** Stelle sicher, dass die Platform-Datei im Repository vorhanden ist:
```bash
ls -la make/platform/osx64
```

#### Problem: Workflow findet keine Artifacts

```yaml
# Stelle sicher, dass der Pfad korrekt ist
- name: Upload artifacts
  uses: actions/upload-artifact@v4
  with:
    name: xmod-macos64
    path: |
      build.osx64-release/game/*.dylib
      build.osx64-release/cgame/*.dylib
      build.osx64-release/ui/*.dylib
    if-no-files-found: error  # Wirft Fehler wenn nichts gefunden
```

### OSXCross Probleme

#### Problem: "SDK not found"

```bash
# Prüfe ob SDK vorhanden
ls -lh ~/osxcross/tarballs/

# Falls leer, SDK herunterladen und extrahieren
# Siehe Abschnitt 3.2
```

#### Problem: "ld: framework not found"

```
ld: framework 'Foundation' not found
```

**Lösung:** Framework-Pfad fehlt. Erweitere Platform-Datei:

```makefile
CXX.opt.F = -F$(OSXCROSS_TARGET_DIR)/SDK/MacOSX14.0.sdk/System/Library/Frameworks
```

#### Problem: "fatal error: 'TargetConditionals.h' file not found"

```bash
# Lösung: SDK-Pfad nicht korrekt
export OSXCROSS_SDK_PATH="$OSXCROSS_ROOT/target/SDK/MacOSX14.0.sdk"

# In Platform-Datei:
CXX.opt.isysroot = -isysroot $(OSXCROSS_ROOT)/target/SDK/MacOSX14.0.sdk
```

#### Problem: Compiler-Version mismatch

```
error: invalid deployment target for -stdlib=libc++
```

**Lösung:** Deployment Target setzen:

```bash
export MACOSX_DEPLOYMENT_TARGET=10.15
make PLATFORM=osx64-cross release
```

### macOS Runtime Probleme

#### Problem: "dylib cannot be opened because the developer cannot be verified"

Auf modernen macOS-Versionen werden nicht-signierte Binaries blockiert.

**Lösung für Benutzer:**
```bash
# Quarantine-Flag entfernen
xattr -d com.apple.quarantine qagame.mp.x86_64.dylib

# Oder für ganzes Verzeichnis:
xattr -dr com.apple.quarantine xmod/
```

**Lösung für Entwickler:** Code-Signing verwenden (siehe Abschnitt 5.6)

#### Problem: "wrong architecture"

```
Error: wrong architecture (expected x86_64, got arm64)
```

**Lösung:** Stelle sicher, dass du für die richtige Architektur kompilierst:
- Intel Macs: `PLATFORM=osx64`
- Apple Silicon: `PLATFORM=osx-arm64`

#### Problem: Universal Binaries

Falls du sowohl Intel als auch ARM unterstützen willst:

```bash
# Baue beide Versionen
make PLATFORM=osx64 release
make PLATFORM=osx-arm64 release

# Erstelle Universal Binary mit lipo
lipo -create \
  build.osx64-release/game/qagame.mp.x86_64.dylib \
  build.osx-arm64-release/game/qagame.mp.arm64.dylib \
  -output qagame.mp.dylib

# Prüfe Architekturen
lipo -info qagame.mp.dylib
# Output: Architectures in the fat file: qagame.mp.dylib are: x86_64 arm64
```

### Build-System Probleme

#### Problem: "No rule to make target"

```
make: *** No rule to make target 'release'. Stop.
```

**Lösung:** Platform nicht korrekt erkannt oder gesetzt:

```bash
# Explizit setzen
make PLATFORM=osx64 release

# Debug: Prüfe was make sieht
make PLATFORM=osx64 --debug=v | grep -i platform
```

#### Problem: m4 Template-Fehler

```
m4: cannot open pkg/osx/Info.plist.m4
```

**Lösung:** macOS-spezifische Template-Dateien fehlen für Bundle-Creation.
Dies kann ignoriert werden wenn du `.dylib` statt `.bundle` verwendest.

#### Problem: Python-Script Fehler

```
./project/info.py: Permission denied
```

**Lösung:**
```bash
chmod +x project/info.py
make PLATFORM=osx64 release
```

### Allgemeine Tipps

#### Sauberer Build

Bei Problemen immer zuerst clean build versuchen:

```bash
make clean
make PLATFORM=osx64 release
```

#### Verbose Output

Für Debugging mehr Informationen anzeigen:

```bash
make PLATFORM=osx64 release VERBOSE=1
```

#### Debug-Build

Falls Release-Build Probleme macht:

```bash
make PLATFORM=osx64 debug
```

#### Dependencies prüfen

```bash
# Auf macOS: Prüfe ob alle Tools vorhanden
which clang
which ar
which ranlib
which m4
which python3

# Auf Ubuntu mit OSXCross:
which x86_64-apple-darwin21-clang
which x86_64-apple-darwin21-ar
```

## 10. Weiterführende Links

### Offizielle Dokumentation

- **GitHub Actions:** https://docs.github.com/en/actions
  - macOS Runners: https://docs.github.com/en/actions/using-github-hosted-runners/about-github-hosted-runners
  - Workflow Syntax: https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions

- **Apple Developer:** https://developer.apple.com
  - Xcode Downloads: https://developer.apple.com/download/
  - Code-Signing Guide: https://developer.apple.com/support/code-signing/

### Tools und Libraries

- **OSXCross:** https://github.com/tpoechtrager/osxcross
  - Installation Guide: https://github.com/tpoechtrager/osxcross#installation
  - FAQ: https://github.com/tpoechtrager/osxcross#faq

- **Homebrew (macOS Package Manager):** https://brew.sh

### Cloud-Anbieter

- **MacStadium:** https://www.macstadium.com
  - Pricing: https://www.macstadium.com/pricing

- **MacinCloud:** https://www.macincloud.com
  - Plans: https://www.macincloud.com/pricing

- **AWS EC2 Mac Instances:** https://aws.amazon.com/ec2/instance-types/mac/
  - Getting Started: https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/ec2-mac-instances.html

### Xmod-spezifisch

- **Build-System Dokumentation:** [notes/BuildSystem.txt](https://github.com/jay1110/xmod/blob/master/notes/BuildSystem.txt)
- **Allgemeine Build-Anleitung:** [BUILD.md](BUILD.md)
- **Visual Studio Build:** [BUILD_VS2022.md](BUILD_VS2022.md)
- **GitHub Repository:** https://github.com/jay1110/xmod

### Cross-Compilation

- **Cross-Compilation Guide (General):** https://clang.llvm.org/docs/CrossCompilation.html
- **Apple SDK License:** https://www.apple.com/legal/sla/

### Community

- **Enemy Territory Community:** https://www.etlegacy.com
- **Xmod Issues:** https://github.com/jay1110/xmod/issues

---

## Zusammenfassung

Für die meisten Benutzer ist **GitHub Actions** (Weg 1) die beste Wahl:
- ✅ Kostenlos
- ✅ Einfach einzurichten
- ✅ Zuverlässig
- ✅ Automatisch bei jedem Release

**OSXCross** ist nur für Experten empfohlen und sollte nicht für Production verwendet werden.

Bei Fragen oder Problemen öffne ein Issue auf GitHub: https://github.com/jay1110/xmod/issues
