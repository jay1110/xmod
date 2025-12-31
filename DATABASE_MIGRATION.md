# Database Migration Guide

This guide explains how to migrate from the legacy text-based `user.db` format to the new SQLite `xmod.db` database.

## Overview

xmod now uses SQLite for user data storage instead of the legacy text-based format. The SQLite database provides:
- Better performance and reliability
- Consistent behavior between 32-bit and 64-bit builds
- Improved data integrity
- Support for concurrent access

## Database Location

The SQLite database is stored at:
```
<fs_homepath>/<fs_game>/xmod.db
```

For example:
- Linux: `~/.etwolf/xmod/xmod.db`
- Windows: `C:\Users\<username>\Documents\WolfET\xmod\xmod.db`

This ensures the database is in the mod folder, not the root directory.

## Automatic Migration

The server automatically migrates users from `user.db` to SQLite on startup if:
1. The SQLite database (`xmod.db`) is empty or doesn't exist
2. The legacy database (`user.db`) contains users

You'll see migration messages in the console:
```
[SQLite] Starting migration from legacy userDB...
[SQLite] Imported user: PlayerName (ID=1, level=5)
[SQLite] Migration complete: 150 imported, 0 updated, 5 skipped
```

## Manual Migration

### Option 1: In-Game Command

Use the admin command to manually trigger migration:
```
!dbload migrate
```

This is useful if:
- SQLite database already has some users and you want to sync
- Automatic migration didn't run
- You want to re-import after making changes to user.db

### Option 2: Migration Script

Run the provided shell script (Linux only):
```bash
./migrate_userdb.sh ~/.etwolf/xmod
```

The script will:
- Detect your mod directory
- Check for existing databases
- Offer to backup and reset SQLite for fresh migration
- Provide instructions for next steps

## What Gets Migrated

The migration process imports:
- User GUID (primary identifier)
- Player name
- Admin level (authLevel)
- Hardware IDs (HWIDs)
- Muted status

## Troubleshooting

### Database is Empty

Check console logs for `[SQLite]` messages to see if migration ran:
```
[SQLite] Database will be created in mod folder: /path/to/xmod.db
[SQLite] Starting migration from legacy userDB...
```

If no migration happened:
1. Verify `user.db` exists in the mod folder
2. Try manual migration: `!dbload migrate`
3. Check file permissions

### 32-bit vs 64-bit Issues

Both architectures now use the same database file path. If you had separate databases:
1. Stop both servers
2. Backup both `xmod.db` files
3. Delete both `xmod.db` files
4. Start the server (pick one architecture)
5. It will migrate from `user.db` creating a fresh `xmod.db`
6. Both 32-bit and 64-bit will now use this same file

### Migration Failed

If migration fails:
1. Check console for error messages
2. Verify SQLite database is writable
3. Check disk space
4. Backup and delete `xmod.db` to force fresh creation

## Reverting to Legacy Database

If you need to revert to the old system:
1. This is **not recommended** as SQLite is more reliable
2. You would need to modify code to re-enable `userDB.load()`
3. Consider reporting issues instead of reverting

## Commands Reference

- `!dbload` - Reload database files and show user/ban counts
- `!dbload migrate` - Manually trigger migration from legacy user.db
- `!dbsave` - Save level database (SQLite auto-saves user data)

## File Reference

- `xmod.db` - SQLite database (new format)
- `user.db` - Legacy text database (old format)
- `level.db` - Level definitions (still text format)
- `map.db` - Map records (still text format)

## Support

If you encounter issues:
1. Check console logs for `[SQLite]` messages
2. Verify database file paths and permissions
3. Try manual migration with `!dbload migrate`
4. Report issues with console log excerpts
