# 吃子重疊顯示修復 (Captured Pieces Overlap Fix)

## 問題描述 (Problem Description)
吃子紀錄重複的子無法重疊 - Duplicate captured pieces in the capture records could not overlap properly.

## 根本原因 (Root Cause)
QLabel 的 HTML 渲染引擎 (QTextDocument) 不支援負的 CSS 邊距 (`margin-left: -8px`)，導致原本應該重疊的重複棋子無法正確顯示。

QLabel's HTML rendering engine (QTextDocument) does not support negative CSS margins (`margin-left: -8px`), which prevented duplicate pieces from overlapping as intended.

## 解決方案 (Solution)

### 修改前 (Before)
```cpp
int leftMargin = (sameTypeCount > 0) ? -8 : 0;
html += QString("<span style='margin-left: %1px; ...'>...</span>")
            .arg(leftMargin);
```
**問題**: 負邊距在 QTextDocument 中不起作用
**Problem**: Negative margins don't work in QTextDocument

### 修改後 (After)
```cpp
const int PIECE_WIDTH = 18;        // 棋子寬度
const int OVERLAP_OFFSET = 8;      // 重疊偏移量
const int CONTAINER_HEIGHT = 20;   // 容器高度

int currentLeft = 0;  // 追蹤水平位置

// 相同類型的棋子重疊
if (type == lastPieceType) {
    currentLeft += (PIECE_WIDTH - OVERLAP_OFFSET);  // 10px
}
// 不同類型的棋子不重疊
else if (i > 0) {
    currentLeft += PIECE_WIDTH;  // 18px
}

html += QString("<div style='position:relative; ...'>");
html += QString("<span style='position:absolute; left:%1px; ...'>...</span>")
            .arg(currentLeft);
html += "</div>";
```
**優點 (Advantages)**:
- ✅ 使用絕對定位 (absolute positioning)，QTextDocument 完全支援
- ✅ 位置計算清晰明確
- ✅ 使用常數提高可維護性

## 技術細節 (Technical Details)

### 棋子尺寸 (Piece Dimensions)
- **PIECE_WIDTH**: 18px (棋子圖示寬度)
- **OVERLAP_OFFSET**: 8px (重疊偏移量)
- **CONTAINER_HEIGHT**: 20px (容器高度)

### 位置計算 (Position Calculation)
1. **相同類型的棋子 (Same-type pieces)**: 
   - 間距 = PIECE_WIDTH - OVERLAP_OFFSET = 10px
   - 視覺效果：重疊 8px

2. **不同類型的棋子 (Different-type pieces)**:
   - 間距 = PIECE_WIDTH = 18px
   - 視覺效果：完全不重疊

### 範例輸出 (Example Output)
輸入: [兵, 兵, 騎士, 騎士]
Input: [Pawn, Pawn, Knight, Knight]

輸出位置 (Output positions):
```
Pawn 1:   left:0px
Pawn 2:   left:10px   (與 Pawn 1 重疊 8px)
Knight 1: left:28px   (與 Pawn 2 不重疊，28 = 10 + 18)
Knight 2: left:38px   (與 Knight 1 重疊 8px，38 = 28 + 10)
```

## 修改的檔案 (Modified Files)
- `src/qt_chess.cpp` (renderCapturedPieces 函數，第 2028-2125 行)

## 測試驗證 (Testing & Verification)
- ✅ 編譯成功無錯誤
- ✅ 測試程式驗證位置正確
- ✅ 程式碼審查意見已處理
- ✅ 安全掃描通過

## Qt 限制說明 (Qt Limitations)
QTextDocument (QLabel 的 RichText 模式) 的 CSS 支援有限：
- ❌ 負邊距 (Negative margins) 不支援
- ✅ 絕對定位 (Absolute positioning) 支援
- ✅ 相對定位 (Relative positioning) 支援
- ✅ 基本行內樣式 (Basic inline styles) 支援

## 未來參考 (Future Reference)
在 Qt 應用中使用 QLabel 顯示 HTML 時，避免使用負邊距。改用絕對定位或其他 QTextDocument 支援的 CSS 屬性。

When using QLabel to display HTML in Qt applications, avoid negative margins. Use absolute positioning or other CSS properties supported by QTextDocument instead.

## 相關連結 (Related Links)
- Qt Documentation: [QTextDocument Supported HTML Subset](https://doc.qt.io/qt-5/richtext-html-subset.html)
- Issue: 找出吃子紀錄重複的子無法重疊原因
- PR: copilot/find-duplicate-eat-records
