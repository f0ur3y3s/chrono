#include "debug.h"

#ifdef _DEBUG

VOID log_debug (LPCWSTR p_format, ...)
{
    va_list args;
    va_start(args, p_format);
    WCHAR buffer[1024] = { 0 };
    wvsprintfW(buffer, p_format, args);
    va_end(args);

    OutputDebugStringW(buffer);
}

DWORD log_last_error (VOID)
{
    wchar_t * p_wmessage = NULL;
    DWORD     error_code = GetLastError();

    DWORD format_retval
        = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL,
                         error_code,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         (LPWSTR)&p_wmessage,
                         0,
                         NULL);

    if (0 == format_retval)
    {
        log_debug(L"FormatMessage failed\n");
        goto EXIT;
    }

    log_debug(L"[-] %ls", p_wmessage);
    LocalFree(p_wmessage);

EXIT:
    return (format_retval);
}

#endif // _DEBUG