# 傳送陣模式實作說明 (Teleportation Mode Implementation)

## 概述 (Overview)

傳送陣模式是一個新的遊戲模式，會在棋盤上隨機選擇兩個空格子並標記為銀色塗層的傳送門。當棋子移動到其中一個傳送門時，會自動傳送到另一個傳送門的位置，然後重新隨機選擇兩個新的空格子作為傳送門。

The teleportation mode is a new game mode that randomly selects two empty squares on the board and marks them with a silver coating as teleportation portals. When a piece lands on one of the portals, it is automatically teleported to the other portal's position, and then two new random empty squares are selected as the new portals.

## 功能特點 (Features)

1. **隨機傳送門選擇** - 在兩個沒有棋子的空格子上隨機放置傳送門
2. **銀色塗層視覺效果** - 傳送門方格顯示為銀色 (RGB: 192, 192, 192)
3. **自動傳送** - 當棋子移動到傳送門時自動傳送到另一個傳送門
4. **動態重置** - 傳送發生後，立即選擇兩個新的隨機空格子作為新傳送門
5. **多模式兼容** - 可以與霧戰模式、地吸引力模式等其他模式同時啟用

## 實作細節 (Implementation Details)

### 1. 狀態變數 (State Variables)

在 `qt_chess.h` 中新增以下變數：

```cpp
// 傳送陣模式相關 (Teleportation Mode)
bool m_teleportModeEnabled;          // 是否啟用傳送陣模式
QPoint m_teleportPortal1;            // 傳送門位置1
QPoint m_teleportPortal2;            // 傳送門位置2
```

### 2. 初始化 (Initialization)

在建構函式中初始化：
```cpp
, m_teleportModeEnabled(false)
, m_teleportPortal1(-1, -1)
, m_teleportPortal2(-1, -1)
```

### 3. 核心函數 (Core Functions)

#### `initializeTeleportPortals()`
- 初始化傳送門位置
- 調用 `resetTeleportPortals()` 來選擇初始位置

#### `resetTeleportPortals()`
- 收集所有空格子的位置
- 隨機選擇兩個不同的空格子
- 更新 `m_teleportPortal1` 和 `m_teleportPortal2`
- 調用 `updateBoard()` 更新視覺效果

#### `isTeleportPortal(int row, int col)`
- 檢查指定位置是否為傳送門
- 返回布林值

#### `handleTeleportation(const QPoint& from, const QPoint& to)`
- 檢查目標位置是否為傳送門
- 確定另一個傳送門的位置
- 檢查目標傳送門是否為空
- 將棋子移動到另一個傳送門
- 調用 `resetTeleportPortals()` 重新選擇新位置

### 4. 視覺效果 (Visual Effects)

在 `updateSquareColor()` 函數中新增：

```cpp
// 檢查是否為傳送門位置，添加銀色塗層
if (m_teleportModeEnabled && isTeleportPortal(logicalRow, logicalCol)) {
    // 銀色塗層效果 - 使用半透明的銀色覆蓋
    color = QColor(192, 192, 192);  // 銀色
}
```

### 5. 整合點 (Integration Points)

傳送處理被整合到以下四個移動處理點：

1. **點擊移動** (`onSquareClicked`) - 第2336-2341行
2. **拖放移動** (`mouseReleaseEvent`) - 第3363-3368行
3. **電腦移動** (`onEngineBestMove`) - 第5206-5211行
4. **線上對手移動** (`onOpponentMove`) - 第5886-5891行

每個位置在應用地吸引力模式之後添加：
```cpp
// 處理傳送陣模式（如果啟用）
if (m_teleportModeEnabled) {
    handleTeleportation(from, to);
    needsUpdate = true;
}
```

### 6. 模式啟用/停用 (Mode Enable/Disable)

在 `onStartGameReceived()` 中啟用：
```cpp
// 檢查是否啟用傳送陣模式
if (m_selectedGameModes.contains("傳送陣") && m_selectedGameModes["傳送陣"]) {
    m_teleportModeEnabled = true;
    qDebug() << "[Qt_Chess::onStartGameReceived] Teleportation mode enabled";
    
    // 初始化傳送門位置
    initializeTeleportPortals();
} else {
    m_teleportModeEnabled = false;
    m_teleportPortal1 = QPoint(-1, -1);
    m_teleportPortal2 = QPoint(-1, -1);
}
```

在各種重置函數中停用：
- `resetGameState()` - 主遊戲重置
- `onHumanModeClicked()` - 切換到雙人模式
- `onComputerModeClicked()` - 切換到電腦模式
- `onCancelRoomClicked()` - 取消線上房間
- `onExitRoomClicked()` - 退出線上房間

## 使用方式 (Usage)

1. 進入線上對戰模式
2. 創建或加入房間時，在遊戲模式對話框中勾選「🔮 傳送陣」
3. 開始遊戲後，會看到棋盤上有兩個銀色格子（傳送門）
4. 移動棋子到銀色格子上
5. 棋子會自動傳送到另一個銀色格子
6. 傳送後，兩個新的銀色格子會出現在其他位置

## 技術特點 (Technical Features)

### 隨機性 (Randomness)
使用 `QRandomGenerator::global()->bounded()` 確保真隨機選擇

### 安全性 (Safety)
- 檢查是否有足夠的空格子（至少2個）
- 確保兩個傳送門不在同一位置
- 檢查目標傳送門是否為空才執行傳送

### 效能 (Performance)
- 使用 `QVector` 收集空格子位置
- O(64) 時間複雜度掃描棋盤
- O(1) 時間複雜度檢查是否為傳送門

## 調試日誌 (Debug Logging)

實作包含以下調試日誌：
- 傳送陣模式啟用時
- 重置傳送門位置時
- 棋子傳送時
- 傳送失敗時（目標傳送門不為空）

## 未來改進 (Future Improvements)

可能的改進方向：
1. 添加傳送動畫效果
2. 添加傳送音效
3. 支援更多傳送門（3個或4個）
4. 添加傳送門冷卻時間
5. 視覺效果增強（發光、脈衝等）

## 兼容性 (Compatibility)

- 與所有現有遊戲模式兼容（雙人、電腦、線上）
- 可與霧戰模式同時啟用
- 可與地吸引力模式同時啟用
- 不影響其他遊戲功能（升變、王車易位、吃過路兵等）

## 程式碼位置 (Code Locations)

- 標頭檔案聲明：`src/qt_chess.h` 第264-268行、第478-481行
- 實作函數：`src/qt_chess.cpp` 第7919-8014行
- 初始化：`src/qt_chess.cpp` 第255-259行
- 視覺效果：`src/qt_chess.cpp` 第1944-1949行
- 模式啟用：`src/qt_chess.cpp` 第6168-6180行
