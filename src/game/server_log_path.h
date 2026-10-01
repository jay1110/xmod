#ifndef GAME_SERVER_LOG_PATH_H
#define GAME_SERVER_LOG_PATH_H

#include <string>

namespace serverlog {
// Relative log filenames belong to fs_homepath/fs_game, regardless of the
// process working directory. Explicit absolute custom filenames remain valid.
// Resolves the name and creates its parent directories; an empty name disables
// logging and returns false without creating anything.
bool resolve(const std::string& configured, std::string& physical);
}

#endif
