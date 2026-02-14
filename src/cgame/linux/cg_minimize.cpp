#include <bgame/impl.h>

// Linux window managers already provide minimize buttons in title bar
void CG_AddMinimizeButton( void ) {
}

void CG_MinimizeWindow( void ) {
    // On Linux, use the engine's minimize command
    extern void trap_SendConsoleCommand( const char *text );
    trap_SendConsoleCommand( "minimize\n" );
}
