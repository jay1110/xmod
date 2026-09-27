# Admin level after map change — 2026-09-25

## Cause

OnClientCommand(authenticate) called Session::onGuidReceived while connectedUsers
still pointed to the temporary PENDING user. Session loaded the SQLite level and
mute data onto that object. The command handler then replaced connectedUsers
with UserManager::fetchByKey(realGuid), which creates an in-memory user with
level 0 when no cached user exists. SQLite itself retained the correct level.
ClientUserinfoChanged can later copy the session level back into the runtime
user, concealing the bug depending on event ordering.

## Fix

Session::guidReceived now resolves and binds the confirmed GUID before applying
SQLite data. This covers the command handler and the session-restoration callers.
The command handler no longer replaces the synchronized user afterward.
Authentication success flags and logging are emitted only after database
integration succeeds. The log now includes the applied level. Failed database
or ban checks stop the handler before XP restoration or success reporting.

## Regression coverage

The CTest auth_mapchange target links the same production game objects used by
qagame, with an engine syscall stub and an isolated in-memory SQLite database.
It checks eight fresh GUIDs, alternating CON_CONNECTING and CON_CONNECTED:
- runtime GUID, session level, User level and actual admin privilege immediately
  after the authenticate command, without another userinfo update or respawn;
- mute synchronization and unchanged database admin level;
- no admin data applied to the temporary PENDING user;
- repeated authentication and direct session restoration with User::BAD;
- invalid/duplicate GUIDs, ignored bot commands, banned account and closed DB.

This exercises the real auth/session/database code, not a live ClientBegin,
map load or Omni-Bot rotation. A running-server map-rotation test is still pending.

## Results

- Before: old HEAD xm_main_ext.cpp, xmod_session.cpp/.h, g_client.cpp and
  xmod_globals.h compiled into the same x64 regression host (remaining objects
  unchanged). Test exits 1: FAIL: level immediately restored.
- After: Windows x64 Release build passes; CTest 2/2 passes.
- After: Windows x86 Release build passes; CTest 2/2 passes.
- Both architectures also pass windows_module_load for game/cgame/ui exports.
- Existing GitHub Actions CTest steps automatically run this new regression test.
- git diff --check passes. No remote changes or workflow dispatch.

## Local evidence

- build/auth-before/CMakeLists.txt: baseline host recipe and original snapshots.
- build/auth-before-build.log and build/auth-before-test.log: negative control.
- build/auth-x64-build.log and build/auth-x64-test.log: fixed x64 verification.
- build/auth-x86-build.log and build/auth-x86-test.log: fixed x86 verification.
- build/msvc-x64/game/Release/qagame_mp_x64.dll
- build/msvc-x86/game/Release/qagame_mp_x86.dll

Production changes are platform-independent. Linux was not rebuilt in this run.
