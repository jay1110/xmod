#ifndef GAME_CMD_JXACRELOAD_H
#define GAME_CMD_JXACRELOAD_H

class JxacReload : public AbstractBuiltin
{
protected:
    PostAction doExecute(Context&);

public:
    JxacReload();
};

#endif
