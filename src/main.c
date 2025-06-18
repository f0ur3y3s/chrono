#include "main.h"

LRESULT CALLBACK WindowProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void             SetRoundedCorners (HWND hwnd, int radius);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    WNDCLASS wc      = { 0 };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowExA(                          // Explicitly use ANSI version
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, // Extended window styles
        CLASS_NAME,
        "ChronoFloat", // Simpler ASCII title
        WS_POPUP,      // No title bar or borders
        100,
        100, // Position
        WINDOW_WIDTH,
        WINDOW_HEIGHT, // Size
        NULL,
        NULL,
        hInstance,
        NULL);

    if (hwnd == NULL)
    {
        return 0;
    }

    // Set window transparency (0-255, where 0 is fully transparent, 255 is opaque)
    SetLayeredWindowAttributes(hwnd, 0, 200, LWA_ALPHA);

    // Set rounded corners
    SetRoundedCorners(hwnd, 20); // 20 pixel radius

    // Set up a timer to update the time every second
    SetTimer(hwnd, TIMER_ID, 1000, NULL);

    ShowWindow(hwnd, nShowCmd);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC         hdc = BeginPaint(hwnd, &ps);

            // Set background to black for better contrast
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);

            // Get current time
            time_t      rawtime;
            struct tm * timeinfo;
            char        timeStr[80];

            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);

            // Set text properties
            SetTextColor(hdc, RGB(255, 255, 255)); // White text
            SetBkMode(hdc, TRANSPARENT);

            // Create a larger font
            HFONT hFont = CreateFont(36,                       // Height
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
                                     "Arial"                   // Font name
            );

            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

            // Draw the time text centered
            DrawText(hdc, timeStr, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Cleanup
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_TIMER:
            if (wParam == TIMER_ID)
            {
                InvalidateRect(hwnd, NULL, TRUE); // Force repaint
            }
            return 0;

        case WM_LBUTTONDOWN:
            // Allow dragging the window by clicking and dragging
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            return 0;

        case WM_RBUTTONDOWN:
            // Right-click to close
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void SetRoundedCorners (HWND hwnd, int radius)
{
    RECT rect;
    GetWindowRect(hwnd, &rect);
    int width  = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create a rounded rectangle region
    HRGN hRgn = CreateRoundRectRgn(0, 0, width, height, radius * 2, radius * 2);

    // Apply the region to the window
    SetWindowRgn(hwnd, hRgn, TRUE);

    // Note: Don't delete the region - Windows takes ownership of it
}