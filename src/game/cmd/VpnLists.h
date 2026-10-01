#ifndef GAME_CMD_VPNLISTS_H
#define GAME_CMD_VPNLISTS_H

// Shared authentication requirement in addition to each command's own ACL.
class VpnListCommand : public AbstractBuiltin
{
protected:
    explicit VpnListCommand(const char* name);
public:
    bool hasPermission(const Context&);
};

class VpnIpList : public VpnListCommand
{
    bool _blacklist;
protected:
    VpnIpList(const char* name, bool blacklist);
    PostAction doExecute(Context&);
};

class Whitelist : public VpnIpList
{
public:
    Whitelist();
};

class Blacklist : public VpnIpList
{
public:
    Blacklist();
};

class NguidList : public VpnListCommand
{
protected:
    PostAction doExecute(Context&);
public:
    NguidList();
};

class VpnCheck : public VpnListCommand
{
protected:
    PostAction doExecute(Context&);
public:
    VpnCheck();
};

#endif
