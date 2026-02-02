#ifndef GAME_CMD_CUSTOMCOMMAND_H
#define GAME_CMD_CUSTOMCOMMAND_H

///////////////////////////////////////////////////////////////////////////////

#include <set>

///////////////////////////////////////////////////////////////////////////////

class CustomCommand : public AbstractCommand
{
public:
    CustomCommand( const string& name, const string& exec, const string& desc, const set<int>& levels );
    ~CustomCommand();

    bool hasPermission( const Context& txt );

protected:
    PostAction doExecute( Context& txt );

private:
    string substituteVariables( const string& str, Context& txt );
    
    string      _exec;     // command to execute
    set<int>    _levels;   // admin levels that have access
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_CUSTOMCOMMAND_H
