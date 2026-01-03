#ifndef THEME_H
#define THEME_H

#include <QString>

// ===== 歐式古典風格主題顏色（European Classical Theme）=====
// 共用的主題配色常數，確保整個應用程式的視覺一致性

namespace Theme {
    // 背景顏色 - 歐式古典色調
    const QString BG_DARK = "#E8DCC8";           // 米白色背景（羊皮紙色）
    const QString BG_MEDIUM = "#F0E8D8";         // 淺米色背景
    const QString BG_PANEL = "#F5F0E5";          // 象牙白面板
    
    // 強調色 - 歐式古典色調
    const QString ACCENT_PRIMARY = "#8B4513";    // 深褐色（主要強調）
    const QString ACCENT_SECONDARY = "#B8860B";  // 深金色（次要強調）
    const QString ACCENT_SUCCESS = "#6B4423";    // 深木色（成功色）
    const QString ACCENT_WARNING = "#CD853F";    // 秘魯褐色（警告色）
    
    // 文字顏色
    const QString TEXT_PRIMARY = "#3E2723";      // 深褐色文字
    
    // 邊框顏色
    const QString BORDER = "#A0826D";            // 古銅色邊框
    
    // 懸停效果顏色
    const QString HOVER_BORDER = "#8B4513";
    const QString HOVER_BG = "rgba(139, 69, 19, 0.2)";
    const QString PRESSED_BG = "rgba(107, 68, 35, 0.3)";
}

#endif // THEME_H
