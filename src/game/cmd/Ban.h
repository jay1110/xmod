#ifndef GAME_CMD_BAN_H
#define GAME_CMD_BAN_H

///////////////////////////////////////////////////////////////////////////////

class Ban : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    Ban();
    ~Ban();

public:
    static void doBan( User&, User&, int, const string&, Buffer&, const Client* = NULL );
    static void doBanSlot( int targetSlot, const string& authorityName, int duration, const string& reason, Buffer& out );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_BAN_H
