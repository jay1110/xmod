#include <bgame/impl.h>
#include <game/xmod_globals.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

UserList::UserList()
    : AbstractBuiltin( "userlist" )
{
    __usage << xvalue( "!" + _name )
           << ' ' << _ovalue( "-name NAME" );

    __descr << "List users in database.";
}

///////////////////////////////////////////////////////////////////////////////

UserList::~UserList()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
UserList::doExecute( Context& txt )
{
    if (!::xmod::g_database || !::xmod::g_database->isOpened()) {
        txt._ebuf << "Database not available.";
        return PA_ERROR;
    }

    std::vector<::xmod::UserData> users;
    if (!::xmod::g_database->getUserList(users) || users.empty()) {
        txt._ebuf << "The user database is empty.";
        return PA_ERROR;
    }

    string nameFilter;

    // parse filter options
    {
        const vector<string>::size_type max = txt._args.size();
        for ( vector<string>::size_type i = 1; i < max; i++ ) {
            // pairs of args are expected
            if ((max - i) < 2)
                return PA_USAGE;

            string s = txt._args[i];
            str::toLower( s );

            if (s == "-name") {
                nameFilter = txt._args[++i];
                str::toLower( nameFilter );
            }
            else {
                return PA_USAGE;
            }
        }
    }

    InlineText cID    = xheader;
    InlineText cName  = xheader;
    InlineText cLevel = xheader;
    InlineText cWhen  = xheader;

    cID.flags    |= ios::left;
    cName.flags  |= ios::left;
    cLevel.flags |= ios::left;
    cWhen.flags  |= ios::left;

    cID.width    = 8;
    cName.width  = 25;
    cLevel.width = 18;

    cName.prefixOutside  = ' ';
    cLevel.prefixOutside = ' ';
    cWhen.prefixOutside  = ' ';

    Buffer buf;
    buf << cID    ( "ID" )
        << cName  ( "NAME" )
        << cLevel ( "LEVEL" )
        << cWhen  ( "SEEN" );

    cID.color    = xcnone;
    cName.color  = xcnone;
    cLevel.color = xcnone;
    cWhen.color  = xcnone;

    string tmp;

    uint32 num = 0;
    for (std::vector<::xmod::UserData>::const_iterator it = users.begin(); it != users.end(); ++it) {
        const ::xmod::UserData& user = *it;
        
        ostringstream idStream;
        idStream << user.id;
        string id = idStream.str();

        if (!nameFilter.empty()) {
            tmp = user.name;
            str::toLower( tmp );
            if (tmp.find( nameFilter ) == string::npos)
                continue;
        }

        if (++num > (Page::maxLines * Page::maxPages))
            break;

        buf << '\n'
            << cID    ( id )
            << cName  ( user.name );

        string err;
        Level& lev = levelDB.fetchByKey( user.level, err );
        if (lev.namex.empty())
            buf << cLevel( lev.level );
        else
            buf << cLevel( str::etAlignLeft( lev.namex, cLevel.width, tmp ));

        char ftbuf[32];
		strftime( ftbuf, sizeof(ftbuf), "%a %b %d %H:%M:%S", localtime( &user.lastSeen ));
        buf << cWhen ( ftbuf );
    }

    Page::report( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
