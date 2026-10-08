#pragma once
#define URANIUM_VERSION_MAJOR 1
#define URANIUM_VERSION_MINOR 0
#define URANIUM_VERSION_PATCH 0
#define URANIUM_VERSION_NUMBER ((URANIUM_VERSION_MAJOR * 10000) + (URANIUM_VERSION_MINOR * 100) + URANIUM_VERSION_PATCH)
#define URANIUM_VERSION_STRING "1.0.0"
#if defined(_WIN32) || defined(__CYGWIN__)
#    if defined(URANIUM_SHARED)
#        if defined(URANIUM_BUILDING_LIBRARY)
#            define URANIUM_API __declspec(dllexport)
#        else
#            define URANIUM_API __declspec(dllimport)
#        endif
#    else
#        define URANIUM_API
#    endif
#elif defined(__GNUC__) && defined(URANIUM_SHARED)
#    if defined(URANIUM_BUILDING_LIBRARY)
#        define URANIUM_API __attribute__((visibility("default")))
#    else
#        define URANIUM_API
#    endif
#else
#    define URANIUM_API
#endif
#define URANIUM_NODISCARD [[nodiscard]]
