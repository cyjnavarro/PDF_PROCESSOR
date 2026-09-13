#pragma once

#include <windows.h>

// GitHub Dark Color Theme
namespace Colors {
    // Primary Colors
    constexpr COLORREF BG_PRIMARY = RGB(13, 17, 23);      // #0d1117 - Main background
    constexpr COLORREF BG_SECONDARY = RGB(22, 27, 34);    // #161b22 - Secondary surface
    constexpr COLORREF BG_TERTIARY = RGB(33, 38, 45);     // #21262d - Tertiary surface
    
    // Text Colors
    constexpr COLORREF TEXT_PRIMARY = RGB(201, 209, 217);   // #c9d1d9 - Main text
    constexpr COLORREF TEXT_SECONDARY = RGB(139, 148, 158); // #8b949e - Secondary text
    constexpr COLORREF TEXT_MUTED = RGB(110, 118, 129);     // #6e7681 - Muted text
    
    // Accent Colors
    constexpr COLORREF ACCENT_BLUE = RGB(88, 166, 255);     // #58a6ff - Primary action
    constexpr COLORREF ACCENT_BLUE_HOVER = RGB(121, 192, 255); // #79c0ff - Hover
    constexpr COLORREF BORDER = RGB(48, 54, 61);           // #30363d - Border
    
    // Status Colors
    constexpr COLORREF SUCCESS_GREEN = RGB(63, 185, 80);   // #3fb950 - Success
    constexpr COLORREF ERROR_RED = RGB(248, 81, 73);       // #f85149 - Error
    constexpr COLORREF WARNING_ORANGE = RGB(210, 153, 34); // #d29922 - Warning
    constexpr COLORREF DANGER_RED = RGB(218, 54, 51);      // #da3633 - Danger
}

// UI Dimensions
namespace UI {
    constexpr int PADDING = 10;
    constexpr int BORDER_RADIUS = 8;
    constexpr int BUTTON_HEIGHT = 36;
    constexpr int BADGE_HEIGHT = 45;
    constexpr int DROP_ZONE_HEIGHT = 120;
}
