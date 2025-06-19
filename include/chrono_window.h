#pragma once

#include <Windows.h>
#include <dwmapi.h>
#include "debug.h"

#pragma comment(lib, "dwmapi.lib")

#define SNAP_DISTANCE   100  // Distance in pixels to snap to the edges of the screen
#define B_SHOULD_SNAP   TRUE // Set to TRUE to enable snapping to screen edges and corners
#define UPDATE_INTERVAL 1000 // Update interval in milliseconds
#define FONT_NAME       L"Consolas"
#define FONT_SIZE       50
#define FONT_COLOR      RGB(255, 255, 255)
#define WINDOW_COLOR    RGB(1, 1, 1)

#define TIMER_ID 31337

LRESULT CALLBACK window_proc (HWND h_wnd, UINT u_msg, WPARAM w_param, LPARAM l_param);
VOID             snap_to_corner (HWND h_wnd);
BOOL             enable_acrylic (HWND h_wnd);
BOOL             enable_dwm_rounded_corners (HWND h_wnd);
VOID             fallback_rounded_corners (HWND h_wnd, INT radius);
