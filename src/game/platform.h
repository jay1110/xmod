#ifndef GAME_PLATFORM_H
#define GAME_PLATFORM_H

///////////////////////////////////////////////////////////////////////////////
// Platform Detection and Includes
///////////////////////////////////////////////////////////////////////////////

// Android Platform
#if defined(ANDROID) || defined(__ANDROID__)

    #include <android/log.h>
    
    // Android logging macros
    #define PLATFORM_LOG_DEBUG(tag, ...) \
        __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)
    
    #define PLATFORM_LOG_INFO(tag, ...) \
        __android_log_print(ANDROID_LOG_INFO, tag, __VA_ARGS__)
    
    #define PLATFORM_LOG_ERROR(tag, ...) \
        __android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__)
    
    #define PLATFORM_ANDROID 1

// macOS Platform
#elif defined(__APPLE__) && defined(__MACH__)

    #include <TargetConditionals.h>
    #include <mach/mach_time.h>
    
    #if TARGET_OS_OSX
        #define PLATFORM_MACOS 1
    #elif TARGET_OS_IPHONE
        #define PLATFORM_IOS 1
    #endif
    
    // macOS logging macros (use standard printf for now)
    #define PLATFORM_LOG_DEBUG(tag, ...) \
        printf("[DEBUG][%s] ", tag); printf(__VA_ARGS__); printf("\n")
    
    #define PLATFORM_LOG_INFO(tag, ...) \
        printf("[INFO][%s] ", tag); printf(__VA_ARGS__); printf("\n")
    
    #define PLATFORM_LOG_ERROR(tag, ...) \
        fprintf(stderr, "[ERROR][%s] ", tag); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")

// Linux Platform (including ARM64)
#elif defined(__linux__)

    #if defined(__aarch64__) || defined(__arm64__)
        #define PLATFORM_LINUX_ARM64 1
    #endif
    
    // Linux logging macros (use standard printf)
    #define PLATFORM_LOG_DEBUG(tag, ...) \
        printf("[DEBUG][%s] ", tag); printf(__VA_ARGS__); printf("\n")
    
    #define PLATFORM_LOG_INFO(tag, ...) \
        printf("[INFO][%s] ", tag); printf(__VA_ARGS__); printf("\n")
    
    #define PLATFORM_LOG_ERROR(tag, ...) \
        fprintf(stderr, "[ERROR][%s] ", tag); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")

// Windows Platform
#elif defined(_WIN32) || defined(_WIN64)

    #define PLATFORM_WINDOWS 1
    
    // Windows logging macros (use standard printf)
    #define PLATFORM_LOG_DEBUG(tag, ...) \
        printf("[DEBUG][%s] ", tag); printf(__VA_ARGS__); printf("\n")
    
    #define PLATFORM_LOG_INFO(tag, ...) \
        printf("[INFO][%s] ", tag); printf(__VA_ARGS__); printf("\n")
    
    #define PLATFORM_LOG_ERROR(tag, ...) \
        fprintf(stderr, "[ERROR][%s] ", tag); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")

#else
    #error "Unsupported platform"
#endif

///////////////////////////////////////////////////////////////////////////////
// Architecture Detection
///////////////////////////////////////////////////////////////////////////////

// ARM64/AArch64 Architecture
#if defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64)
    #define PLATFORM_ARCH_ARM64 1
    #define PLATFORM_ARCH_NAME "ARM64"

// ARMv7 Architecture
#elif defined(__arm__) || defined(__ARM_ARCH_7A__) || defined(_M_ARM)
    #define PLATFORM_ARCH_ARMV7 1
    #define PLATFORM_ARCH_NAME "ARMv7"

// x86_64 Architecture
#elif defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
    #define PLATFORM_ARCH_X86_64 1
    #define PLATFORM_ARCH_NAME "x86_64"

// x86 Architecture
#elif defined(__i386__) || defined(_M_IX86)
    #define PLATFORM_ARCH_X86 1
    #define PLATFORM_ARCH_NAME "x86"

#else
    #define PLATFORM_ARCH_NAME "Unknown"
#endif

///////////////////////////////////////////////////////////////////////////////
// Endianness Detection
///////////////////////////////////////////////////////////////////////////////

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define PLATFORM_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define PLATFORM_BIG_ENDIAN 1
#else
    // Default to little endian for common platforms
    #if defined(PLATFORM_ARCH_X86) || defined(PLATFORM_ARCH_X86_64) || \
        defined(PLATFORM_ARCH_ARM64) || defined(PLATFORM_ARCH_ARMV7)
        #define PLATFORM_LITTLE_ENDIAN 1
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_PLATFORM_H
