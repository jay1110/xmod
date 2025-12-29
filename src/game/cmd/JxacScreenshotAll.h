#ifndef GAME_CMD_JXACSCREENSHOTALL_H
#define GAME_CMD_JXACSCREENSHOTALL_H

///////////////////////////////////////////////////////////////////////////////

class JxacScreenshotAll : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    JxacScreenshotAll();
    ~JxacScreenshotAll();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_JXACSCREENSHOTALL_H
