#include <bgame/impl.h>
#include <game/xmod_globals.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

BanList::BanList()
    : AbstractBuiltin( "banlist" )
{
    __usage << xvalue( "!" + _name )
            << ' ' << _ovalue( "-name NAME" );

    __descr << "List banned users.";
}

///////////////////////////////////////////////////////////////////////////////

BanList::~BanList()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
BanList::doExecute( Context& txt )
{
    if (!xmod::g_database || !xmod::g_database->isOpened()) {
        txt._ebuf << "Database not available.";
        return PA_ERROR;
    }

    std::vector<xmod::BanData> bans;
    if (!xmod::g_database->getBanList(bans) || bans.empty()) {
        txt._ebuf << "There are no banned users.";
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

    InlineText cID      = xheader;
    InlineText cWhen    = xheader;
    InlineText cSubject = xheader;
    InlineText cRemain  = xheader;
    InlineText cAuth    = xheader;

    cID.flags      |= ios::left;
    cWhen.flags    |= ios::left;
    cSubject.flags |= ios::left;

    cID.width      = 8;
    cWhen.width    = 24;
    cSubject.width = 25;
    cRemain.width  = 13;

    cWhen.prefixOutside    = ' ';
    cSubject.prefixOutside = ' ';
    cRemain.prefixOutside  = ' ';
    cAuth.prefixOutside    = ' ';

    Buffer buf;
    buf << cID      ( "ID" )
        << cWhen    ( "WHEN" )
        << cSubject ( "SUBJECT" )
        << cRemain  ( "REMAINING" )
        << cAuth    ( "AUTHORITY" );

    cID.color      = xcnone;
    cWhen.color    = xcnone;
    cSubject.color = xcnone;
    cRemain.color  = xcnone;
    cAuth.color    = xcnone;

    const time_t now = time( NULL );
    string tmp;

    uint32 numExpired = 0;
    uint32 num = 0;
    
    for (std::vector<xmod::BanData>::const_iterator it = bans.begin(); it != bans.end(); ++it) {
        const xmod::BanData& ban = *it;
        
        // Generate ID from ban ID or GUID
        ostringstream idStream;
        idStream << ban.id;
        string id = idStream.str();

        const time_t deltaTime = ban.expires - now;

        // skip if ban has expired (non-permanent bans)
        if (ban.expires != 0 && deltaTime < 0) {
            numExpired++;
            continue;
        }

        if (!nameFilter.empty()) {
            tmp = ban.name;
            str::toLower( tmp );
            if (tmp.find( nameFilter ) == string::npos)
                continue;
        }

        if (++num > (Page::maxLines * Page::maxPages))
            break;

        buf << '\n'
            << cID      ( id )
            << cWhen    ( ban.ban_date )
            << cSubject ( ban.name );

        ostringstream remain;
        if (ban.expires == 0) {
            remain << "permanent";
        }
        else {
            int secs = (ban.expires - now);

            int days = secs / (60*60*24);
            secs -= (days * (60*60*24));

            int hours = secs / (60*60);
            secs -= (hours * (60*60));

            int mins = secs / 60;
            secs -= (mins * 60);

            if (days > 999)
                remain << "999d-";
            else if (days > 0)
                remain << days << "d-";

            remain << setw(2) << setfill('0') << hours
                   << ':' << setw(2) << setfill('0') << mins
                   << ':' << setw(2) << setfill('0') << secs;
        }

        buf << cRemain ( remain.str() )
            << cAuth   ( ban.banned_by );
    }

    Page::report( txt._client, buf );

    if (numExpired) {
        buf.reset();
        buf << xcheader << "--there were " << xvalue( numExpired ) << " expired-bans not listed.";
        print( txt._client, buf );
    }

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
