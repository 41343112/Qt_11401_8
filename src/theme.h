#ifndef THEME_H
#define THEME_H

#include <QString>

// ===== 現代科技風格主題顏色（基於 chess.jpg 的霓虹電路板主題）=====
// 共用的主題配色常數，確保整個應用程式的視覺一致性

namespace Theme {
    // 背景顏色
    const QString BG_DARK = "#0A1628";           // 深色背景（深藍黑色）
    const QString BG_MEDIUM = "#0D1F3C";         // 中等深度背景（深藍色）
    const QString BG_PANEL = "#0F2940";          // 面板背景（霓虹藍色調）
    
    // 強調色
    const QString ACCENT_PRIMARY = "#00FFFF";    // 主要強調色（霓虹青色）
    const QString ACCENT_SECONDARY = "#FF9955";  // 次要強調色（霓虹橙色）
    const QString ACCENT_SUCCESS = "#00D9FF";    // 成功色（青色）
    const QString ACCENT_WARNING = "#FFB366";    // 警告色（霓虹橙黃色）
    
    // 文字顏色
    const QString TEXT_PRIMARY = "#E8F4F8";      // 主要文字顏色（淺青白色）
    
    // 邊框顏色
    const QString BORDER = "#1A3F5C";            // 邊框顏色（深青藍色）
    
    // 懸停效果顏色
    const QString HOVER_BORDER = "#00FFFF";
    const QString HOVER_BG = "rgba(0, 255, 255, 0.2)";
    const QString PRESSED_BG = "rgba(0, 255, 255, 0.4)";
}

#endif // THEME_H
