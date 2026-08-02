#ifndef BGAME_ENGINE_H
#define BGAME_ENGINE_H

///////////////////////////////////////////////////////////////////////////////

/* Static class for global ET engine functions.
 *
 * This class should only contain functions common to both game/cgame.
 * This class is static and cgame/game must implement Engine.cpp.
 */
class Engine
{
public:
#if defined( __EMSCRIPTEN__ )
    // WebAssembly's call_indirect requires exact signature matching, and
    // variadic function pointers do not work across MAIN_MODULE/SIDE_MODULE
    // boundaries (they trap with "indirect call signature mismatch" or "table
    // index is out of bounds"). The engine therefore hands us a non-variadic,
    // array-based syscall entry point. This wrapper packs the trap arguments
    // into a contiguous array so existing call sites -- Engine::ptr(cmd, ...) --
    // keep working unchanged.
    typedef intptr_t (QDECL* Ptr)( intptr_t* );

    class Caller
    {
    public:
        Ptr fn;

        Caller& operator=( Ptr p ) { fn = p; return *this; }

        intptr_t operator()( intptr_t cmd ) const
        {
            intptr_t a[1] = { cmd };
            return fn( a );
        }

        template< typename... Args >
        intptr_t operator()( intptr_t cmd, Args... args ) const
        {
            intptr_t a[] = { cmd, (intptr_t)( args )... };
            return fn( a );
        }
    };

    static Caller ptr;
#else
    typedef intptr_t (QDECL* Ptr)( intptr_t, ... );

    static Ptr ptr;
#endif

    static size_t argc ( );
    static size_t argl ( string& );
    static size_t args ( vector<string>& );
    static size_t args ( vector<string>&, const string& );
    static size_t argv ( size_t, string& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_ENGINE_H
