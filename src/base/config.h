#ifndef BASE_CONFIG_H
#define BASE_CONFIG_H

///////////////////////////////////////////////////////////////////////////////

#if defined( XMOD_LINUX ) || defined( XMOD_LINUX64 ) || defined( XMOD_LINUX_AARCH64 )
#    include <base/linux/public.h>
#elif defined( XMOD_MINGW ) || defined( XMOD_MINGW64 )
#    include <base/mingw/public.h>
#elif defined( XMOD_OSX ) || defined( XMOD_OSX64 ) || defined( XMOD_OSX_ARM64 )
#    include <base/osx/public.h>
#elif defined( XMOD_WINDOWS ) || defined( XMOD_WINDOWS64 )
#    include <base/windows/public.h>
#elif defined( XMOD_ANDROID_ARM64 ) || defined( XMOD_ANDROID_ARMV7A ) || defined( XMOD_ANDROID_X86_64 ) || defined( XMOD_ANDROID_X86 )
#    include <base/linux/public.h>
#elif defined( XMOD_WASM ) || defined( __EMSCRIPTEN__ )
#    include <base/linux/public.h>
#else
#    error "XMOD platform is not defined."
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined( __i386__ )
#    define XMOD_LITTLE_ENDIAN
#elif defined( __x86_64__ ) || defined( _M_X64 )
#    define XMOD_LITTLE_ENDIAN
#elif defined( __aarch64__ ) || defined( __arm64__ )
#    define XMOD_LITTLE_ENDIAN
#elif  defined( __ppc__ )
#    define XMOD_BIG_ENDIAN
#else
#    define XMOD_LITTLE_ENDIAN
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined( __GNUC__ ) || defined( __clang__ )
#    define XMOD_FUNCTION __PRETTY_FUNCTION__
#elif defined( _MSC_VER )
#    define XMOD_FUNCTION __FUNCTION__
#else
#    define XMOD_FUNCTION __FUNCTION__
#endif

///////////////////////////////////////////////////////////////////////////////

#define USE_MDXFILE

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_CONFIG_H
