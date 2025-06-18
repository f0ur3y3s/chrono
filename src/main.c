#include "main.h"

static LRESULT CALLBACK window_proc (HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param);
static BOOL             set_rounded_corners (HWND h_wnd, int radius);
static void             debug_log (const WCHAR * p_format, ...);
static void             snap_to_corner (HWND h_wnd);

static BOOL  gb_is_dragging = FALSE;
static POINT g_drag_offset  = { 0, 0 };

int WINAPI WinMain (HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmdline, int show_cmd)
{
    UNREFERENCED_PARAMETER(h_prev_instance);
    UNREFERENCED_PARAMETER(lp_cmdline);

    int         status = 0;
    WNDCLASSEXW wnd    = { 0 };
    wnd.cbSize         = sizeof(WNDCLASSEXW);
    wnd.lpfnWndProc    = window_proc;
    wnd.hInstance      = h_instance;
    wnd.lpszClassName  = CLASS_NAME;
    wnd.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wnd.hCursor        = LoadCursor(NULL, IDC_ARROW);

    if (0 == RegisterClassExW(&wnd))
    {
        debug_log(L"Failed to register window class: %d\n", GetLastError());
        status = 1;
        goto EXIT;
    }

    HWND h_wnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                                 CLASS_NAME,
                                 WINDOW_NAME,
                                 WS_POPUP,
                                 100,
                                 100,
                                 WINDOW_WIDTH,
                                 WINDOW_HEIGHT,
                                 NULL,
                                 NULL,
                                 h_instance,
                                 NULL);

    if (NULL == h_wnd)
    {
        debug_log(L"Failed to create window, %d\n", GetLastError());
        status = 2;
        goto EXIT;
    }

    if (!SetLayeredWindowAttributes(h_wnd, 0, TRANSPARENCY, LWA_ALPHA))
    {
        debug_log(L"Failed to set layered window attributes, %d\n", GetLastError());
        status = 3;
        goto EXIT;
    }

    if (!set_rounded_corners(h_wnd, RADIUS))
    {
        debug_log(L"Failed to set rounded corners, %d\n", GetLastError());
        status = 4;
        goto EXIT;
    }

    if (0 == SetTimer(h_wnd, TIMER_ID, UPDATE_INTERVAL, NULL))
    {
        debug_log(L"Failed to set timer, %d\n", GetLastError());
        status = 5;
        goto EXIT;
    }

    ShowWindow(h_wnd, show_cmd);

    // TODO: something funky here with winapi
    // if (!ShowWindow(h_wnd, show_cmd))
    // {
    //     debug_log(L"Failed to show window, %d\n", GetLastError());
    //     status = 6;
    //     goto EXIT;
    // }

    if (!UpdateWindow(h_wnd))
    {
        debug_log(L"Failed to update window, %d\n", GetLastError());
        status = 7;
        goto EXIT;
    }

    MSG msg = { 0 };

    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

EXIT:
    return status;
}

static LRESULT CALLBACK window_proc (HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    if (NULL == h_wnd)
    {
        debug_log(L"Invalid window handle, %d\n", GetLastError());
        PostQuitMessage(1);
    }

    switch (msg)
    {
        case WM_DESTROY:
            if (!KillTimer(h_wnd, TIMER_ID))
            {
                debug_log(L"Failed to kill timer, %d\n", GetLastError());
            }

            PostQuitMessage(0);
            return 0;

        case WM_PAINT:;
            PAINTSTRUCT paint         = { 0 };
            HDC         h_display_ctx = BeginPaint(h_wnd, &paint);

            if (NULL == h_display_ctx)
            {
                debug_log(L"Failed to get display context, %d\n", GetLastError());
                PostQuitMessage(1);
                return -1;
            }

            // set background to black for better contrast
            RECT rect = { 0 };

            if (!GetClientRect(h_wnd, &rect))
            {
                debug_log(L"Failed to get client rect, %d\n", GetLastError());
                EndPaint(h_wnd, &paint);
                return -1;
            }

            HBRUSH h_brush = CreateSolidBrush(RGB(0, 0, 0));

            if (NULL == h_brush)
            {
                debug_log(L"Failed to create brush, %d\n", GetLastError());
                EndPaint(h_wnd, &paint);
                return -1;
            }

            if (0 == FillRect(h_display_ctx, &rect, h_brush))
            {
                debug_log(L"Failed to fill rect, %d\n", GetLastError());
                DeleteObject(h_brush);
                EndPaint(h_wnd, &paint);
                return -1;
            }

            if (!DeleteObject(h_brush))
            {
                debug_log(L"Failed to delete brush, %d\n", GetLastError());
                EndPaint(h_wnd, &paint);
                return -1;
            }

            time_t    rawtime      = 0;
            struct tm timeinfo     = { 0 };
            char      time_str[80] = { 0 };

            time(&rawtime);

            errno_t err = localtime_s(&timeinfo, &rawtime);

            if (0 != err)
            {
                debug_log(L"localtime_s failed with error: %d\n", err);
                // default time string if localtime fails
                strcpy_s(time_str, sizeof(time_str), "00:00:00");
            }
            else
            {
                strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);
            }

            if (CLR_INVALID == SetTextColor(h_display_ctx, RGB(255, 255, 255)))
            {
                debug_log(L"Failed to set text color, %d\n", GetLastError());
                EndPaint(h_wnd, &paint);
                return -1;
            }

            if (0 == SetBkMode(h_display_ctx, TRANSPARENT))
            {
                debug_log(L"Failed to set background mode, %d\n", GetLastError());
                EndPaint(h_wnd, &paint);
                return -1;
            }

            HFONT h_font = CreateFontW(FONT_SIZE,                // Height
                                       0,                        // Width
                                       0,                        // Escapement
                                       0,                        // Orientation
                                       FW_BOLD,                  // Weight
                                       FALSE,                    // Italic
                                       FALSE,                    // Underline
                                       FALSE,                    // StrikeOut
                                       ANSI_CHARSET,             // CharSet
                                       OUT_DEFAULT_PRECIS,       // OutPrecision
                                       CLIP_DEFAULT_PRECIS,      // ClipPrecision
                                       DEFAULT_QUALITY,          // Quality
                                       DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
                                       FONT_NAME                 // Font name
            );

            HFONT h_old_font = (HFONT)SelectObject(h_display_ctx, h_font);

            DrawText(h_display_ctx, time_str, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(h_display_ctx, h_old_font);
            DeleteObject(h_font);

            EndPaint(h_wnd, &paint);
            return 0;

        case WM_TIMER:
            if (w_param == TIMER_ID)
            {
                if (!InvalidateRect(h_wnd, NULL, TRUE))
                {
                    debug_log(L"Failed to invalidate rect, %d\n", GetLastError());
                    PostQuitMessage(1);
                }
            }

            return 0;

        case WM_RBUTTONDOWN:
            PostMessageW(h_wnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_KEYDOWN:
            if (VK_ESCAPE == w_param)
            {
                PostMessageW(h_wnd, WM_CLOSE, 0, 0);
            }

            return 0;

        case WM_LBUTTONDOWN:
            if (B_SHOULD_SNAP)
            {
                gb_is_dragging   = TRUE;
                POINT cursor_pos = { 0 };
                GetCursorPos(&cursor_pos);

                RECT window_rect = { 0 };
                GetWindowRect(h_wnd, &window_rect);

                g_drag_offset.x = cursor_pos.x - window_rect.left;
                g_drag_offset.y = cursor_pos.y - window_rect.top;

                SetCapture(h_wnd);
                SendMessageW(h_wnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }
            else
            {
                SendMessageW(h_wnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }

            return 0;

        case WM_LBUTTONUP:
            if (gb_is_dragging)
            {
                gb_is_dragging = FALSE;
                ReleaseCapture();
                snap_to_corner(h_wnd);
            }

            return 0;

        case WM_MOUSEMOVE:
            if (gb_is_dragging && (w_param & MK_LBUTTON))
            {
                POINT cursor_pos = { 0 };
                GetCursorPos(&cursor_pos);

                int new_x = cursor_pos.x - g_drag_offset.x;
                int new_y = cursor_pos.y - g_drag_offset.y;

                SetWindowPos(h_wnd, NULL, new_x, new_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }

            return 0;
    }

    return DefWindowProcW(h_wnd, msg, w_param, l_param);
}

static BOOL set_rounded_corners (HWND h_wnd, int radius)
{
    BOOL b_status = FALSE;

    if (NULL == h_wnd || radius < 0)
    {
        goto EXIT;
    }

    RECT rect = { 0 };

    if (!GetWindowRect(h_wnd, &rect))
    {
        goto EXIT;
    }

    int width  = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HRGN h_region = CreateRoundRectRgn(0, 0, width, height, radius * 2, radius * 2);

    if (0 == SetWindowRgn(h_wnd, h_region, TRUE))
    {
        DeleteObject(h_region);
        goto EXIT;
    }

    // Note: Don't delete the region - Windows takes ownership of it

    b_status = TRUE;

EXIT:
    return b_status;
}

static void debug_log (const WCHAR * p_format, ...)
{
    WCHAR   buffer[1024] = { 0 };
    va_list args;
    va_start(args, p_format);
    vswprintf(buffer, sizeof(buffer) / sizeof(WCHAR), p_format, args);
    va_end(args);
    OutputDebugStringW(buffer);
}

static void snap_to_corner (HWND h_wnd)
{
    RECT window_rect = { 0 };

    if (!GetWindowRect(h_wnd, &window_rect))
    {
        debug_log(L"Failed to get window rectangle for snapping\n");
        goto EXIT;
    }

    HMONITOR h_monitor = MonitorFromWindow(h_wnd, MONITOR_DEFAULTTONEAREST);

    if (NULL == h_monitor)
    {
        debug_log(L"Failed to get monitor handle for snapping\n");
        goto EXIT;
    }

    MONITORINFO monitor_info = { 0 };
    monitor_info.cbSize      = sizeof(MONITORINFO);

    if (!GetMonitorInfoW(h_monitor, &monitor_info))
    {
        debug_log(L"Failed to get monitor info for snapping\n");
        goto EXIT;
    }

    RECT monitor_rect   = monitor_info.rcMonitor;
    int  window_width   = window_rect.right - window_rect.left;
    int  window_height  = window_rect.bottom - window_rect.top;
    int  monitor_left   = monitor_rect.left;
    int  monitor_top    = monitor_rect.top;
    int  monitor_right  = monitor_rect.right;
    int  monitor_bottom = monitor_rect.bottom;
    int  new_x          = window_rect.left;
    int  new_y          = window_rect.top;
    BOOL b_snapped      = FALSE;

    // clang-format off
    snap_target_t snap_targets[] = {
        { (window_rect.left <= monitor_left + SNAP_DISTANCE && window_rect.top <= monitor_top + SNAP_DISTANCE),
          monitor_left,
          monitor_top,
          "top-left" },

        { (window_rect.right >= monitor_right - SNAP_DISTANCE && window_rect.top <= monitor_top + SNAP_DISTANCE),
          monitor_right - window_width,
          monitor_top,
          "top-right" },

        { (window_rect.left <= monitor_left + SNAP_DISTANCE && window_rect.bottom >= monitor_bottom - SNAP_DISTANCE),
          monitor_left,
          monitor_bottom - window_height,
          "bottom-left" },

        { (window_rect.right >= monitor_right - SNAP_DISTANCE && window_rect.bottom >= monitor_bottom - SNAP_DISTANCE),
          monitor_right - window_width,
          monitor_bottom - window_height,
          "bottom-right" },

        { (window_rect.top <= monitor_top + SNAP_DISTANCE),
          new_x,
          monitor_top,
          "top-edge" },

        { (window_rect.bottom >= monitor_bottom - SNAP_DISTANCE),
          new_x,
          monitor_bottom - window_height,
          "bottom-edge" },

        { (window_rect.left <= monitor_left + SNAP_DISTANCE),
          monitor_left,
          new_y,
          "left-edge" },

        { (window_rect.right >= monitor_right - SNAP_DISTANCE),
          monitor_right - window_width,
          new_y,
          "right-edge" }
    };
    // clang-format on

    // check each snap target in priority order
    for (int idx = 0; idx < (sizeof(snap_targets) / sizeof(snap_targets[0])); idx++)
    {
        if (snap_targets[idx].b_cond)
        {
            new_x     = snap_targets[idx].x;
            new_y     = snap_targets[idx].y;
            b_snapped = TRUE;
            debug_log(L"Snapped to %S\n", snap_targets[idx].p_name);
            break;
        }
    }

    if (b_snapped)
    {
        SetWindowPos(h_wnd, NULL, new_x, new_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        debug_log(L"Window b_snapped to position: %d, %d on monitor\n", new_x, new_y);
    }

EXIT:
    return;
}