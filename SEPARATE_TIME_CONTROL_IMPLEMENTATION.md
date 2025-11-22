# 雙方獨立時間控制實現 (Separate Time Control Implementation)

## 需求描述 (Requirements)

根據問題陳述：
- 總時間最大30分鐘 (Maximum total time: 30 minutes)
- 可以單獨控制雙方的時間 (Can separately control time for both sides)
- 當點擊開始時 (When clicking start):
  - 隱藏時間控制 (Hide time controls)
  - 棋盤置中 (Center the chessboard)
  - 時間顯示在棋盤左側上方、右側下方 (Display time on upper left and lower right of the chessboard)

## 實現概述 (Implementation Overview)

### ✅ 完成的功能 (Completed Features)

#### 1. 獨立時間控制 (Separate Time Controls)
- **白方時間滑桿**: 獨立控制白方的時間限制（0=不限時，1=30秒，2-31=1-30分鐘）
- **黑方時間滑桿**: 獨立控制黑方的時間限制（0=不限時，1=30秒，2-31=1-30分鐘）
- **每著加秒滑桿**: 0-60秒的增量設定，對雙方皆適用

#### 2. 時間上限調整 (Time Limit Adjustment)
- 最大時間從 60 分鐘降低到 30 分鐘
- 滑桿範圍更新：MAX_SLIDER_POSITION = 31（0, 1, 2-31）
- 常數更新：MAX_MINUTES = 30

#### 3. UI 行為變更 (UI Behavior Changes)

**遊戲開始前 (Before Game Starts)**:
```
+--------------------------------------------------+
|  Menu Bar                                         |
+--------------------------------------------------+
|  +--------------------+  +--------------------+  |
|  |                    |  | 時間控制           |  |
|  |   Chess Board      |  | - 白方時間滑桿     |  |
|  |   (visible)        |  | - 黑方時間滑桿     |  |
|  |                    |  | - 每著加秒滑桿     |  |
|  |                    |  | - [開始] 按鈕      |  |
|  +--------------------+  | - [新遊戲] 按鈕    |  |
|                          +--------------------+  |
+--------------------------------------------------+
```

**點擊開始後 (After Clicking Start)**:
```
+--------------------------------------------------+
|  Menu Bar                                         |
+--------------------------------------------------+
|                                                  |
|         +--------------------------------+        |
|         | 黑:10:00                       |        |
|         |                                |        |
|         |       Chess Board              |        |
|         |       (centered)               |        |
|         |                                |        |
|         |                       白:10:00 |        |
|         +--------------------------------+        |
|                                                  |
+--------------------------------------------------+
```

時間標籤疊加在棋盤上：
- **黑方時間**: 左上角（upper left）
- **白方時間**: 右下角（lower right）

#### 4. 時間控制邏輯 (Timer Logic)

**獨立計時**:
- 白方和黑方各自獨立計時
- 支援一方不限時，另一方有時間限制
- 只有設定了時間限制的一方才會倒數計時

**時間用盡處理**:
```cpp
if (currentPlayer == PieceColor::White && m_whiteTimeMs > 0) {
    m_whiteTimeMs -= 100;
    if (m_whiteTimeMs <= 0) {
        // 白方超時，黑方獲勝
    }
}
```

**不限時顯示**:
- 時間為 0 時顯示 "不限時"
- 時間 > 0 時顯示 "mm:ss" 格式

## 技術細節 (Technical Details)

### 修改的檔案 (Modified Files)

#### 1. qt_chess.h

**新增成員變數**:
```cpp
// Separate time control sliders
QSlider* m_whiteTimeLimitSlider;
QLabel* m_whiteTimeLimitLabel;
QSlider* m_blackTimeLimitSlider;
QLabel* m_blackTimeLimitLabel;

// Panel container for show/hide
QWidget* m_timeControlPanel;
```

**新增方法**:
```cpp
void onWhiteTimeLimitChanged(int value);
void onBlackTimeLimitChanged(int value);
void hideTimeControlPanel();
void showTimeControlPanel();
void positionOverlayTimeLabels();
```

#### 2. qt_chess.cpp

**新增常數**:
```cpp
const int MAX_TIME_LIMIT_SECONDS = 1800;  // 30 minutes
const int MAX_SLIDER_POSITION = 31;        // 0-31
const int MAX_MINUTES = 30;                // 30 minutes
const int TIME_LABEL_MARGIN = 10;          // Margin for overlays
```

**主要函數變更**:

1. **setupUI()**: 
   - 添加 boardContainerLayout 來居中棋盤
   - 創建時間標籤為絕對定位的疊加層
   - 將時間控制面板包裝在 m_timeControlPanel 中

2. **setupTimeControlUI()**:
   - 替換單一時間滑桿為白方和黑方獨立滑桿
   - 移除計時器顯示（改為疊加顯示）

3. **onWhiteTimeLimitChanged() / onBlackTimeLimitChanged()**:
   - 處理各自的時間限制變更
   - 更新 m_timeControlEnabled 狀態
   - 只要任一方有時間限制即啟用

4. **onStartButtonClicked()**:
   - 隱藏時間控制面板
   - 顯示疊加時間標籤
   - 定位時間標籤到棋盤上
   - 啟動計時器

5. **onNewGameClicked()**:
   - 顯示時間控制面板
   - 隱藏疊加時間標籤
   - 根據滑桿重置時間

6. **positionOverlayTimeLabels()**:
   - 統一的定位邏輯
   - 黑方：左上角（boardRect.topLeft + margin）
   - 白方：右下角（boardRect.bottomRight - size - margin）

7. **updateTimeDisplays()**:
   - 支援顯示 "不限時" 當時間為 0
   - 格式化時間為 "mm:ss"
   - 高亮當前玩家的計時器

8. **onGameTimerTick()**:
   - 檢查當前玩家是否有時間限制
   - 只對有時間限制的玩家倒數計時
   - 時間用盡時顯示對話框並恢復面板

9. **resizeEvent()**:
   - 窗口調整大小時重新定位疊加標籤

10. **loadTimeControlSettings() / saveTimeControlSettings()**:
    - 分別保存白方和黑方的時間設定
    - 設定鍵：`whiteTimeLimitSeconds` 和 `blackTimeLimitSeconds`

## 使用說明 (Usage Instructions)

### 設定時間控制 (Setting Time Control)

1. **白方時間**:
   - 移動白方時間滑桿
   - 0 = 不限時
   - 1 = 30 秒
   - 2-31 = 1-30 分鐘

2. **黑方時間**:
   - 移動黑方時間滑桿
   - 範圍與白方相同

3. **每著加秒**:
   - 設定 0-60 秒
   - 每走一步後自動增加

### 開始遊戲 (Starting the Game)

1. 點擊 **新遊戲** 按鈕
2. 設定白方和黑方的時間
3. 設定每著加秒（可選）
4. 點擊 **開始** 按鈕
   - 時間控制面板隱藏
   - 棋盤自動居中
   - 時間標籤顯示在棋盤上

### 遊戲進行中 (During the Game)

- 計時器自動切換
- 當前玩家的計時器以綠色高亮
- 時間標籤隨窗口調整大小自動重新定位

### 重新開始 (Restarting)

- 點擊 **新遊戲** 返回設定畫面
- 時間控制面板重新顯示
- 可以調整新的時間設定

## 特殊情況處理 (Special Cases)

### 1. 雙方都不限時
- 開始按鈕被禁用
- 無法啟動計時器

### 2. 一方不限時，另一方有限時
- 開始按鈕啟用
- 只有有限時的一方倒數計時
- 不限時的一方顯示 "不限時"

### 3. 時間用盡
- 停止計時器
- 顯示勝負結果
- 恢復時間控制面板
- 隱藏疊加時間標籤

## 代碼品質 (Code Quality)

### ✅ 代碼審查改進
1. 提取 `positionOverlayTimeLabels()` 避免重複代碼
2. 添加 `TIME_LABEL_MARGIN` 常數
3. 確保棋盤正確居中

### ✅ 安全檢查
- CodeQL 掃描：通過（無安全漏洞）

## 測試建議 (Testing Recommendations)

### 基本功能測試
1. ✓ 設定白方 5 分鐘，黑方 3 分鐘，開始遊戲
2. ✓ 確認時間標籤正確顯示在棋盤上
3. ✓ 確認計時器正確倒數
4. ✓ 走棋後確認增量正確添加

### 邊界測試
1. ✓ 白方不限時，黑方 1 分鐘
2. ✓ 黑方不限時，白方 30 秒
3. ✓ 雙方都不限時（開始按鈕應禁用）

### UI 測試
1. ✓ 調整窗口大小，確認時間標籤正確重新定位
2. ✓ 點擊新遊戲，確認面板正確恢復
3. ✓ 確認棋盤在隱藏面板後居中顯示

### 超時測試
1. ✓ 設定短時間（30 秒）測試超時
2. ✓ 確認超時對話框正確顯示
3. ✓ 確認面板正確恢復

## 向後兼容性 (Backward Compatibility)

### 設定遷移
- 舊設定鍵 `timeLimitSeconds` 不再使用
- 新設定鍵：
  - `whiteTimeLimitSeconds`
  - `blackTimeLimitSeconds`
  - `incrementSeconds`（保持不變）

### 預設行為
- 首次啟動時，雙方時間都預設為「不限時」
- 增量預設為 0 秒

## 未來改進建議 (Future Improvements)

1. **快速設定按鈕**
   - 一鍵設定常見時間控制（如 3+2, 5+3, 10+0）

2. **時間警告**
   - 剩餘時間少於 30 秒時改變顏色或閃爍

3. **暫停功能**
   - 允許暫停/恢復計時器

4. **時間記錄**
   - 記錄每步耗時
   - 顯示時間統計

5. **延遲計時**
   - 支援 Fischer 延遲
   - 支援 Bronstein 延遲

## 結論 (Conclusion)

實現已完成所有需求：
- ✅ 最大時間 30 分鐘
- ✅ 獨立控制雙方時間
- ✅ 點擊開始時隱藏控制面板
- ✅ 棋盤自動居中
- ✅ 時間疊加顯示（左上/右下）
- ✅ 代碼審查通過
- ✅ 安全檢查通過

實現完整且穩健，已準備好進行測試和使用！
