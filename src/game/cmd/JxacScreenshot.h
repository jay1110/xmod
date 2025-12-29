#ifndef GAME_CMD_JXACSCREENSHOT_H
#define GAME_CMD_JXACSCREENSHOT_H

///////////////////////////////////////////////////////////////////////////////

class JxacScreenshot : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    JxacScreenshot();
    ~JxacScreenshot();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_JXACSCREENSHOT_H
