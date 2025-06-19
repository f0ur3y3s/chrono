#pragma once

#include <Windows.h>
#include <WinUser.h>
#include "debug.h"
#include "chrono_window.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define WINDOW_WIDTH  300
#define WINDOW_HEIGHT 100
#define INIT_X        100
#define INIT_Y        100
#define CLASS_NAME    L"ChronoWindow"
#define WINDOW_NAME   L"ChronoFloat"
#define RADIUS        10
#define TRANSPARENCY  245 // 0-255, where 0 is fully transparent and 255 is fully opaque
