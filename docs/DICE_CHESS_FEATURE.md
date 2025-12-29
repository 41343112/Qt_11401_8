# 骰子西洋棋模式 (Dice Chess Mode)

## 功能概述

骰子西洋棋模式是一種創新的遊戲變體，為傳統西洋棋增添了隨機性和策略深度。在此模式下，玩家在每回合開始前必須投擲三個骰子，只能移動骰子結果中顯示的棋子類型。

## 核心特性

### 1. 骰子機制
- **三個骰子**：每回合投擲三個虛擬骰子
- **不同棋子**：每個骰子顯示一種不同的棋子類型
- **可選棋子**：兵、騎士、主教、城堡、皇后（國王不受限制，可以隨時移動）
- **移動限制**：玩家只能移動骰子結果中顯示的棋子類型

### 2. 回合規則
- 每回合開始前自動投骰子
- 如果投出的三種棋子類型都無法移動：
  - 顯示提示訊息
  - 自動跳過該玩家的回合
  - 為下一個玩家投骰子
- 如果至少有一種棋子可以移動，玩家必須選擇並移動該類型的棋子

### 3. 線上模式整合
- **僅限線上多人遊戲**：骰子模式只在線上對戰時可用
- **遊戲模式選擇**：在線上遊戲對話框中選擇「🎲 骰子」模式
- **網路同步**：骰子結果在雙方玩家間自動同步
- **主動投擲**：只有當前玩家投骰子，對手接收相同的結果

## 技術實現

### ChessBoard 類別變更

#### 新增成員變數
```cpp
bool m_diceModeEnabled;                 // 骰子模式是否啟用
std::vector<PieceType> m_diceRoll;      // 當前骰子結果
```

#### 新增方法
```cpp
void enableDiceMode(bool enable);                       // 啟用/停用骰子模式
bool isDiceModeEnabled() const;                         // 檢查骰子模式是否啟用
void rollDice();                                        // 投擲骰子
void setDiceRoll(const std::vector<PieceType>&);       // 設置骰子結果（網路同步用）
const std::vector<PieceType>& getDiceRoll() const;     // 獲取骰子結果
bool canMovePieceType(PieceType type) const;            // 檢查棋子是否可移動
bool hasAnyValidMovesWithDice(PieceColor) const;       // 檢查是否有合法移動
```

### Qt_Chess 類別變更

#### UI 元件
```cpp
QWidget* m_diceDisplayWidget;    // 骰子顯示容器
QLabel* m_diceLabel;             // 骰子標題
QLabel* m_diceResult1/2/3;       // 三個骰子結果標籤
```

#### 新增方法
```cpp
void setupDiceDisplay();                            // 設置骰子UI
void updateDiceDisplay();                           // 更新骰子顯示
void rollDiceForTurn();                             // 為當前回合投骰子
QString getPieceTypeSymbol(PieceType) const;        // 獲取棋子符號
QString getPieceTypeName(PieceType) const;          // 獲取棋子名稱
void onDiceRollReceived(const std::vector<PieceType>&); // 接收網路骰子結果
```

### NetworkManager 類別變更

#### 新增訊息類型
```cpp
enum class MessageType {
    // ...
    DiceRoll,  // 骰子結果
    // ...
};
```

#### 新增方法
```cpp
void sendDiceRoll(const std::vector<PieceType>& diceResult);
```

#### 新增信號
```cpp
signals:
    void diceRollReceived(const std::vector<PieceType>& diceResult);
```

## 遊戲流程

### 1. 遊戲開始
```
房主選擇「骰子」模式 → 遊戲開始 → 啟用骰子模式 → 為白方投第一次骰子
```

### 2. 每回合流程
```
投擲骰子 → 檢查可移動棋子 → 
    如果有可移動棋子：
        顯示骰子結果 → 玩家選擇棋子 → 執行移動 → 切換回合
    如果無可移動棋子：
        顯示提示 → 自動跳過 → 切換回合
```

### 3. 網路同步
```
玩家A投骰子 → 發送給伺服器 → 伺服器轉發給玩家B → 雙方顯示相同結果
```

## UI 設計

### 骰子顯示面板
- **位置**：右側時間面板下方
- **內容**：
  - 標題：「🎲 骰子結果」
  - 三個骰子結果卡片
  - 每張卡片顯示：
    - 棋子符號（Unicode）
    - 棋子名稱（中文）
- **樣式**：
  - 現代科技風格
  - 霓虹青色邊框
  - 深色背景

### 棋子符號對應
- ♟ 兵 (Pawn)
- ♞ 騎士 (Knight)
- ♝ 主教 (Bishop)
- ♜ 城堡 (Rook)
- ♛ 皇后 (Queen)

## 使用說明

### 如何啟用骰子模式

1. 點擊「線上」按鈕進入線上對戰模式
2. 選擇「創建房間」或「加入房間」
3. 在遊戲模式選擇對話框中勾選「🎲 骰子」
4. 可以與其他模式同時選擇（如「💣 踩地雷」）
5. 開始遊戲

### 遊戲過程

1. **回合開始**：自動投擲骰子，顯示三種棋子類型
2. **選擇棋子**：點擊符合骰子結果的棋子
3. **限制提示**：如果選擇不符合的棋子，會顯示提示訊息
4. **執行移動**：移動選中的棋子到有效位置
5. **下一回合**：對手的回合開始，自動投新的骰子

### 特殊情況處理

- **無法移動**：如果三種棋子都無法移動，自動跳過回合
- **國王安全**：國王不受骰子限制，可以隨時移動（避免因無法移動而送死）
- **將軍逃脫**：即使骰子中沒有國王，也可以移動國王逃離將軍

## 網路協議

### DiceRoll 訊息格式

#### 發送骰子結果
```json
{
  "action": "diceRoll",
  "room": "房間號碼",
  "dice": [1, 2, 3]  // PieceType 枚舉值陣列
}
```

#### 接收骰子結果
```json
{
  "action": "diceRoll",
  "room": "房間號碼",
  "dice": [1, 2, 3]
}
```

### PieceType 枚舉值
```cpp
enum class PieceType {
    None = 0,
    Pawn = 1,      // 兵
    Rook = 2,      // 城堡
    Knight = 3,    // 騎士
    Bishop = 4,    // 主教
    Queen = 5,     // 皇后
    King = 6       // 國王
};
```

## 伺服器要求

伺服器需要處理 `diceRoll` 動作，並將骰子結果轉發給房間中的另一位玩家：

```javascript
// server.js 範例
if (data.action === 'diceRoll') {
    const room = rooms.get(data.room);
    if (room) {
        // 轉發給房間中的其他玩家
        const otherPlayer = room.players.find(p => p !== ws);
        if (otherPlayer) {
            otherPlayer.send(JSON.stringify({
                action: 'diceRoll',
                room: data.room,
                dice: data.dice
            }));
        }
    }
}
```

## 策略建議

### 玩家策略
1. **靈活布局**：保持各種棋子的機動性
2. **風險管理**：避免過度依賴單一棋子類型
3. **控制中心**：保持多種棋子在中心位置
4. **備用計劃**：為不同骰子結果準備多個走法

### 開局建議
- 快速發展多種棋子
- 避免棋子過於集中
- 保持中心控制權
- 預留多個移動選項

## 已知限制

1. **伺服器更新**：需要伺服器支援 `diceRoll` 動作
2. **單人模式**：目前僅支援線上多人模式
3. **國王特殊規則**：國王不受骰子限制（設計決策）

## 未來改進

可能的改進方向：
1. 添加骰子動畫效果
2. 支援自訂骰子規則
3. 添加骰子歷史記錄
4. 提供骰子統計資訊
5. 支援單人模式對電腦

## 技術細節

### 骰子投擲演算法
```cpp
void ChessBoard::rollDice() {
    // 可選棋子類型（排除國王）
    std::vector<PieceType> availablePieces = {
        Pawn, Knight, Bishop, Rook, Queen
    };
    
    // 隨機打亂
    std::shuffle(availablePieces.begin(), availablePieces.end(), rng);
    
    // 選擇前三個
    for (int i = 0; i < 3; ++i) {
        m_diceRoll.push_back(availablePieces[i]);
    }
}
```

### 合法移動檢查
```cpp
bool ChessBoard::hasAnyValidMovesWithDice(PieceColor color) const {
    // 遍歷所有棋子
    for (每個棋子) {
        if (棋子顏色 == color && canMovePieceType(棋子類型)) {
            // 檢查是否有合法移動
            if (有合法移動) return true;
        }
    }
    return false;
}
```

## 測試建議

### 功能測試
- [ ] 骰子正確顯示三種不同棋子
- [ ] 只能選擇骰子中的棋子類型
- [ ] 無法移動時自動跳過回合
- [ ] 網路同步正常工作
- [ ] 與其他模式相容（地雷等）

### 邊界測試
- [ ] 所有棋子都被吃掉的情況
- [ ] 只剩國王的情況
- [ ] 被將軍時的處理
- [ ] 連續多次跳過回合

### 網路測試
- [ ] 雙方看到相同的骰子結果
- [ ] 斷線重連後狀態正確
- [ ] 延遲情況下的同步

## 版本歷史

- v1.0 (2025-12-29): 初始實現
  - 基本骰子功能
  - 網路同步
  - UI 顯示
  - 回合跳過邏輯
