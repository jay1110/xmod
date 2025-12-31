#!/bin/bash
#
# Migration script to convert legacy user.db to SQLite xmod.db
# 
# This script helps migrate user data from the old text-based user.db format
# to the new SQLite database format (xmod.db).
#
# Usage:
#   ./migrate_userdb.sh [path_to_mod_directory]
#
# Example:
#   ./migrate_userdb.sh ~/.etwolf/xmod
#   ./migrate_userdb.sh /opt/etserver/xmod
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}xmod Database Migration Script${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Determine the mod directory
if [ $# -eq 1 ]; then
    MOD_DIR="$1"
else
    # Try to auto-detect common ET server paths
    if [ -d "$HOME/.etwolf/xmod" ]; then
        MOD_DIR="$HOME/.etwolf/xmod"
    elif [ -d "/opt/etserver/xmod" ]; then
        MOD_DIR="/opt/etserver/xmod"
    else
        echo -e "${RED}Error: Could not auto-detect mod directory${NC}"
        echo "Usage: $0 [path_to_mod_directory]"
        echo "Example: $0 ~/.etwolf/xmod"
        exit 1
    fi
fi

# Verify the directory exists
if [ ! -d "$MOD_DIR" ]; then
    echo -e "${RED}Error: Directory not found: $MOD_DIR${NC}"
    exit 1
fi

echo -e "${GREEN}Mod directory: $MOD_DIR${NC}"
echo ""

# Check for legacy user.db
LEGACY_DB="$MOD_DIR/user.db"
SQLITE_DB="$MOD_DIR/xmod.db"

if [ ! -f "$LEGACY_DB" ]; then
    echo -e "${YELLOW}Warning: Legacy user.db not found at: $LEGACY_DB${NC}"
    echo "Nothing to migrate."
    exit 0
fi

echo -e "${GREEN}Found legacy database: $LEGACY_DB${NC}"

# Check if SQLite database exists
if [ -f "$SQLITE_DB" ]; then
    echo -e "${YELLOW}SQLite database already exists: $SQLITE_DB${NC}"
    
    # Count users in SQLite database
    USER_COUNT=$(sqlite3 "$SQLITE_DB" "SELECT COUNT(*) FROM users;" 2>/dev/null || echo "0")
    echo -e "${BLUE}Current SQLite user count: $USER_COUNT${NC}"
    
    echo ""
    echo -e "${YELLOW}Migration options:${NC}"
    echo "1. The server can automatically migrate users on startup if SQLite is empty"
    echo "2. You can manually trigger migration with the !dbload migrate command in-game"
    echo "3. You can backup and delete xmod.db to force a fresh migration"
    echo ""
    read -p "Do you want to backup and delete xmod.db for fresh migration? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        BACKUP_DB="$SQLITE_DB.backup.$(date +%Y%m%d_%H%M%S)"
        echo -e "${BLUE}Backing up SQLite database to: $BACKUP_DB${NC}"
        cp "$SQLITE_DB" "$BACKUP_DB"
        rm "$SQLITE_DB"
        echo -e "${GREEN}SQLite database deleted. Will be recreated on next server start.${NC}"
    else
        echo -e "${BLUE}Keeping existing SQLite database.${NC}"
        echo "Use !dbload migrate command in-game to sync with legacy database."
    fi
else
    echo -e "${BLUE}SQLite database not found - will be created on server startup${NC}"
    echo -e "${GREEN}Automatic migration will run when server starts${NC}"
fi

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}Migration preparation complete!${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "Next steps:"
echo "1. Start your ET server with xmod"
echo "2. Watch console for [SQLite] migration messages"
echo "3. Or use !dbload migrate command in-game for manual migration"
echo ""

exit 0
