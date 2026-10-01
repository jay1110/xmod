#ifndef GAME_CMD_ANTIRUSH_H
#define GAME_CMD_ANTIRUSH_H
class AntiRush : public AbstractBuiltin {
protected:
    PostAction doExecute(Context&);
public:
    AntiRush();
    bool hasPermission(const Context&);
};
#endif
