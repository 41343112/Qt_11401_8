# 骰子模式功能說明 (Dice Mode Feature)

## 概述 (Overview)

骰子模式是線上多人對戰專屬的遊戲變體模式，為遊戲增加了隨機性和策略深度。在此模式下，玩家每回合只能移動三個隨機選定的棋子之一，這些棋子都是當前有合法移動的棋子。

## 功能特色 (Features)

### 1. 隨機棋子選擇 (Random Piece Selection)
- 每回合開始時，系統會自動投擲骰子
- 從當前玩家所有**能夠合法移動**的棋子中隨機選擇3個
- 如果合法可動棋子少於3個，則顯示所有可動棋子

### 2. 視覺反饋系統 (Visual Feedback System)

骰子顯示在棋盤右側，包含3個骰子框：

#### 邊框顏色含義：
- **橘色邊框** (#FFA500)：未使用的骰子
- **綠色邊框** (#00FF00)：當前選中的棋子
- **灰色邊框** (#808080)：已使用的骰子

#### 骰子顯示內容：
- 棋子符號（♔ ♕ ♖ ♗ ♘ ♙）
- 棋子中文名稱（國王、皇后、城堡、主教、騎士、兵）

### 3. 移動限制 (Move Restrictions)
- 玩家**只能**選擇骰子中顯示的三個棋子之一進行移動
- 嘗試移動不在骰子中的棋子時，會顯示錯誤訊息：
  ```
  當前骰子為：♔ 國王、♕ 皇后、♖ 城堡
  ```

### 4. 自動重新生成 (Automatic Regeneration)
- 玩家完成移動後，使用的骰子會變成灰色
- 當對手完成移動後，骰子會**立即重新投擲**
- 新的骰子只包含當前玩家可合法移動的棋子

### 5. 合法移動驗證 (Legal Move Validation)
骰子系統確保：
- 只有在西洋棋規則中能夠移動的棋子才會出現在骰子中
- 不會出現無法使用的骰子（如被困住或移動後會導致將軍的棋子）
- 自動考慮將軍、釘子等特殊情況

## 技術實現 (Technical Implementation)

### 核心函數 (Core Functions)

1. **initializeDiceMode()**
   - 初始化骰子UI面板
   - 創建3個骰子顯示框
   - 設置初始樣式

2. **rollDice()**
   - 獲取所有合法可動棋子
   - 使用 Fisher-Yates shuffle 隨機選擇3個
   - 更新骰子顯示

3. **getLegalMovablePieces(PieceColor color)**
   - 遍歷棋盤上所有棋子
   - 檢查每個棋子是否有合法移動
   - 返回可移動棋子的位置列表

4. **isPieceInDice(const QPoint& pos)**
   - 檢查指定位置的棋子是否在當前骰子中
   - 用於驗證玩家選擇

5. **markDiceAsUsed(const QPoint& pos)**
   - 標記使用過的骰子
   - 更新視覺狀態為灰色

6. **updateDiceDisplay()**
   - 更新骰子UI顯示
   - 根據狀態應用對應的邊框顏色
   - 顯示棋子符號和名稱

### 數據結構 (Data Structures)

```cpp
struct DicePiece {
    QPoint position;     // 棋子位置
    PieceType type;      // 棋子類型
    bool used;           // 是否已使用
};

std::vector<DicePiece> m_diceRoll;  // 當前骰子結果（3個）
```

### 遊戲流程整合 (Game Flow Integration)

1. **遊戲開始時**
   - 檢查 `m_selectedGameModes["骰子"]` 是否為 true
   - 啟用骰子模式並初始化UI
   - 為當前玩家投擲第一次骰子

2. **玩家選擇棋子時**
   - 檢查棋子是否在骰子中
   - 如果不在，顯示錯誤訊息並拒絕選擇
   - 如果在，允許選擇並更新骰子顯示（綠色邊框）

3. **玩家完成移動後**
   - 標記使用的骰子為灰色
   - 發送移動給對手

4. **對手完成移動後**
   - 立即重新投擲骰子
   - 更新骰子顯示

5. **遊戲結束或返回主選單時**
   - 重置骰子模式狀態
   - 隱藏骰子UI面板

## 使用方式 (How to Use)

### 創建骰子模式遊戲

1. 點擊「線上」按鈕進入線上對戰
2. 選擇「我要創建房間」
3. 在遊戲模式選擇對話框中勾選「🎲 骰子」
4. 點擊「確定」創建房間
5. 等待對手加入並開始遊戲

### 遊戲中

1. **查看骰子**：骰子顯示在棋盤右側
2. **選擇棋子**：只能選擇骰子中的三個棋子之一
3. **移動棋子**：正常移動選中的棋子
4. **觀察更新**：
   - 使用後骰子變灰
   - 對手回合結束後骰子自動更新

## 策略提示 (Strategy Tips)

1. **適應隨機性**：由於棋子是隨機的，需要靈活調整策略
2. **保持選擇多樣性**：盡量讓多個棋子保持可移動狀態
3. **預測對手**：對手也受到骰子限制，可以根據其棋子分佈猜測可能的移動
4. **緊急應變**：即使在將軍情況下，骰子也只顯示可解除將軍的棋子

## 限制與注意事項 (Limitations and Notes)

1. **僅限線上模式**：骰子模式只能在線上多人對戰中使用
2. **不與AI對戰**：電腦對弈模式不支援骰子模式
3. **不與本地雙人**：本地雙人模式不支援骰子模式
4. **可與其他模式組合**：骰子模式可以與其他遊戲模式（霧戰、地吸引力等）同時啟用

## 未來改進方向 (Future Improvements)

1. 允許自訂骰子數量（2-5個）
2. 添加骰子重投功能（每局限次）
3. 特殊骰子效果（如萬用骰子）
4. 骰子歷史記錄顯示
5. 統計分析（骰子分佈、使用頻率等）

## 技術參考 (Technical References)

### 相關文件
- `src/qt_chess.h` - 骰子模式類定義
- `src/qt_chess.cpp` - 骰子模式實現（第8213行起）
- `src/onlinedialog.cpp` - 遊戲模式選擇

### 相關函數
- `Qt_Chess::initializeDiceMode()`
- `Qt_Chess::rollDice()`
- `Qt_Chess::getLegalMovablePieces()`
- `Qt_Chess::isPieceInDice()`
- `Qt_Chess::markDiceAsUsed()`
- `Qt_Chess::updateDiceDisplay()`
- `Qt_Chess::getPieceChineseName()`

### 相關成員變數
- `bool m_diceModeEnabled`
- `std::vector<DicePiece> m_diceRoll`
- `QWidget* m_dicePanel`
- `std::vector<QLabel*> m_diceLabels`
- `std::vector<QFrame*> m_diceFrames`
