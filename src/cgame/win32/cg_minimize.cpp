#include <bgame/impl.h>
#include <windows.h>

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

void CG_AddMinimizeButton( void ) {
    char buffer[64];
    const char *WindowClassName = "Enemy Territory";
    trap_Cvar_VariableStringBuffer( "win_hinstance", buffer, sizeof( buffer ) );
    HINSTANCE etHandle = reinterpret_cast<HINSTANCE>( atoll( buffer ) );
    HWND wnd = NULL;
    while ( ( wnd = FindWindowEx( NULL, wnd, WindowClassName, WindowClassName ) ) != NULL ) {
        HINSTANCE hInst = reinterpret_cast<HINSTANCE>( GetWindowLongPtr( wnd, GWLP_HINSTANCE ) );
        if ( etHandle == hInst ) {
            LONG_PTR style = GetWindowLongPtr( wnd, GWL_STYLE );
            SetWindowLongPtr( wnd, GWL_STYLE, style | WS_MINIMIZEBOX );
            break;
        }
    }
}
