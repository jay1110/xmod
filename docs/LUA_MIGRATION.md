# Lua Script Migration Guide for xmod

This guide explains how to migrate Lua scripts from jaymod/nitmod/ETPro to xmod.

## Table of Contents

- [Overview](#overview)
- [Key API Changes](#key-api-changes)
- [GUID Functions](#guid-functions)
- [File I/O Operations](#file-io-operations)
- [Admin Level Functions](#admin-level-functions)
- [Quick Reference Tables](#quick-reference-tables)
- [Complete Migration Example](#complete-migration-example)

---

## Overview

xmod's Lua API is based on ETLegacy's implementation with some differences from older mods like jaymod, nitmod, and ETPro. The main differences are:

1. **No standard `io` library** - Use `et.trap_FS_*` functions instead
2. **Different GUID system** - Use `et.G_GetClientGuid(clientNum)` instead of userinfo fields
3. **Enhanced security** - More validation and bounds checking
4. **Backward compatibility** - Many jaymod/shrubbot functions work via aliases

---

## Key API Changes

### ❌ NOT Available in xmod

The following standard Lua functions are **disabled for security**:

```lua
-- THESE DO NOT WORK IN XMOD:
io.open()
io.read()
io.write()
io.close()
io.lines()
file:read()
file:write()
file:lines()
loadfile()
dofile() -- Use with caution, restricted
```

### ✅ Available in xmod

xmod provides secure alternatives through the `et` API:

```lua
-- File operations
et.trap_FS_FOpenFile(filename, mode)
et.trap_FS_Read(fd, count)
et.trap_FS_Write(data, count, fd)
et.trap_FS_FCloseFile(fd)
et.trap_FS_Rename(oldname, newname)

-- Client information
et.G_GetClientGuid(clientNum)
et.G_GetClientLevel(clientNum)  -- Modern name
et.G_shrubbot_level(clientNum)  -- jaymod/shrubbot alias
et.G_GetClientName(clientNum)
```

---

## GUID Functions

### Old Way (jaymod/nitmod)

```lua
-- jaymod/nitmod used different GUID systems:
local info = et.trap_GetUserinfo(clientNum)
local n_guid = et.Info_ValueForKey(info, "n_guid")      -- nitmod GUID
local authguid = et.Info_ValueForKey(info, "authguid")  -- May not exist
```

### New Way (xmod)

```lua
-- xmod provides a unified GUID function:
local guid = et.G_GetClientGuid(clientNum)
```

**How it works:**
- Returns **40-character authGuid** if player is authenticated (preferred)
- Falls back to **32-character cl_guid** if not authenticated
- Always returns a valid string (empty string if invalid clientNum)

**Example:**

```lua
function et_ClientCommand(clientNum, cmd)
  -- Get GUID the xmod way
  local guid = et.G_GetClientGuid(clientNum)
  
  -- Get player name
  local name = et.G_GetClientName(clientNum)
  
  -- Use them
  if playerDB[guid] then
    et.G_Print(name .. " is registered with GUID: " .. guid .. "\n")
  end
end
```

---

## File I/O Operations

### File Mode Constants

```lua
local FS_READ = 0         -- Read mode
local FS_WRITE = 1        -- Write mode (truncates existing file)
local FS_APPEND = 2       -- Append mode
local FS_APPEND_SYNC = 3  -- Append with sync
```

### Opening Files

#### Old Way (jaymod/nitmod)

```lua
-- THIS DOES NOT WORK IN XMOD:
local file = io.open("myfile.txt", "r")
if file then
  -- use file
  file:close()
end
```

#### New Way (xmod)

```lua
local FS_READ = 0

local fd, len = et.trap_FS_FOpenFile("myfile.txt", FS_READ)
if fd >= 0 then
  -- fd is valid file descriptor
  -- len is file length in bytes
  et.trap_FS_FCloseFile(fd)
else
  et.G_Print("Error: Could not open file\n")
end
```

### Reading Files

#### Old Way (jaymod/nitmod)

```lua
-- THIS DOES NOT WORK IN XMOD:
local file = io.open("data.txt", "r")
if file then
  for line in file:lines() do
    -- process line
  end
  file:close()
end
```

#### New Way (xmod)

```lua
local FS_READ = 0

local fd, len = et.trap_FS_FOpenFile("data.txt", FS_READ)
if fd >= 0 and len > 0 then
  -- Read entire file content
  local content = et.trap_FS_Read(fd, len)
  et.trap_FS_FCloseFile(fd)
  
  -- Process line by line
  for line in string.gmatch(content, "[^\r\n]+") do
    -- process each line
  end
end
```

### Writing Files

#### Old Way (jaymod/nitmod)

```lua
-- THIS DOES NOT WORK IN XMOD:
local file = io.open("output.txt", "w")
if file then
  file:write("Line 1\n")
  file:write("Line 2\n")
  file:close()
end
```

#### New Way (xmod)

```lua
local FS_WRITE = 1

local data = "Line 1\nLine 2\n"
local fd = et.trap_FS_FOpenFile("output.txt", FS_WRITE)
if fd >= 0 then
  et.trap_FS_Write(data, string.len(data), fd)
  et.trap_FS_FCloseFile(fd)
else
  et.G_Print("Error: Could not write to file\n")
end
```

### Appending to Files

```lua
local FS_APPEND = 2

local newData = "New line\n"
local fd = et.trap_FS_FOpenFile("log.txt", FS_APPEND)
if fd >= 0 then
  et.trap_FS_Write(newData, string.len(newData), fd)
  et.trap_FS_FCloseFile(fd)
end
```

---

## Admin Level Functions

### Old Way (jaymod/shrubbot)

```lua
local level = et.G_shrubbot_level(clientNum)
```

### New Way (xmod)

Both of these work in xmod:

```lua
-- Modern xmod name:
local level = et.G_GetClientLevel(clientNum)

-- Backward compatible (jaymod/shrubbot) alias:
local level = et.G_shrubbot_level(clientNum)
```

**Both functions:**
- Return the player's admin level (0 for regular players)
- Use xmod's session-aware authentication system
- Check authenticated sessions first, fall back to User system

---

## Quick Reference Tables

### GUID Migration

| Old (jaymod/nitmod) | New (xmod) |
|---------------------|------------|
| `et.Info_ValueForKey(info, "n_guid")` | `et.G_GetClientGuid(clientNum)` |
| `et.Info_ValueForKey(info, "authguid")` | `et.G_GetClientGuid(clientNum)` |
| `et.Info_ValueForKey(info, "cl_guid")` | `et.G_GetClientGuid(clientNum)` |

### File I/O Migration

| Old (io library) | New (xmod API) |
|------------------|----------------|
| `io.open(file, "r")` | `et.trap_FS_FOpenFile(file, 0)` |
| `io.open(file, "w")` | `et.trap_FS_FOpenFile(file, 1)` |
| `io.open(file, "a")` | `et.trap_FS_FOpenFile(file, 2)` |
| `file:read("*a")` | `et.trap_FS_Read(fd, len)` |
| `file:write(data)` | `et.trap_FS_Write(data, len, fd)` |
| `file:lines()` | `string.gmatch(content, "[^\r\n]+")` |
| `file:close()` | `et.trap_FS_FCloseFile(fd)` |

### Admin Level Migration

| Old | New |
|-----|-----|
| `et.G_shrubbot_level(cn)` | `et.G_GetClientLevel(cn)` or `et.G_shrubbot_level(cn)` |
| Custom level systems | Use xmod's User/Session system |

---

## Complete Migration Example

Here's a complete example showing how to migrate a typical jaymod/nitmod Lua script to xmod.

### BEFORE (jaymod/nitmod - Does NOT work in xmod)

```lua
-- Old script using io.open() and n_guid
local rulesDB = "etcrules.db"
local acceptedPlayers = {}

function loadRulesDB()
  local file = io.open(rulesDB, "r")
  if file then
    for line in file:lines() do
      local guid, name, flag = line:match("^(.-)|(.-)|([01])$")
      if guid and flag == "1" then
        acceptedPlayers[guid] = true
      end
    end
    file:close()
  end
end

function savePlayerAcceptance(clientNum)
  local info = et.trap_GetUserinfo(clientNum)
  local guid = et.Info_ValueForKey(info, "n_guid")  -- Wrong for xmod!
  local name = et.Info_ValueForKey(info, "name")
  
  acceptedPlayers[guid] = true
  
  local file = io.open(rulesDB, "a")  -- Wrong for xmod!
  if file then
    file:write(string.format("%s|%s|1\n", guid, name))
    file:close()
  end
end

function checkPlayerAcceptance(clientNum)
  local info = et.trap_GetUserinfo(clientNum)
  local guid = et.Info_ValueForKey(info, "n_guid")  -- Wrong for xmod!
  
  return acceptedPlayers[guid] == true
end
```

### AFTER (xmod - Correct)

```lua
-- Migrated script for xmod using et.trap_FS_* and et.G_GetClientGuid
local rulesDB = "etcrules.db"
local acceptedPlayers = {}

-- File mode constants
local FS_READ = 0
local FS_WRITE = 1
local FS_APPEND = 2

function loadRulesDB()
  local fd, len = et.trap_FS_FOpenFile(rulesDB, FS_READ)
  if fd >= 0 and len > 0 then
    local content = et.trap_FS_Read(fd, len)
    et.trap_FS_FCloseFile(fd)
    
    -- Parse line by line
    for line in string.gmatch(content, "[^\r\n]+") do
      local guid, name, flag = line:match("^(.-)|(.-)|([01])$")
      if guid and flag == "1" then
        acceptedPlayers[guid] = true
      end
    end
  end
end

function savePlayerAcceptance(clientNum)
  -- Use xmod's GUID function
  local guid = et.G_GetClientGuid(clientNum)
  local name = et.G_GetClientName(clientNum)
  
  acceptedPlayers[guid] = true
  
  -- Use xmod's file API
  local data = string.format("%s|%s|1\n", guid, name)
  local fd = et.trap_FS_FOpenFile(rulesDB, FS_APPEND)
  if fd >= 0 then
    et.trap_FS_Write(data, string.len(data), fd)
    et.trap_FS_FCloseFile(fd)
  else
    et.G_Print("ERROR: Could not save player acceptance\n")
  end
end

function checkPlayerAcceptance(clientNum)
  -- Use xmod's GUID function
  local guid = et.G_GetClientGuid(clientNum)
  
  return acceptedPlayers[guid] == true
end
```

---

## Advanced File Operations

### Reading and Updating a Database File

This pattern is common for player databases where you need to update existing entries:

```lua
local FS_READ = 0
local FS_WRITE = 1

function updatePlayerDatabase(dbPath, playerGuid, playerName, newData)
  local lines = {}
  local found = false
  
  -- Read existing file
  local fd, len = et.trap_FS_FOpenFile(dbPath, FS_READ)
  if fd >= 0 and len > 0 then
    local content = et.trap_FS_Read(fd, len)
    et.trap_FS_FCloseFile(fd)
    
    -- Parse and update
    for line in string.gmatch(content, "[^\r\n]+") do
      local guid = line:match("^(.-)|")
      if guid == playerGuid then
        -- Replace this player's line
        table.insert(lines, string.format("%s|%s|%s", playerGuid, playerName, newData))
        found = true
      else
        -- Keep other lines
        table.insert(lines, line)
      end
    end
  end
  
  -- Add new entry if not found
  if not found then
    table.insert(lines, string.format("%s|%s|%s", playerGuid, playerName, newData))
  end
  
  -- Write back to file
  local newContent = table.concat(lines, "\n") .. "\n"
  fd = et.trap_FS_FOpenFile(dbPath, FS_WRITE)
  if fd >= 0 then
    et.trap_FS_Write(newContent, string.len(newContent), fd)
    et.trap_FS_FCloseFile(fd)
    return true
  else
    et.G_Print("ERROR: Could not write to " .. dbPath .. "\n")
    return false
  end
end

-- Usage:
function et_ClientCommand(clientNum, cmd)
  if cmd == "register" then
    local guid = et.G_GetClientGuid(clientNum)
    local name = et.G_GetClientName(clientNum)
    
    if updatePlayerDatabase("players.db", guid, name, "registered") then
      et.trap_SendServerCommand(clientNum, "print \"^2Registration successful!^7\n\"")
    end
  end
end
```

---

## Common Pitfalls and Solutions

### Pitfall 1: File Handle Not Checked

❌ **Wrong:**
```lua
local fd = et.trap_FS_FOpenFile("file.txt", FS_WRITE)
et.trap_FS_Write(data, len, fd)  -- May crash if fd < 0
et.trap_FS_FCloseFile(fd)
```

✅ **Correct:**
```lua
local fd = et.trap_FS_FOpenFile("file.txt", FS_WRITE)
if fd >= 0 then
  et.trap_FS_Write(data, len, fd)
  et.trap_FS_FCloseFile(fd)
else
  et.G_Print("ERROR: Could not open file\n")
end
```

### Pitfall 2: Using Wrong GUID Source

❌ **Wrong:**
```lua
local info = et.trap_GetUserinfo(clientNum)
local guid = et.Info_ValueForKey(info, "n_guid")  -- Doesn't exist in xmod!
```

✅ **Correct:**
```lua
local guid = et.G_GetClientGuid(clientNum)  -- Works in xmod
```

### Pitfall 3: Not Closing File Handles

❌ **Wrong:**
```lua
local fd = et.trap_FS_FOpenFile("file.txt", FS_READ)
if fd >= 0 then
  local content = et.trap_FS_Read(fd, len)
  -- Missing: et.trap_FS_FCloseFile(fd)
end
```

✅ **Correct:**
```lua
local fd = et.trap_FS_FOpenFile("file.txt", FS_READ)
if fd >= 0 then
  local content = et.trap_FS_Read(fd, len)
  et.trap_FS_FCloseFile(fd)  -- Always close!
end
```

### Pitfall 4: Using file:lines() Pattern

❌ **Wrong:**
```lua
local file = io.open("file.txt", "r")  -- Doesn't work in xmod
for line in file:lines() do
  -- process
end
```

✅ **Correct:**
```lua
local fd, len = et.trap_FS_FOpenFile("file.txt", FS_READ)
if fd >= 0 and len > 0 then
  local content = et.trap_FS_Read(fd, len)
  et.trap_FS_FCloseFile(fd)
  
  for line in string.gmatch(content, "[^\r\n]+") do
    -- process
  end
end
```

---

## Full Working Example: Rules Acceptance System

Here's your complete migrated script:

```lua
-- rules.lua - xmod compatible version
-- Handles persistent rule reading and acceptance for members

dofile(basePath .. "core.lua")

assert(rulesDBPath, "[rules.lua] 'rulesDBPath' is nil. Make sure config.lua is loaded first.")

local acceptedPlayers = {}  -- [guid] = true  (flag == 1)
local acceptedRead    = {}  -- [guid] = true  (entry exists, flag 0 or 1)

-- File mode constants
local FS_READ = 0
local FS_WRITE = 1
local FS_APPEND = 2

-- Called on mod init
function Rules_InitGame()
  local fd, len = et.trap_FS_FOpenFile(rulesDBPath, FS_READ)
  if fd >= 0 and len > 0 then
    local content = et.trap_FS_Read(fd, len)
    et.trap_FS_FCloseFile(fd)
    
    for line in string.gmatch(content, "[^\r\n]+") do
      local guidAndName, flag = line:match("^(.-)|([01])$")
      if guidAndName and flag then
        local sep = guidAndName:find("|", 1, true)
        if sep then
          local guid = guidAndName:sub(1, sep - 1)
          local name = guidAndName:sub(sep + 1)
          acceptedRead[guid] = true
          if flag == "1" then
            acceptedPlayers[guid] = true
          end
        end
      end
    end
  else
    -- Create empty file if doesn't exist
    fd = et.trap_FS_FOpenFile(rulesDBPath, FS_WRITE)
    if fd >= 0 then
      et.trap_FS_FCloseFile(fd)
    end
  end
  logDebug("Rules_InitGame loaded etcrules.db")
end

-- Save accepted entry (flag = 1)
local function saveAcceptedPlayer(guid, name)
  acceptedPlayers[guid] = true
  acceptedRead[guid] = true

  local temp = {}
  
  -- Read existing entries
  local fd, len = et.trap_FS_FOpenFile(rulesDBPath, FS_READ)
  if fd >= 0 and len > 0 then
    local content = et.trap_FS_Read(fd, len)
    et.trap_FS_FCloseFile(fd)
    
    for line in string.gmatch(content, "[^\r\n]+") do
      local lineGuid = line:match("^(.-)|")
      if lineGuid and lineGuid ~= guid then
        table.insert(temp, line)
      end
    end
  end
  
  -- Add new entry
  table.insert(temp, string.format("%s|%s|1", guid, name))
  
  -- Write back
  local newContent = table.concat(temp, "\n") .. "\n"
  fd = et.trap_FS_FOpenFile(rulesDBPath, FS_WRITE)
  if fd >= 0 then
    et.trap_FS_Write(newContent, string.len(newContent), fd)
    et.trap_FS_FCloseFile(fd)
    logCommand(name .. " accepted rules (saved)")
  else
    et.G_Print("ERROR: Could not save to " .. rulesDBPath .. "\n")
  end
end

-- Save read-only status (flag = 0) if no accepted entry exists
local function saveReadOnlyStatus(guid, name)
  acceptedRead[guid] = true
  if not acceptedPlayers[guid] then
    local data = string.format("%s|%s|0\n", guid, name)
    local fd = et.trap_FS_FOpenFile(rulesDBPath, FS_APPEND)
    if fd >= 0 then
      et.trap_FS_Write(data, string.len(data), fd)
      et.trap_FS_FCloseFile(fd)
      logCommand(name .. " viewed rules (/etcrules)")
    end
  end
end

-- Overwrite entry for GUID with new flag (0 or 1)
local function overwriteEntry(guid, name, flag)
  local temp = {}
  
  local fd, len = et.trap_FS_FOpenFile(rulesDBPath, FS_READ)
  if fd >= 0 and len > 0 then
    local content = et.trap_FS_Read(fd, len)
    et.trap_FS_FCloseFile(fd)
    
    for line in string.gmatch(content, "[^\r\n]+") do
      local lineGuid = line:match("^(.-)|")
      if lineGuid and lineGuid ~= guid then
        table.insert(temp, line)
      end
    end
  end
  
  table.insert(temp, string.format("%s|%s|%s", guid, name, flag))
  
  local newContent = table.concat(temp, "\n") .. "\n"
  fd = et.trap_FS_FOpenFile(rulesDBPath, FS_WRITE)
  if fd >= 0 then
    et.trap_FS_Write(newContent, string.len(newContent), fd)
    et.trap_FS_FCloseFile(fd)
  end

  -- Update state
  if flag == "1" then
    acceptedPlayers[guid] = true
    acceptedRead[guid] = true
  else
    acceptedPlayers[guid] = nil
    acceptedRead[guid] = true
  end
end

-- Find ONLINE client by name fragment
local function findClientByName(partial)
  partial = string.lower(partial)
  for i = 0, tonumber(et.trap_Cvar_Get("sv_maxclients")) - 1 do
    if et.gentity_get(i, "inuse") then
      local guid = et.G_GetClientGuid(i)  -- xmod way
      local name = et.G_GetClientName(i)
      if name and string.find(string.lower(name), partial, 1, true) then
        return guid, name
      end
    end
  end
end

-- Search database for name (OFFLINE support)
local function findOfflineByName(partial)
  partial = string.lower(partial)
  
  local fd, len = et.trap_FS_FOpenFile(rulesDBPath, FS_READ)
  if fd < 0 then
    return nil, nil, "none"
  end
  
  if len == 0 then
    et.trap_FS_FCloseFile(fd)
    return nil, nil, "none"
  end

  local content = et.trap_FS_Read(fd, len)
  et.trap_FS_FCloseFile(fd)
  
  local foundGuid, foundName
  local count = 0

  for line in string.gmatch(content, "[^\r\n]+") do
    local guid, name, flag = line:match("^(.-)|(.-)|([01])$")
    if guid and name then
      if string.find(string.lower(name), partial, 1, true) then
        foundGuid, foundName = guid, name
        count = count + 1
      end
    end
  end

  if count == 0 then
    return nil, nil, "none"
  elseif count == 1 then
    return foundGuid, foundName, "ok"
  else
    return nil, nil, "multiple"
  end
end

-- On userinfo change
function Rules_ClientUserinfoChanged(clientNum)
  local guid = et.G_GetClientGuid(clientNum)  -- xmod way
  local level = et.G_shrubbot_level(clientNum)  -- Works in xmod

  if level >= et.minLvL and level <= et.maxLvL then
    if not (acceptedPlayers[guid] and acceptedRead[guid]) then
      if et.gentity_get(clientNum, "sess.sessionTeam") ~= 3 then
        et.gentity_set(clientNum, "sess.sessionTeam", 3)
        et.trap_SendServerCommand(clientNum,
          'cp "^1You must read the rules with ^3/etcrules ^1and accept them with ^3/accept before joining a team.^7"')
        logDebug("Blocked team join: client " .. clientNum .. " has not accepted rules")
      end
    end
  end
end

-- Handle rule-related commands
function Rules_ClientCommand(clientNum, cmd)
  cmd = string.lower(cmd)
  local level = et.G_shrubbot_level(clientNum)  -- Works in xmod
  local guid = et.G_GetClientGuid(clientNum)     -- xmod way
  local name = et.G_GetClientName(clientNum)     -- xmod way

  -- /etcrules
  if cmd == "etcrules" then
    if level >= et.minLvL and level <= et.maxLvL then
      for _, rule in ipairs(etcrules) do
        et.trap_SendServerCommand(clientNum, "print \"" .. rule .. "^7\n\"")
      end
      saveReadOnlyStatus(guid, name)
      logCommand(name .. " used /etcrules")
    else
      et.trap_SendServerCommand(clientNum,
        "print \"^1Access Denied: You must be level " .. et.minLvL .. " or higher.^7\n\"")
      logCommand(name .. " tried /etcrules without required level")
    end
    return 1

  -- /accept
  elseif cmd == "accept" then
    if level >= et.minLvL and level <= et.maxLvL then
      if acceptedPlayers[guid] then
        et.trap_SendServerCommand(clientNum, 'print "^3You have already accepted the rules.^7\n"')
        logCommand(name .. " used /accept but was already accepted")
      else
        if acceptedRead[guid] then
          saveAcceptedPlayer(guid, name)
          et.trap_SendServerCommand(clientNum,
            'cp "^2Thank you! You have accepted the rules. You may now join a team.^7"')
        else
          et.trap_SendServerCommand(clientNum,
            'cp "^1You must first read the rules with ^3/etcrules^7"')
        end
      end
    else
      et.trap_SendServerCommand(clientNum,
        'print "^1Access Denied: This command is for members level ' ..
          et.minLvL .. ' to ' .. et.maxLvL .. '^7\n"')
      logCommand(name .. " tried /accept without required member level")
    end
    return 1

  -- /addaccept [player]
  elseif cmd == "addaccept" then
    if allowedAdminLevels[level] then
      local arg = string.lower(et.trap_Argv(1) or "")
      local targetGuid, targetName

      if arg == "" then
        targetGuid = guid
        targetName = name
      else
        targetGuid, targetName = findClientByName(arg)
        if not targetGuid then
          local guidOff, nameOff, status = findOfflineByName(arg)
          if status == "multiple" then
            et.trap_SendServerCommand(clientNum,
              'print "^1Multiple matches found in rules database. Please be more specific.^7\n"')
            return 1
          elseif status == "ok" then
            targetGuid, targetName = guidOff, nameOff
          end
        end
      end

      if not targetGuid or not targetName or targetGuid == "" then
        et.trap_SendServerCommand(clientNum,
          'print "^1Player not found (online or in rules database).^7\n"')
        return 1
      end

      if acceptedPlayers[targetGuid] then
        et.trap_SendServerCommand(clientNum,
          'print "^3' .. targetName .. ' ^7has already accepted the rules.^7\n"')
        logCommand(name .. " tried /addaccept but " .. targetName .. " already accepted")
        return 1
      end

      saveAcceptedPlayer(targetGuid, targetName)
      et.trap_SendServerCommand(clientNum,
        'print "^2Rules accepted entry saved for ^7' .. targetName .. '^2.^7\n"')
      logCommand(name .. " used /addaccept for " .. targetName)
    end
    return 1

  -- /forgetaccept [player]
  elseif cmd == "forgetaccept" then
    if allowedAdminLevels[level] then
      local arg = string.lower(et.trap_Argv(1) or "")
      local targetGuid, targetName

      if arg == "" then
        targetGuid = guid
        targetName = name
      else
        targetGuid, targetName = findClientByName(arg)
        if not targetGuid then
          local guidOff, nameOff, status = findOfflineByName(arg)
          if status == "multiple" then
            et.trap_SendServerCommand(clientNum,
              'print "^1Multiple matches found in rules database. Please be more specific.^7\n"')
            return 1
          elseif status == "ok" then
            targetGuid, targetName = guidOff, nameOff
          end
        end
      end

      if not targetGuid or not targetName or targetGuid == "" then
        et.trap_SendServerCommand(clientNum,
          'print "^1Player not found (online or in rules database).^7\n"')
        return 1
      end

      local hasRecord = acceptedRead[targetGuid]
      local isAccepted = acceptedPlayers[targetGuid] and true or false

      if not hasRecord then
        et.trap_SendServerCommand(clientNum,
          'print "^3No acceptance entry stored for ^7' .. targetName .. '^3.^7\n"')
        logCommand(name .. " tried /forgetaccept for " .. targetName .. " but no entry exists")
        return 1
      end

      if not isAccepted then
        et.trap_SendServerCommand(clientNum,
          'print "^3Acceptance already reset for ^7' .. targetName .. '^3.^7\n"')
        logCommand(name .. " tried /forgetaccept for " .. targetName .. " but already reset")
        return 1
      end

      overwriteEntry(targetGuid, targetName, "0")
      et.trap_SendServerCommand(clientNum,
        'print "^3Acceptance reset for ^7' .. targetName .. '^3.^7\n"')
      logCommand(name .. " used /forgetaccept for " .. targetName)
    end
    return 1

  -- /checkaccept [player]
  elseif cmd == "checkaccept" then
    if allowedAdminLevels[level] then
      local arg = string.lower(et.trap_Argv(1) or "")
      local targetGuid, targetName

      if arg == "" then
        targetGuid = guid
        targetName = name
      else
        targetGuid, targetName = findClientByName(arg)
        if not targetGuid then
          local guidOff, nameOff, status = findOfflineByName(arg)
          if status == "multiple" then
            et.trap_SendServerCommand(clientNum,
              'print "^1Multiple matches found in rules database. Please be more specific.^7\n"')
            return 1
          elseif status == "ok" then
            targetGuid, targetName = guidOff, nameOff
          end
        end
      end

      if not targetGuid or not targetName or targetGuid == "" then
        et.trap_SendServerCommand(clientNum,
          'print "^1Player not found (online or in rules database).^7\n"')
        return 1
      end

      local read = acceptedRead[targetGuid] and "yes" or "no"
      local accepted = acceptedPlayers[targetGuid] and "yes" or "no"

      et.trap_SendServerCommand(clientNum,
        string.format('print "^3Player ^7%s ^3- Read: ^7%s ^3- Accepted: ^7%s^7\n"',
          targetName, read, accepted))

      logCommand(name .. " used /checkaccept on " .. targetName ..
        " (read=" .. read .. ", accepted=" .. accepted .. ")")
    end
    return 1
  end

  return 0
end
```

---

## Additional Resources

- **xmod Source Code**: Check `src/game/g_lua.cpp` for all available Lua API functions
- **File I/O Functions**: Lines 439-499 in `g_lua.cpp`
- **GUID Functions**: Lines 1627-1649 in `g_lua.cpp`
- **Admin Level Functions**: Lines 1596-1611 in `g_lua.cpp`

---

## Summary Checklist

When migrating Lua scripts to xmod:

- [ ] Replace `io.open()` with `et.trap_FS_FOpenFile()`
- [ ] Replace `file:read()` with `et.trap_FS_Read()`
- [ ] Replace `file:write()` with `et.trap_FS_Write()`
- [ ] Replace `file:close()` with `et.trap_FS_FCloseFile()`
- [ ] Replace `file:lines()` with `string.gmatch(content, "[^\r\n]+")`
- [ ] Replace `n_guid` or `authguid` from userinfo with `et.G_GetClientGuid(clientNum)`
- [ ] Use `et.G_GetClientName(clientNum)` instead of parsing userinfo manually
- [ ] Use `et.G_GetClientLevel(clientNum)` or `et.G_shrubbot_level(clientNum)` for admin levels
- [ ] Always check if file descriptor `>= 0` before using
- [ ] Always close file descriptors with `et.trap_FS_FCloseFile()`
- [ ] Define file mode constants (FS_READ=0, FS_WRITE=1, FS_APPEND=2)

---

**Last Updated**: March 2026  
**Version**: xmod 2.0.4
