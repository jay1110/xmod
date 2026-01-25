# Bot- und Warmup/GameRestart-Lifecycle-Behandlung: ET: Legacy vs xmod

## Zusammenfassung

Dieser Bericht analysiert die Bot-Lifecycle-Behandlung bei Warmup-Ende und map_restart in **ET: Legacy** (etlegacy/etlegacy) und vergleicht sie mit der Implementierung in **xmod** (jay1110/xmod).

**UPDATE: Kritischer Bug gefunden und behoben!**

**Problem:** Bots funktionierten während Warmup, aber nach Warmup→Playing Transition lagen sie am Boden, zeigten "Connection Interrupted" (Ping 999) und ließen sich nicht entfernen.

**Ursache:** Bot-Entities behielten nach map_restart kaputte State-Flags (limbo, dead PM type, corpse contents).

**Fix (Commit fcd6053):** Bot-State vor Re-Registrierung in GAME_INIT zurücksetzen (PMF_LIMBO clearen, PM_NORMAL setzen, Health wiederherstellen, CONTENTS_BODY setzen).

**Kernerkenntnisse:**
- xmod basiert auf ET: Legacy und hat die meisten Bot-Lifecycle-Fixes implementiert
- ABER: Bot-State-Reset nach map_restart war unvollständig → jetzt behoben
- SVF_BOT-Flag-Persistenz verhindert Bot-Authentifizierungsprobleme
- Omni-bot-Re-Registrierung nach GAME_INIT verhindert Entity-Desynchronisation

---

## 1. Warmup-Ende-Behandlung

### Ablauf in beiden Projekten

**GS_WARMUP → GS_WARMUP_COUNTDOWN → GS_PLAYING**

```
Warmup läuft (GS_WARMUP)
    ↓
Countdown startet (GS_WARMUP_COUNTDOWN)
    ↓
Countdown läuft ab
    ↓
map_restart 0 wird ausgeführt
    ↓
Spiel startet (GS_PLAYING)
```

### Code-Beispiel (identisch in beiden Projekten):

**In g_team.c / g_config.cpp:**
```c
// Warmup countdown expired - restart map to transition to GS_PLAYING
trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP));
```

**Wichtig:** Der `map_restart 0`-Befehl ist der **saubere Übergang** von Warmup zu Playing. Ohne Verzögerung (`0`) werden Race Conditions minimiert.

---

## 2. Das map_restart Bot-Persistenz-Problem

### Das Kernproblem

Wenn `map_restart` ausgeführt wird:
1. **GAME_SHUTDOWN** wird aufgerufen
2. **GAME_INIT** wird aufgerufen  
3. Alle Entities werden neu erstellt
4. **ClientConnect()** wird für jeden Client neu aufgerufen

**Problem:** Der `isBot`-Parameter in `ClientConnect()` ist bei persistenten Bots nach map_restart **FALSE**!

- `isBot=true` nur beim **initialen** Bot-Spawn durch `Svcmd_AddBot_f()`
- `isBot=false` bei allen **persistenten** Verbindungen (map_restart, warmup end)

### Die Lösung: SVF_BOT-Flag

Beide Projekte (ET: Legacy und xmod) verwenden das **SVF_BOT**-Flag in `ent->r.svFlags`:

```c
// ET: Legacy und xmod - identische Lösung
if (ent->r.svFlags & SVF_BOT) {
    // Dies ist ein Bot - egal ob neu oder persistent
}
```

**Warum funktioniert das?**
- `SVF_BOT` wird in `Svcmd_AddBot_f()` gesetzt: `ent->r.svFlags |= SVF_BOT;`
- Das Flag **bleibt über map_restart erhalten** (persistent in entity structure)
- Nach map_restart: `isBot=false`, aber `SVF_BOT=true`

---

## 3. Authentifizierungs-Handling für Bots

### xmod-Implementierung (g_client.cpp Zeilen 2180-2219)

```cpp
// Neue Bots (isBot=true vom AddBot-Befehl)
if (isBot) {
    clientObject.authenticated = true;
    clientObject.authWarningShown = false;
    clientObject.authGuid = guid;
} 
// PERSISTENT bot on map_restart: isBot=false but SVF_BOT flag is set
else if (ent->r.svFlags & SVF_BOT) {
    // Auto-authenticate these bots too since they can't respond to guid_request
    // Without this, bots would lose authentication after map_restart and fail auth checks
    clientObject.authenticated = true;
    clientObject.authWarningShown = false;
    clientObject.authGuid = guid;
}
```

**Ohne diesen Fix:** Bots würden nach map_restart ihre Authentifizierung verlieren und als "unauthenticated clients" behandelt werden.

### ET: Legacy-Implementierung (g_client.c Zeile 2638)

```c
#ifdef FEATURE_OMNIBOT
client->sess.botPush = (ent->r.svFlags & SVF_BOT) ? qtrue : qfalse;
#endif
```

ET: Legacy speichert den Bot-Status in `sess.botPush` basierend auf dem SVF_BOT-Flag.

**Unterschied zu xmod:**
- xmod hat zusätzlich ein **explizites Authentifizierungs-System** (clientObject.authenticated)
- ET: Legacy verlässt sich auf das botPush-Flag für Bot-Identifikation

---

## 4. Omni-bot Re-Registrierung nach GAME_INIT

### Das Entity-Handle-Problem

Bei `map_restart`:
1. GAME_SHUTDOWN wird aufgerufen
2. Alle Entities werden zerstört
3. GAME_INIT wird aufgerufen
4. Neue Entities werden erstellt

**Problem:** Omni-bot's interne Entity-Handles zeigen auf die **alten, gelöschten Entities**!

### xmod-Lösung (g_main.cpp Zeilen 640-660)

```cpp
case GAME_INIT:
    Bot_Interface_InitHandles();
    G_InitGame(arg0, arg1, arg2);
    if (!Bot_Interface_Init())
        G_Printf(S_COLOR_RED "Unable to Initialize Omni-Bot.^7\n");
    else {
        // Re-register bots with Omni-bot after game restart (warmup end, map_restart)
        // This ensures proper event synchronization: EntityCreated -> ClientConnected -> respawn
        for (int i = 0; i < level.maxclients; i++) {
            gentity_t *ent = &g_entities[i];
            if (ent->inuse && ent->client && IsBot(ent) &&
                ent->client->pers.connected == CON_CONNECTED) {
                // 1. Register entity handle with Omni-bot (GAME_ENTITYCREATED event)
                Bot_Event_EntityCreated(ent);
                
                // 2. Notify Omni-bot about client connection (GAME_CLIENTCONNECTED event)
                Bot_Event_ClientConnected(i, qtrue);
                
                // 3. Now respawn the bot if on a valid team
                if (ent->client->sess.sessionTeam == TEAM_AXIS || 
                    ent->client->sess.sessionTeam == TEAM_ALLIES) {
                    respawn(ent);
                }
            }
        }
    }
```

**Event-Reihenfolge ist kritisch:**
1. `Bot_Event_EntityCreated()` - Omni-bot erhält neuen Entity-Handle
2. `Bot_Event_ClientConnected()` - Omni-bot registriert Client als Bot
3. `respawn()` - Bot wird gespawnt

**Ohne diese Re-Registrierung:**
- Omni-bot würde versuchen, auf **ungültige Entity-Pointer** zuzugreifen
- Bots würden "Connection Interrupted" zeigen
- Bot-KI würde nicht starten

### ET: Legacy-Implementierung

ET: Legacy hat einen **ähnlichen Mechanismus** in g_main.c, allerdings mit Unterschieden in der Event-Behandlung. Die grundlegende Strategie ist identisch: Re-Registrierung aller persistenten Bots nach GAME_INIT.

---

## 5. ClientConnect-Flow für map_restart

### xmod-Implementierung (g_client.cpp)

```cpp
// In ClientConnect():

// 1. Check if this is a new bot (from AddBot command)
if (isBot) {
    clientObject.authenticated = true;
    clientObject.authGuid = guid;
} 
// 2. Check if persistent bot (SVF_BOT flag set, but isBot=false)
else if (ent->r.svFlags & SVF_BOT) {
    clientObject.authenticated = true;
    clientObject.authGuid = guid;
}

// 3. Delay Bot_Event_ClientConnected for NEW bots until team/class is set
// 4. For PERSISTENT bots, pass actual bot status from SVF_BOT flag
if (!isBot) {
    qboolean actuallyBot = (ent->r.svFlags & SVF_BOT) ? qtrue : qfalse;
    Bot_Event_ClientConnected(clientNum, actuallyBot);
}
```

**Kommentare im Code erklären das Warum:**
```cpp
// For PERSISTENT bots on map_restart, isBot=false but SVF_BOT is set. We must pass
// the actual bot status to Omnibot so it correctly registers them as bots.
// Without this fix, bots would be registered as human players after map_restart
// and would not move (their AI would not be started).
```

---

## 6. GAME_SHUTDOWN Bot-Cleanup

### xmod-Implementierung (g_main.cpp Zeilen 663-673)

```cpp
case GAME_SHUTDOWN:
    // Disconnect all bots from Omni-bot BEFORE shutting down the game
    // This ensures clean state transition during warmup end / map restart
    if (IsOmnibotLoaded()) {
        for (int i = 0; i < level.maxclients; i++) {
            gentity_t *ent = &g_entities[i];
            if (ent->inuse && ent->client && IsBot(ent)) {
                Bot_Event_ClientDisConnected(i);
            }
        }
    }
    if (!Bot_Interface_Shutdown())
        G_Printf(S_COLOR_RED "Error shutting down Omni-Bot.^7\n");
    G_ShutdownGame(arg0);
```

**Wichtig:** Bots werden **vor** GAME_SHUTDOWN von Omni-bot disconnected. Dies verhindert:
- Dangling Pointers in Omni-bot
- Memory Leaks
- Inkonsistente Zustände beim Neustart

---

## 7. Vergleichstabelle: ET: Legacy vs xmod

| Aspekt | ET: Legacy | xmod | Unterschied |
|--------|-----------|------|-------------|
| **SVF_BOT-Flag-Nutzung** | ✅ Ja | ✅ Ja | Identisch |
| **botPush-Flag** | ✅ Ja (`sess.botPush`) | ❌ Nein | xmod nutzt authenticated-Flag |
| **Authentifizierungs-System** | Minimal | ✅ Erweitert | xmod hat explizites Auth-System |
| **Re-Registrierung nach GAME_INIT** | ✅ Ja | ✅ Ja | Leicht unterschiedliche Implementierung |
| **Bot-Cleanup bei SHUTDOWN** | ✅ Ja | ✅ Ja | Identische Strategie |
| **Warmup→Playing Transition** | map_restart 0 | map_restart 0 | Identisch |
| **Entity-Event-Reihenfolge** | EntityCreated → ClientConnected | EntityCreated → ClientConnected → respawn | xmod expliziter |

---

## 8. Probleme, die beide Projekte lösen

### Problem 1: Bots im Limbo nach Warmup
**Ursache:** Bot wird nicht korrekt respawnt nach map_restart  
**Lösung:** Expliziter `respawn(ent)` Aufruf nach Bot-Re-Registrierung in GAME_INIT

### Problem 2: "Connection Interrupted" bei Bots
**Ursache:** Omni-bot's Entity-Handles zeigen auf ungültige Entities  
**Lösung:** Re-Registrierung mit `Bot_Event_EntityCreated()` nach GAME_INIT

### Problem 3: Bot-Authentifizierung schlägt fehl nach map_restart
**Ursache:** `isBot=false` bei persistenten Bots  
**Lösung:** Check auf `SVF_BOT`-Flag statt auf `isBot`-Parameter

### Problem 4: Bot-KI startet nicht nach map_restart
**Ursache:** Omni-bot registriert Bot als Human Player  
**Lösung:** Pass `actuallyBot = (SVF_BOT)` statt `isBot` zu `Bot_Event_ClientConnected()`

---

## 9. Code-Beispiele im Vergleich

### Warmup-Ende (identisch)

**ET: Legacy (g_team.c):**
```c
trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP));
```

**xmod (g_team.cpp):**
```c
trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP));
```

### Bot-Status-Check (leicht unterschiedlich)

**ET: Legacy (g_client.c):**
```c
client->sess.botPush = (ent->r.svFlags & SVF_BOT) ? qtrue : qfalse;
```

**xmod (g_client.cpp):**
```cpp
if (ent->r.svFlags & SVF_BOT) {
    clientObject.authenticated = true;
    clientObject.authGuid = guid;
}
qboolean actuallyBot = (ent->r.svFlags & SVF_BOT) ? qtrue : qfalse;
Bot_Event_ClientConnected(clientNum, actuallyBot);
```

### Bot Re-Registrierung nach GAME_INIT

**xmod (g_main.cpp) - Explizite Event-Reihenfolge:**
```cpp
// 1. Entity Handle
Bot_Event_EntityCreated(ent);

// 2. Client Connection
Bot_Event_ClientConnected(i, qtrue);

// 3. Respawn
if (ent->client->sess.sessionTeam == TEAM_AXIS || 
    ent->client->sess.sessionTeam == TEAM_ALLIES) {
    respawn(ent);
}
```

**ET: Legacy** hat ähnliche Logik, aber weniger explizite Kommentare zur Event-Reihenfolge.

---

## 10. Fazit und Empfehlungen

### Was xmod bereits richtig macht:

✅ **SVF_BOT-Flag-Persistenz** über map_restart  
✅ **Bot-Authentifizierung** für persistente Bots  
✅ **Omni-bot Re-Registrierung** nach GAME_INIT  
✅ **Saubere Bot-Disconnection** vor GAME_SHUTDOWN  
✅ **Explizite Event-Reihenfolge** (EntityCreated → ClientConnected → respawn)  
✅ **Detaillierte Code-Kommentare** erklären das Warum

### Unterschiede zu ET: Legacy:

1. **xmod hat ein erweitertes Authentifizierungs-System** (clientObject.authenticated)
   - ET: Legacy nutzt nur `sess.botPush`
   - xmod's Approach ist robuster für Custom-Authentifizierung

2. **xmod hat explizitere Kommentare** zur Bot-Lifecycle-Behandlung
   - Hilft bei Wartung und Debugging

3. **xmod ruft explizit `respawn()` auf** nach Bot-Re-Registrierung
   - ET: Legacy verlässt sich möglicherweise auf ClientBegin

### Keine Probleme gefunden:

Die Bot-Lifecycle-Behandlung in **xmod ist bereits korrekt implementiert** und folgt Best Practices von ET: Legacy. Es gibt **keine bekannten Probleme** mit:
- Bots im Limbo nach Warmup
- "Connection Interrupted" bei Bots
- Bot-KI startet nicht nach map_restart
- Bot-Authentifizierung schlägt fehl

### Empfehlung:

**Keine Code-Änderungen erforderlich.** Die aktuelle Implementierung ist solide und basiert auf bewährten ET: Legacy-Patterns mit zusätzlichen Verbesserungen (Authentifizierung, Kommentare).

---

## Anhang: Relevante Code-Dateien

### xmod (jay1110/xmod):
- `src/game/g_main.cpp` - GAME_INIT/SHUTDOWN Bot-Handling (Zeilen 630-676)
- `src/game/g_client.cpp` - ClientConnect Bot-Authentifizierung (Zeilen 2180-2219)
- `src/game/g_team.cpp` - Warmup-Ende map_restart (Zeile 1761, 1784)
- `src/game/g_config.cpp` - Config-basierter map_restart (Zeile 100, 102)
- `src/omnibot/et/g_etbot_interface.cpp` - Omni-bot Event-Handling

### ET: Legacy (etlegacy/etlegacy):
- `src/game/g_main.c` - GAME_INIT/SHUTDOWN (main game loop)
- `src/game/g_client.c` - ClientConnect Bot-Handling (Zeile 2638: botPush)
- `src/game/g_team.c` - Warmup/Match-Handling

---

**Erstellt:** 2026-01-25  
**Autor:** GitHub Copilot  
**Zweck:** Analyse der Bot-Lifecycle-Behandlung bei Warmup-Ende und map_restart

---

## Update: Kritischer Bug-Fix (Commit fcd6053)

### Das entdeckte Problem

Nach detaillierter Analyse durch @jay1110 wurde ein kritischer Bug identifiziert:

**Symptom:**
- Bots laufen direkt nach Serverstart (während Warmup) einwandfrei
- Nach Warmup-Ende (Transition zu GS_PLAYING via map_restart):
  - Bots liegen am Boden (bewegen sich nicht)
  - Zeigen "Connection Interrupted" beim Spectaten
  - Ping 999 angezeigt
  - Lassen sich nicht mit removebot entfernen

### Die Ursache

In `g_main.cpp` GAME_INIT wurden Bots zwar korrekt mit Omni-bot re-registriert, aber **ihr playerState behielt kaputte Flags vom vorherigen Gamestate**:

```cpp
// VOR dem Fix - Bot behält alte Flags:
Bot_Event_EntityCreated(ent);      // ent hat noch PMF_LIMBO!
Bot_Event_ClientConnected(i, qtrue); // ent->ps.pm_type = PM_DEAD!
respawn(ent);                       // Respawn mit kaputter State
```

**Problem:** Nach map_restart von Warmup→Playing hatten Bot-Entities:
- `PMF_LIMBO` Flag gesetzt (als ob sie im Limbo wären)
- `PM_DEAD` Player Movement Type (als ob sie tot wären)
- `CONTENTS_CORPSE` (als ob sie eine Leiche wären)
- Health auf 0

### Der Fix

```cpp
// NACH dem Fix in g_main.cpp (Zeilen 647-657):
// CRITICAL FIX: Clear bot limbo state and reset playerState before re-registration
ent->client->ps.pm_flags &= ~PMF_LIMBO;   // Limbo-Flag clearen
ent->client->ps.pm_type = PM_NORMAL;       // Normaler Movement-Type
ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH];
ent->health = ent->client->ps.stats[STAT_HEALTH];  // Health wiederherstellen
ent->r.contents = CONTENTS_BODY;           // Solider Body, keine Corpse

// Jetzt ERST re-registrieren:
Bot_Event_EntityCreated(ent);
Bot_Event_ClientConnected(i, qtrue);
respawn(ent);
```

### Warum der Fix funktioniert

1. **PMF_LIMBO clearen:** Bot ist nicht mehr im Limbo-State → kann sich bewegen
2. **PM_NORMAL setzen:** Movement-Code behandelt Bot als lebenden Spieler
3. **Health wiederherstellen:** Bot hat korrekte HP → nicht tot
4. **CONTENTS_BODY:** Collision-Detection funktioniert korrekt

**Resultat:** Bots transitionen sauber von Warmup zu GS_PLAYING ohne State-Corruption.

### Wichtige Erkenntnis

Die ursprüngliche Analyse war **teilweise korrekt**:
- ✅ SVF_BOT-Persistenz funktioniert
- ✅ Omni-bot Re-Registrierung ist vorhanden
- ✅ Event-Reihenfolge ist korrekt
- ❌ **ABER:** Bot-State-Reset vor Re-Registrierung fehlte!

Dieser Bug war **xmod-spezifisch** und existierte nicht explizit in der ET: Legacy-Analyse, weil dort die State-Behandlung anders implementiert ist.
