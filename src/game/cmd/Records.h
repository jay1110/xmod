#ifndef GAME_CMD_RECORDS_H
#define GAME_CMD_RECORDS_H

class Records : public AbstractBuiltin {
protected:
    PostAction doExecute(Context&);
public:
    Records();
};

#endif
