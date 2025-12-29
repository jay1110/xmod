#ifndef GAME_CMD_JXACSTATUS_H
#define GAME_CMD_JXACSTATUS_H

///////////////////////////////////////////////////////////////////////////////

class JxacStatus : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    JxacStatus();
    ~JxacStatus();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_JXACSTATUS_H
