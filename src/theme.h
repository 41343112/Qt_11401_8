#ifndef THEME_H
#define THEME_H

#include <QString>

// ===== 明亮現代風格主題顏色 =====
// 共用的主題配色常數，確保整個應用程式的視覺一致性

namespace Theme {
    // 背景顏色（淺色）
    const QString BG_DARK = "#F5F9FC";           // 極淺藍白色背景
    const QString BG_MEDIUM = "#E8F4F8";         // 淡藍白色背景
    const QString BG_PANEL = "#D4E8F0";          // 柔和淺藍面板
    
    // 強調色（適合淺色背景）
    const QString ACCENT_PRIMARY = "#0099CC";    // 深青藍色
    const QString ACCENT_SECONDARY = "#E91E63";  // 活力洋紅色
    const QString ACCENT_SUCCESS = "#00C853";    // 明亮綠色
    const QString ACCENT_WARNING = "#FF9800";    // 明亮橙色
    
    // 文字顏色（深色以確保可讀性）
    const QString TEXT_PRIMARY = "#1A1A1A";      // 深灰色文字
    
    // 邊框顏色
    const QString BORDER = "#90CAF9";            // 淺藍色邊框
    
    // 懸停效果顏色
    const QString HOVER_BORDER = "#0099CC";
    const QString HOVER_BG = "rgba(0, 153, 204, 0.2)";
    const QString PRESSED_BG = "rgba(0, 153, 204, 0.4)";
}

#endif // THEME_H
