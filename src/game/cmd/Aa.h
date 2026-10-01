#ifndef GAME_CMD_AA_H
#define GAME_CMD_AA_H
class Aa : public AbstractBuiltin {
protected:
    PostAction doExecute(Context&);
public:
    Aa();
    bool hasPermission(const Context&);
};
#endif
