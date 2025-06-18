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
#define FONT_SIZE       36