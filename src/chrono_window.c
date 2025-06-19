#include "chrono_window.h"

static BOOL  gb_is_dragging = FALSE;
static POINT g_drag_offset  = { 0, 0 };
static VOID  get_time_str (WCHAR * buffer, SIZE_T buffer_size);

// windows 10 build 1803+ acrylic effect
typedef enum _ACCENT_STATE
{
    ACCENT_DISABLED                   = 0,
    ACCENT_ENABLE_GRADIENT            = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND          = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND   = 4,
    ACCENT_INVALID_STATE              = 5
} ACCENT_STATE;

typedef struct _ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD        AccentFlags;
    DWORD        GradientColor;
    DWORD        AnimationId;
} ACCENT_POLICY;

typedef enum _WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED                     = 0,
    WCA_NCRENDERING_ENABLED           = 1,
    WCA_NCRENDERING_POLICY            = 2,
    WCA_TRANSITIONS_FORCEDISABLED     = 3,
    WCA_ALLOW_NCPAINT                 = 4,
    WCA_CAPTION_BUTTON_BOUNDS         = 5,
    WCA_NONCLIENT_RTL_LAYOUT          = 6,
    WCA_FORCE_ICONIC_REPRESENTATION   = 7,
    WCA_EXTENDED_FRAME_BOUNDS         = 8,
    WCA_HAS_ICONIC_BITMAP             = 9,
    WCA_THEME_ATTRIBUTES              = 10,
    WCA_NCRENDERING_EXILED            = 11,
    WCA_NCADORNMENTINFO               = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW     = 13,
    WCA_VIDEO_OVERLAY_ACTIVE          = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK                 = 16,
    WCA_CLOAK                         = 17,
    WCA_CLOAKED                       = 18,
    WCA_ACCENT_POLICY                 = 19,
    WCA_FREEZE_REPRESENTATION         = 20,
    WCA_EVER_UNCLOAKED                = 21,
    WCA_VISUAL_OWNER                  = 22,
    WCA_HOLOGRAPHIC                   = 23,
    WCA_EXCLUDED_FROM_DDA             = 24,
    WCA_PASSIVEUPDATEMODE             = 25,
    WCA_USEDARKMODECOLORS             = 26,
    WCA_LAST                          = 27
} WINDOWCOMPOSITIONATTRIB;

typedef struct _WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attribute;
    PVOID                   pvData;
    SIZE_T                  cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef BOOL(WINAPI * pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);

typedef struct snap_target_t
{
    BOOL         b_cond;
    int          x;
    int          y;
    const char * p_name;
} snap_target_t;

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
        DLOG("DWM is not enabled\n");
        log_last_error();
        goto EXIT;
    }

    HMODULE h_user32 = GetModuleHandleA("user32");

    if (!h_user32)
    {
        DLOG("user32 module not found\n");
        goto EXIT;
    }

    struct
    {
        HMODULE      module;
        FARPROC      function;
        const char * name;
    } api_info = { .module = h_user32, .function = NULL, .name = "SetWindowCompositionAttribute" };

    api_info.function = GetProcAddress(api_info.module, api_info.name);

    if (!api_info.function)
    {
        DLOG("Composition attribute function not available\n");
        goto EXIT;
    }

    pSetWindowCompositionAttribute composition_func = (pSetWindowCompositionAttribute)api_info.function;

    ACCENT_POLICY accent = { 0 };
    accent.AccentState   = ACCENT_ENABLE_ACRYLICBLURBEHIND;
    accent.AccentFlags   = 0;
    accent.GradientColor = 0xA0000000;
    accent.AnimationId   = 0;

    WINDOWCOMPOSITIONATTRIBDATA data = { 0 };
    data.Attribute                   = WCA_ACCENT_POLICY;
    data.pvData                      = &accent;
    data.cbData                      = sizeof(accent);

    b_status = composition_func(h_wnd, &data);
    DLOG("Acrylic effect %s\n", b_status ? "enabled" : "failed");

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