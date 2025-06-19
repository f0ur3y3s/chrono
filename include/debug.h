#pragma once

// clang-format off

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef _DEBUG
    #include <strsafe.h>
    #pragma comment(lib, "user32.lib")

    VOID log_debug(LPCWSTR p_format, ...);
    DWORD log_last_error(VOID);
    #define DLOG(fmt, ...) log_debug(L##fmt, ##__VA_ARGS__)
#else
    #define log_debug(format, ...) ((void)0)
    #define log_last_error() (0)
    #define DLOG(fmt, ...) ((void)0)
#endif

// clang-format on