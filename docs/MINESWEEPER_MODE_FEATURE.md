# 地雷模式功能 (Minesweeper Mode Feature)

## 概述

地雷模式是一個創新的遊戲變體，將經典的踩地雷遊戲機制融入西洋棋中。在這個模式下，棋盤上會隨機放置地雷，當棋子移動到有地雷的格子時會被炸毀。這為傳統西洋棋增添了新的策略層面和刺激感。

## 功能特色

### 🎯 核心功能

- **可自訂地雷數量**：支援 1-20 個地雷，預設 8 個
- **智能放置**：地雷只會出現在棋盤中間 4 行（第 2-5 行）的空格
- **踩雷懲罰**：棋子踩到地雷會被炸毀並計入被吃棋子
- **數字提示**：顯示周圍 8 個方格的地雷數量
- **視覺標示**：
  - 💣 符號表示已觸發的地雷
  - 數字 1-8 以不同顏色顯示周圍地雷數量
  - 踩雷格子有紅色背景高亮

### 🎨 視覺效果

地雷數字使用以下顏色方案：
- **1**：藍色 (#0000FF)
- **2**：綠色 (#008000)
- **3**：紅色 (#FF0000)
- **4**：深藍色 (#000080)
- **5**：褐紅色 (#800000)
- **6**：青色 (#008080)
- **7**：黑色 (#000000)
- **8**：灰色 (#808080)

## 使用方法

### 啟用地雷模式

1. 在主選單選擇「雙人」、「電腦」或「線上」模式
2. 在右側遊戲設定面板找到「💣 地雷模式」區域
3. 勾選「啟用」複選框
4. 使用滑桿調整地雷數量（1-20）
5. 點擊「▶ 開始對弈」按鈕

### 遊戲進行

1. **地雷放置**：開始遊戲後，指定數量的地雷會隨機放置在棋盤中間區域
2. **正常移動**：按照正常西洋棋規則移動棋子
3. **踩雷事件**：
   - 當棋子移動到有地雷的格子
   - 棋子會被炸毀並從棋盤移除
   - 顯示警告訊息：「💣 [顏色]的棋子踩到地雷被炸毀了！」
   - 被炸毀的棋子加入被吃棋子列表
4. **方格揭開**：
   - 棋子移動到的格子會被「揭開」
   - 顯示該格周圍的地雷數量（如果有）
   - 已揭開的空格會顯示數字提示

### 策略提示

1. **觀察數字**：注意已揭開格子的數字，推測地雷位置
2. **避開危險區**：如果某格周圍有多個高數字，該區域可能有多個地雷
3. **計算風險**：在關鍵時刻，可能需要冒險穿越可疑區域
4. **保護重要棋子**：避免讓王、后等重要棋子進入高風險區域

## 技術實現

### 資料結構

```cpp
// ChessBoard 類別中的地雷相關成員
bool m_minesweeperEnabled;                    // 是否啟用地雷模式
std::vector<std::vector<bool>> m_minePositions;    // 地雷位置 (8x8)
std::vector<std::vector<bool>> m_revealedSquares;  // 已揭開的方格 (8x8)
```

### 主要方法

1. **enableMinesweeperMode(bool enable, int mineCount)**
   - 啟用或禁用地雷模式
   - 設定地雷數量並重新放置地雷

2. **placeMines(int count)**
   - 在棋盤中間 4 行隨機放置指定數量的地雷
   - 確保地雷不會放在有棋子的位置

3. **hasMine(int row, int col)**
   - 檢查指定位置是否有地雷

4. **getAdjacentMineCount(int row, int col)**
   - 計算指定位置周圍 8 個方格的地雷數量

5. **revealSquare(int row, int col)**
   - 標記指定位置為已揭開狀態

### 移動處理邏輯

```cpp
// 在 movePiece() 方法中的地雷處理
if (m_minesweeperEnabled) {
    // 揭開目標方格
    revealSquare(to.y(), to.x());
    
    // 如果踩到地雷，移除該棋子
    if (hasMine(to.y(), to.x())) {
        ChessPiece destroyedPiece = m_board[to.y()][to.x()];
        m_board[to.y()][to.x()] = ChessPiece(PieceType::None, PieceColor::None);
        
        // 將被炸毀的棋子加入被吃掉列表
        if (destroyedPiece.getColor() == PieceColor::White) {
            m_capturedWhite.push_back(destroyedPiece);
        } else if (destroyedPiece.getColor() == PieceColor::Black) {
            m_capturedBlack.push_back(destroyedPiece);
        }
    }
}
```

## 設定持久化

地雷模式的設定會使用 QSettings 儲存：

```cpp
void Qt_Chess::saveMinesweeperSettings() {
    QSettings settings("Qt_Chess", "MinesweeperSettings");
    settings.setValue("enabled", m_minesweeperEnabled);
    settings.setValue("mineCount", m_mineCount);
}

void Qt_Chess::loadMinesweeperSettings() {
    QSettings settings("Qt_Chess", "MinesweeperSettings");
    m_minesweeperEnabled = settings.value("enabled", false).toBool();
    m_mineCount = settings.value("mineCount", 8).toInt();
}
```

## 相容性

地雷模式與以下功能完全相容：
- ✅ 雙人對弈模式
- ✅ AI 人機對弈
- ✅ 線上對戰模式
- ✅ 時間控制
- ✅ 棋譜記錄
- ✅ 回放功能
- ✅ 所有特殊走法（王車易位、吃過路兵、兵升變）

## 限制與注意事項

1. **地雷放置範圍**：地雷只會放置在第 2-5 行（索引 2-5），避免影響起始棋子
2. **最大地雷數**：限制為 20 個，避免棋盤過於危險
3. **不影響將軍/將死判定**：地雷模式不改變正常的西洋棋勝負條件
4. **線上模式同步**：在線上模式下，雙方玩家應使用相同的地雷設定（目前版本需手動協調）

## 未來改進方向

1. **線上模式同步**：自動同步地雷配置給對手
2. **自訂放置模式**：允許玩家手動選擇地雷位置
3. **遊戲預設**：提供多種預設配置（簡單、中等、困難）
4. **統計追蹤**：記錄踩雷次數和倖存率
5. **音效增強**：為踩雷事件添加專屬音效
6. **動畫效果**：為地雷爆炸添加視覺動畫

## 版本歷史

- **v1.0** (2025-12-15)：初始實現
  - 基本地雷放置和偵測
  - 數字提示顯示
  - 設定持久化
  - UI 控制項整合
