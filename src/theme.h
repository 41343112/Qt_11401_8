#ifndef THEME_H
#define THEME_H

#include <QString>

// ===== 簡約風格主題顏色（Minimalist Design Theme）=====
// 共用的主題配色常數，確保整個應用程式的視覺一致性

namespace Theme {
    // 背景顏色 - 簡約中性色調
    const QString BG_DARK = "#F5F5F5";           // 淺灰背景
    const QString BG_MEDIUM = "#FAFAFA";         // 極淺灰背景
    const QString BG_PANEL = "#FFFFFF";          // 白色面板
    
    // 強調色 - 簡約黑灰色調
    const QString ACCENT_PRIMARY = "#2C2C2C";    // 深灰（主要強調）
    const QString ACCENT_SECONDARY = "#666666";  // 中灰（次要強調）
    const QString ACCENT_SUCCESS = "#4A4A4A";    // 成功色（深中灰）
    const QString ACCENT_WARNING = "#757575";    // 警告色（中灰）
    
    // 文字顏色
    const QString TEXT_PRIMARY = "#333333";      // 深灰文字
    
    // 邊框顏色
    const QString BORDER = "#E0E0E0";            // 淺灰邊框
    
    // 懸停效果顏色
    const QString HOVER_BORDER = "#B0B0B0";
    const QString HOVER_BG = "rgba(200, 200, 200, 0.3)";
    const QString PRESSED_BG = "rgba(150, 150, 150, 0.4)";
}

#endif // THEME_H
