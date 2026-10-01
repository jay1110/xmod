#ifndef GAME_CMD_STATS_H
#define GAME_CMD_STATS_H

class Stats : public AbstractBuiltin {
protected:
    PostAction doExecute(Context&);
public:
    Stats();
};

#endif
