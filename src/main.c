
#include "main.h"

static LRESULT CALLBACK window_proc (HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param);
static VOID             snap_to_corner (HWND h_wnd);
static BOOL             enable_acrylic (HWND h_wnd);
static BOOL             enable_dwm_rounded_corners (HWND h_wnd);
static VOID             apply_rounded_corners (HWND h_wnd, INT radius);

// No-CRT helper functions
static VOID get_current_time_string (WCHAR * buffer, SIZE_T buffer_size);
static VOID strcpy_nocrt (CHAR * dest, const CHAR * src, SIZE_T dest_size);

static BOOL  gb_is_dragging = FALSE;
static POINT g_drag_offset  = { 0, 0 };

INT WINAPI WinMain (HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmdline, INT show_cmd)
{
    DLOG("Starting chronofloat...\n");
    UNREFERENCED_PARAMETER(h_prev_instance);
    UNREFERENCED_PARAMETER(lp_cmdline);

    INT         status = 1;
    WNDCLASSEXW wnd    = { 0 };
    wnd.cbSize         = sizeof(WNDCLASSEXW);
    wnd.lpfnWndProc    = window_proc;
    wnd.hInstance      = h_instance;
    wnd.lpszClassName  = CLASS_NAME;
    wnd.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wnd.hCursor        = LoadCursor(NULL, IDC_ARROW);

    if (0 == RegisterClassExW(&wnd))
    {
        DLOG("Failed to register window class: %d\n", GetLastError());
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
        DLOG("Failed to create window, %d\n", GetLastError());
        goto EXIT;
    }

    if (!enable_dwm_rounded_corners(h_wnd))
    {
        apply_rounded_corners(h_wnd, RADIUS);
    }

    if (!enable_acrylic(h_wnd))
    {
        MessageBoxW(NULL,
                    L"Failed to enable acrylic effect. This feature requires Windows 10 build 1803 or later.",
                    L"Warning",
                    MB_OK | MB_ICONWARNING);
    }

    if (!SetLayeredWindowAttributes(h_wnd, 0, TRANSPARENCY, LWA_ALPHA))
    {
        DLOG("Failed to set layered window attributes, %d\n", GetLastError());
        goto EXIT;
    }

    if (0 == SetTimer(h_wnd, TIMER_ID, UPDATE_INTERVAL, NULL))
    {
        DLOG("Failed to set timer, %d\n", GetLastError());
        goto EXIT;
    }

    ShowWindow(h_wnd, show_cmd);

    if (!UpdateWindow(h_wnd))
    {
        DLOG("Failed to update window, %d\n", GetLastError());
        goto EXIT;
    }

    MSG msg = { 0 };

    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    status = 0;

EXIT:
    return status;
}

static LRESULT CALLBACK window_proc (HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    if (NULL == h_wnd)
    {
        DLOG("Invalid window handle, %d\n", GetLastError());
        PostQuitMessage(1);
    }

    switch (msg)
    {
        case WM_DESTROY:
            if (!KillTimer(h_wnd, TIMER_ID))
            {
                DLOG("Failed to kill timer, %d\n", GetLastError());
            }

            PostQuitMessage(0);
            return 0;

        case WM_PAINT:;
            PAINTSTRUCT paint         = { 0 };
            HDC         h_display_ctx = BeginPaint(h_wnd, &paint);

            if (NULL == h_display_ctx)
            {
                DLOG("Failed to get display context, %d\n", GetLastError());
                PostQuitMessage(1);
                return -1;
            }

            RECT rect = { 0 };

            if (!GetClientRect(h_wnd, &rect))
            {
                DLOG("Failed to get client rect, %d\n", GetLastError());
                EndPaint(h_wnd, &paint);
                return -1;
            }

            HBRUSH h_brush = CreateSolidBrush(RGB(0, 0, 0));

            if (NULL == h_brush)
            {
                DLOG("Failed to create brush, %d\n", GetLastError());
                EndPaint(h_wnd, &paint);
                return -1;
            }

            if (0 == FillRect(h_display_ctx, &rect, h_brush))
            {
                DLOG("Failed to fill rect, %d\n", GetLastError());
                DeleteObject(h_brush);
                EndPaint(h_wnd, &paint);
                return -1;
            }

            if (0 == SetBkMode(h_display_ctx, TRANSPARENT))
            {
                DLOG("Failed to set background mode, %d\n", GetLastError());
                EndPaint(h_wnd, &paint);
                return -1;
            }

            if (!DeleteObject(h_brush))
            {
                DLOG("Failed to delete brush, %d\n", GetLastError());
                EndPaint(h_wnd, &paint);
                return -1;
            }

            // Use Windows API to get current time instead of CRT functions
            WCHAR time_str[80] = { 0 };
            get_current_time_string(time_str, sizeof(time_str) / sizeof(WCHAR));

            if (CLR_INVALID == SetTextColor(h_display_ctx, RGB(255, 255, 255)))
            {
                DLOG("Failed to set text color, %d\n", GetLastError());
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

            DrawTextW(h_display_ctx, time_str, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(h_display_ctx, h_old_font);
            DeleteObject(h_font);

            EndPaint(h_wnd, &paint);
            return 0;

        case WM_TIMER:
            if (w_param == TIMER_ID)
            {
                if (!InvalidateRect(h_wnd, NULL, TRUE))
                {
                    DLOG("Failed to invalidate rect, %d\n", GetLastError());
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

                INT new_x = cursor_pos.x - g_drag_offset.x;
                INT new_y = cursor_pos.y - g_drag_offset.y;

                SetWindowPos(h_wnd, NULL, new_x, new_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }

            return 0;
    }

    return DefWindowProcW(h_wnd, msg, w_param, l_param);
}

// No-CRT helper function to get current time as formatted string
static VOID get_current_time_string (WCHAR * buffer, SIZE_T buffer_size)
{
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);

    // Format as HH:MM:SS
    if (buffer_size >= 9) // Minimum size for "HH:MM:SS\0"
    {
        buffer[0] = L'0' + (st.wHour / 10);
        buffer[1] = L'0' + (st.wHour % 10);
        buffer[2] = L':';
        buffer[3] = L'0' + (st.wMinute / 10);
        buffer[4] = L'0' + (st.wMinute % 10);
        buffer[5] = L':';
        buffer[6] = L'0' + (st.wSecond / 10);
        buffer[7] = L'0' + (st.wSecond % 10);
        buffer[8] = L'\0';
    }
    else if (buffer_size > 0)
    {
        buffer[0] = L'\0'; // Ensure null termination
    }
}

// No-CRT string copy function (if needed elsewhere)
static VOID strcpy_nocrt (CHAR * dest, const CHAR * src, SIZE_T dest_size)
{
    SIZE_T i = 0;
    if (dest_size > 0)
    {
        while (src[i] != '\0' && i < (dest_size - 1))
        {
            dest[i] = src[i];
            i++;
        }
        dest[i] = '\0';
    }
}

static VOID apply_rounded_corners (HWND h_wnd, INT corner_radius)
{
    RECT rect = { 0 };
    GetClientRect(h_wnd, &rect);

    INT width  = rect.right - rect.left;
    INT height = rect.bottom - rect.top;

    HRGN h_rgn = CreateRoundRectRgn(0, 0, width + 1, height + 1, corner_radius * 2, corner_radius * 2);

    if (h_rgn)
    {
        SetWindowRgn(h_wnd, h_rgn, TRUE);
        // windows takes ownership of the region, don't delete it
    }
}

static VOID snap_to_corner (HWND h_wnd)
{
    RECT window_rect = { 0 };

    if (!GetWindowRect(h_wnd, &window_rect))
    {
        DLOG("Failed to get window rectangle for snapping\n");
        goto EXIT;
    }

    HMONITOR h_monitor = MonitorFromWindow(h_wnd, MONITOR_DEFAULTTONEAREST);

    if (NULL == h_monitor)
    {
        DLOG("Failed to get monitor handle for snapping\n");
        goto EXIT;
    }

    MONITORINFO monitor_info = { 0 };
    monitor_info.cbSize      = sizeof(MONITORINFO);

    if (!GetMonitorInfoW(h_monitor, &monitor_info))
    {
        DLOG("Failed to get monitor info for snapping\n");
        goto EXIT;
    }

    RECT monitor_rect   = monitor_info.rcMonitor;
    INT  window_width   = window_rect.right - window_rect.left;
    INT  window_height  = window_rect.bottom - window_rect.top;
    INT  monitor_left   = monitor_rect.left;
    INT  monitor_top    = monitor_rect.top;
    INT  monitor_right  = monitor_rect.right;
    INT  monitor_bottom = monitor_rect.bottom;
    INT  new_x          = window_rect.left;
    INT  new_y          = window_rect.top;
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

        { (window_rect.left <= monitor_left + SNAP_DISTANCE && window_rect.bottom >= monitor_bottom -
        SNAP_DISTANCE),
          monitor_left,
          monitor_bottom - window_height,
          "bottom-left" },

        { (window_rect.right >= monitor_right - SNAP_DISTANCE && window_rect.bottom >= monitor_bottom -
        SNAP_DISTANCE),
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
    for (INT idx = 0; idx < (sizeof(snap_targets) / sizeof(snap_targets[0])); idx++)
    {
        if (snap_targets[idx].b_cond)
        {
            new_x     = snap_targets[idx].x;
            new_y     = snap_targets[idx].y;
            b_snapped = TRUE;
            DLOG("Snapped to %S\n", snap_targets[idx].p_name);
            break;
        }
    }

    if (b_snapped)
    {
        SetWindowPos(h_wnd, NULL, new_x, new_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        DLOG("Window b_snapped to position: %d, %d on monitor\n", new_x, new_y);
    }

EXIT:
    return;
}

static BOOL enable_acrylic (HWND h_wnd)
{
    BOOL b_status      = FALSE;
    BOOL b_dwm_enabled = FALSE;

    if (FAILED(DwmIsCompositionEnabled(&b_dwm_enabled)) || (!b_dwm_enabled))
    {
        DLOG("DWM is not enabled, %d\n", GetLastError());
        goto EXIT;
    }

    // load SetWindowCompositionAttribute
    HMODULE h_user32 = GetModuleHandleW(L"user32.dll");

    if (h_user32)
    {
        pSetWindowCompositionAttribute SetWindowCompositionAttribute
            = (pSetWindowCompositionAttribute)GetProcAddress(h_user32, "SetWindowCompositionAttribute");

        if (SetWindowCompositionAttribute)
        {
            ACCENT_POLICY accent = { 0 };
            accent.AccentState   = ACCENT_ENABLE_ACRYLICBLURBEHIND;
            accent.AccentFlags   = 0;
            accent.GradientColor = 0xA0000000;
            accent.AnimationId   = 0;

            WINDOWCOMPOSITIONATTRIBDATA data = { 0 };
            data.Attribute                   = WCA_ACCENT_POLICY;
            data.pvData                      = &accent;
            data.cbData                      = sizeof(accent);

            b_status = SetWindowCompositionAttribute(h_wnd, &data);
        }
    }

EXIT:
    return b_status;
}

static BOOL enable_dwm_rounded_corners (HWND h_wnd)
{
    // should work on Windows 11 build 22000+
    DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUND;
    HRESULT h_result = DwmSetWindowAttribute(h_wnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));

    return SUCCEEDED(h_result);
}

static VOID debug_log (const WCHAR * p_format, ...)
{
    WCHAR   buffer[1024] = { 0 };
    va_list args;
    va_start(args, p_format);
    vswprintf(buffer, sizeof(buffer) / sizeof(WCHAR), p_format, args);
    va_end(args);
    OutputDebugStringW(buffer);
}