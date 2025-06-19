#include "chrono_window.h"

static BOOL  gb_is_dragging = FALSE;
static POINT g_drag_offset  = { 0, 0 };
static VOID  get_time_str (WCHAR * buffer, SIZE_T buffer_size);

LRESULT CALLBACK window_proc (HWND h_wnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
    switch (u_msg)
    {
        case WM_CREATE:
            SetTimer(h_wnd, TIMER_ID, UPDATE_INTERVAL, NULL);
            return 0;

        case WM_TIMER:
            if (TIMER_ID == w_param)
            {
                // force repaint
                InvalidateRect(h_wnd, NULL, TRUE);
            }

            return 0;

        case WM_DESTROY:
            KillTimer(h_wnd, TIMER_ID);
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:;
            PAINTSTRUCT paint         = { 0 };
            HDC         h_display_ctx = BeginPaint(h_wnd, &paint);

            RECT rect = { 0 };
            GetClientRect(h_wnd, &rect);

            HBRUSH h_brush = CreateSolidBrush(WINDOW_COLOR);
            FillRect(h_display_ctx, &rect, h_brush);
            DeleteObject(h_brush);

            HFONT h_font = CreateFontW(FONT_SIZE,
                                       0,
                                       0,
                                       0,
                                       FW_BOLD,
                                       FALSE,
                                       FALSE,
                                       FALSE,
                                       DEFAULT_CHARSET,
                                       OUT_DEFAULT_PRECIS,
                                       CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY,
                                       DEFAULT_PITCH | FF_SWISS,
                                       FONT_NAME);

            HFONT h_old_font = (HFONT)SelectObject(h_display_ctx, h_font);

            WCHAR time_str[16] = { 0 };
            get_time_str(time_str, sizeof(time_str) / sizeof(WCHAR));

            SetTextColor(h_display_ctx, FONT_COLOR);
            SetBkMode(h_display_ctx, TRANSPARENT);

            DrawTextW(h_display_ctx, time_str, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(h_display_ctx, h_old_font);
            DeleteObject(h_font);

            EndPaint(h_wnd, &paint);
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
        case WM_KEYDOWN:
            if (VK_ESCAPE == w_param)
            {
                DestroyWindow(h_wnd);
            }

            return 0;
    }

    return DefWindowProcW(h_wnd, u_msg, w_param, l_param);
}

VOID fallback_rounded_corners (HWND h_wnd, INT corner_radius)
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

VOID snap_to_corner (HWND h_wnd)
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
        DLOG("Window snapped to position: %d, %d on monitor\n", new_x, new_y);
    }

EXIT:
    return;
}

BOOL enable_acrylic (HWND h_wnd)
{
    BOOL b_status      = FALSE;
    BOOL b_dwm_enabled = FALSE;

    if (FAILED(DwmIsCompositionEnabled(&b_dwm_enabled)) || (!b_dwm_enabled))
    {
        DLOG("DWM is not enabled, \n");
        log_last_error();
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

BOOL enable_dwm_rounded_corners (HWND h_wnd)
{
    // should work on Windows 11 build 22000+
    DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUND;
    HRESULT h_result = DwmSetWindowAttribute(h_wnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));

    return SUCCEEDED(h_result);
}

static VOID get_time_str (WCHAR * buffer, SIZE_T buffer_size)
{
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);

    if (9 <= buffer_size)
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
    else
    {
        DLOG("Buffer size is too small to hold the time string\n");
        buffer[0] = L'\0';
    }
}