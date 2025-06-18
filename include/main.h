#pragma once

#include <Windows.h>
#include <time.h>
#include <stdio.h>

#define TIMER_ID        31337
#define WINDOW_WIDTH    300
#define WINDOW_HEIGHT   100
#define CLASS_NAME      L"ChronoWindow"
#define WINDOW_NAME     L"ChronoFloat"
#define TRANSPARENCY    200  // 0-255, where 0 is fully transparent and 255 is fully opaque
#define UPDATE_INTERVAL 1000 // Update interval in milliseconds
#define FONT_NAME       L"Consolas"
#define FONT_SIZE       50
#define RADIUS          10
#define SNAP_DISTANCE   100 // Distance in pixels to snap to the edges of the screen
#define B_SHOULD_SNAP   TRUE

typedef struct snap_target_t
{
    BOOL         b_cond;
    int          x;
    int          y;
    const char * p_name;
} snap_target_t;