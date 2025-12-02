#ifndef THEME_H
#define THEME_H

#include <QString>

// ===== 現代科技風格主題顏色 =====
// 共用的主題配色常數，確保整個應用程式的視覺一致性

namespace Theme {
    // 背景顏色
    const QString BG_DARK = "#1A1A2E";           // 深色背景
    const QString BG_MEDIUM = "#16213E";         // 中等深度背景
    const QString BG_PANEL = "#0F3460";          // 面板背景
    
    // 強調色
    const QString ACCENT_PRIMARY = "#00D9FF";    // 主要強調色（霓虹青色）
    const QString ACCENT_SECONDARY = "#E94560";  // 次要強調色（霓虹粉色）
    const QString ACCENT_SUCCESS = "#00FF88";    // 成功色（霓虹綠色）
    const QString ACCENT_WARNING = "#FFD93D";    // 警告色（霓虹黃色）
    
    // 文字顏色
    const QString TEXT_PRIMARY = "#EAEAEA";      // 主要文字顏色
    
    // 邊框顏色
    const QString BORDER = "#2A4066";            // 邊框顏色
    
    // 懸停效果顏色
    const QString HOVER_BORDER = "#00D9FF";
    const QString HOVER_BG = "rgba(0, 217, 255, 0.2)";
    const QString PRESSED_BG = "rgba(0, 217, 255, 0.4)";
}

#endif // THEME_H
