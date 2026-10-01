#ifndef GAME_CMD_VPN_H
#define GAME_CMD_VPN_H

class Vpn : public AbstractBuiltin
{
protected:
    PostAction doExecute(Context&);

public:
    Vpn();
    ~Vpn();
    bool hasPermission(const Context&);
};

#endif // GAME_CMD_VPN_H
