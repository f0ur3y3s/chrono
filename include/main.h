#pragma once

#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <dwmapi.h>
#include <WinUser.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define TIMER_ID        31337
#define WINDOW_WIDTH    300
#define WINDOW_HEIGHT   100
#define CLASS_NAME      L"ChronoWindow"
#define WINDOW_NAME     L"ChronoFloat"
#define FONT_NAME       L"Consolas"
#define FONT_SIZE       50
#define RADIUS          10
#define TRANSPARENCY    250  // 0-255, where 0 is fully transparent and 255 is fully opaque
#define UPDATE_INTERVAL 1000 // Update interval in milliseconds
#define SNAP_DISTANCE   100  // Distance in pixels to snap to the edges of the screen
#define B_SHOULD_SNAP   TRUE // Set to TRUE to enable snapping to screen edges and corners

typedef struct snap_target_t
{
    BOOL         b_cond;
    int          x;
    int          y;
    const char * p_name;
} snap_target_t;

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
