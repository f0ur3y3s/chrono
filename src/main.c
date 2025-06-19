#include "main.h"

INT WINAPI WinMain (HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmdline, INT show_cmd)
{
    UNREFERENCED_PARAMETER(h_prev_instance);
    UNREFERENCED_PARAMETER(lp_cmdline);
    UNREFERENCED_PARAMETER(show_cmd);

    DLOG("Starting chrono...\n");

    INT         status = 1;
    WNDCLASSEXW wnd    = { 0 };
    wnd.cbSize         = sizeof(WNDCLASSEXW);
    wnd.lpfnWndProc    = window_proc;
    wnd.hInstance      = h_instance;
    wnd.lpszClassName  = CLASS_NAME;
    wnd.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wnd.hbrBackground  = NULL;

    DLOG("Registering window class...\n");

    if (0 == RegisterClassExW(&wnd))
    {
        DLOG("Failed to register window class\n");
        log_last_error();
        goto EXIT;
    }

    HWND h_wnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                                 CLASS_NAME,
                                 WINDOW_NAME,
                                 WS_POPUP | WS_VISIBLE,
                                 INIT_X,
                                 INIT_Y,
                                 WINDOW_WIDTH,
                                 WINDOW_HEIGHT,
                                 NULL,
                                 NULL,
                                 h_instance,
                                 NULL);

    if (NULL == h_wnd)
    {
        DLOG("Failed to create window\n");
        log_last_error();
        goto UNREGISTER_EXIT;
    }

    if (!enable_dwm_rounded_corners(h_wnd))
    {
        // TODO: this isnt working on windows 10 for some reason
        fallback_rounded_corners(h_wnd, RADIUS);
    }

    if (!enable_acrylic(h_wnd))
    {
        MessageBoxW(NULL,
                    L"Failed to enable acrylic effect. This feature requires Windows 10 build 1803 or later.",
                    L"Warning",
                    MB_OK | MB_ICONWARNING);
        DLOG("Failed to enable acrylic effect\n");
        log_last_error();
        goto DESTROY_EXIT;
    }

    if (!SetLayeredWindowAttributes(h_wnd, 0, TRANSPARENCY, LWA_ALPHA))
    {
        DLOG("Failed to set layered window attributes\n");
        log_last_error();
        goto UNREGISTER_EXIT;
    }

    MSG msg = { 0 };

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    status = (INT)msg.wParam;

UNREGISTER_EXIT:
    if (0 == UnregisterClassW(CLASS_NAME, h_instance))
    {
        DLOG("Failed to unregister window class\n");
        log_last_error();
    }

DESTROY_EXIT:
    if (0 == DestroyWindow(h_wnd))
    {
        DLOG("Failed to destroy window\n");
        log_last_error();
    }

EXIT:
    DLOG("Exiting chrono...\n");

    return status;
}
