#pragma once

#ifdef DEBUG
    #define _STRINGIFY(str) #str
    #define STRINGIFY(str) _STRINGIFY(str)
    #define DEBUG_FORMAT(str) (__FILE__ ":" STRINGIFY(__LINE__) ": " str "\n")

    #ifdef __cplusplus
        #include <cstdio>
    #else
        #include <stdio.h>
    #endif

    #define debug(format, ...) fprintf(stderr, DEBUG_FORMAT(format), ##__VA_ARGS__)
#else
    #define debug(format, ...) ((void) 0)
#endif


#ifndef NDEBUG
    #define _STRINGIFY(str) #str
    #define STRINGIFY(str) _STRINGIFY(str)
    #define ERROR_FORMAT(str) (__FILE__ ":" STRINGIFY(__LINE__) ": " str "\n")

    #ifdef __cplusplus
        #include <cstdio>
    #else
        #include <stdio.h>
    #endif

    #define ERROR(format, ...) fprintf(stderr, ERROR_FORMAT(format), ##__VA_ARGS__)
#else
    #define ERROR(format, ...) ((void) 0)
#endif
