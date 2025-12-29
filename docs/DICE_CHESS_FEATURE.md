# 骰子西洋棋模式 (Dice Chess Mode)

## 功能概述

骰子西洋棋模式是一種創新的遊戲變體，為傳統西洋棋增添了隨機性和策略深度。在此模式下，玩家在每回合開始前必須投擲三個骰子，**必須依序從左到右使用每個骰子**，使用完全部三個骰子後才換到下一位玩家。

## 核心特性

### 1. 骰子機制
- **三個骰子**：每回合投擲三個虛擬骰子
- **可重複棋子**：每個骰子獨立隨機選擇，**三個骰子可以是相同的棋子類型**
- **可選棋子**：兵、騎士、主教、城堡、皇后（國王不受限制，可以隨時移動）
- **順序限制**：玩家**必須依序從左到右使用骰子**（第1個→第2個→第3個）
- **視覺回饋**：
  - 🟠 **橘色**：未使用的骰子
  - ⚫ **灰色**：已使用的骰子

### 2. 回合規則
- 每回合開始前自動投骰子（三個骰子都重新投擲）
- 玩家**必須依序使用**：
  1. 先使用第一個（最左邊）骰子
  2. 使用後該骰子變灰，才能使用第二個骰子
  3. 第二個使用後變灰，才能使用第三個骰子
  4. 三個骰子**全部使用完畢**後，才換到下一位玩家
- 如果當前骰子顯示的棋子無法移動：
  - 顯示提示訊息，說明當前應使用哪個骰子
  - 該骰子跳過，自動進入下一個骰子
  - 如果三個骰子都無法使用，則換到下一位玩家

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
std::vector<PieceType> m_diceRoll;      // 當前骰子結果（三個棋子類型）
int m_currentDiceIndex;                 // 當前應使用的骰子索引（0-2）
```

#### 新增方法
```cpp
void enableDiceMode(bool enable);                       // 啟用/停用骰子模式
bool isDiceModeEnabled() const;                         // 檢查骰子模式是否啟用
void rollDice();                                        // 投擲骰子（可產生重複）
void setDiceRoll(const std::vector<PieceType>&);       // 設置骰子結果（網路同步用）
const std::vector<PieceType>& getDiceRoll() const;     // 獲取骰子結果
int getCurrentDiceIndex() const;                        // 獲取當前骰子索引
void markDiceAsUsed();                                  // 標記當前骰子為已使用
void resetDiceUsage();                                  // 重置骰子使用狀態
bool canMovePieceType(PieceType type) const;            // 檢查當前骰子是否匹配
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
房主選擇「骰子」模式 → 遊戲開始 → 啟用骰子模式 → 為白方投第一次骰子（三個）
```

### 2. 每回合流程（順序使用骰子）
```
投擲3個骰子（可能重複） → 顯示橘色骰子 → 
    玩家使用第1個骰子：
        選擇符合的棋子 → 執行移動 → 第1個骰子變灰 →
    玩家使用第2個骰子：
        選擇符合的棋子 → 執行移動 → 第2個骰子變灰 →
    玩家使用第3個骰子：
        選擇符合的棋子 → 執行移動 → 第3個骰子變灰 →
    所有骰子使用完畢 → 切換到下一位玩家 → 重新投3個骰子
```

### 3. 特殊情況處理
```
如果當前骰子無法使用（無合法移動）：
    自動跳過該骰子 → 進入下一個骰子
如果三個骰子都無法使用：
    自動跳過回合 → 切換到下一位玩家
```

### 4. 網路同步
```
玩家A投骰子 → 發送給伺服器 → 伺服器轉發給玩家B → 雙方顯示相同結果
玩家A使用骰子1 → 骰子1變灰 → 發送移動 → 玩家B更新顯示
玩家A使用骰子2 → 骰子2變灰 → 發送移動 → 玩家B更新顯示
玩家A使用骰子3 → 骰子3變灰 → 發送移動 → 切換到玩家B
```

## UI 設計

### 骰子顯示面板
- **位置**：右側時間面板下方
- **內容**：
  - 標題：「🎲 骰子結果」
  - 三個骰子結果卡片（從左到右）
  - 每張卡片顯示：
    - 棋子符號（Unicode）
    - 棋子名稱（中文）
- **顏色變化**：
  - 🟠 **橘色**（#FF8C00背景，#FFA500邊框）：未使用的骰子
  - ⚫ **灰色**（#505050背景，#808080邊框）：已使用的骰子
- **樣式**：
  - 現代科技風格
  - 動態顏色變化反映使用狀態
  - 深色背景

### 骰子使用順序視覺化
```
初始狀態：
[🟠 騎士] [🟠 兵] [🟠 騎士]  ← 全部橘色

使用第1個後：
[⚫ 騎士] [🟠 兵] [🟠 騎士]  ← 第1個變灰

使用第2個後：
[⚫ 騎士] [⚫ 兵] [🟠 騎士]  ← 第2個變灰

使用第3個後：
[⚫ 騎士] [⚫ 兵] [⚫ 騎士]  ← 全部變灰，回合結束
```

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

1. **回合開始**：自動投擲三個骰子（可能重複），全部顯示為橘色
2. **使用第1個骰子**：
   - 必須移動與第1個骰子相符的棋子
   - 如果選擇錯誤，會提示「請按順序使用骰子！當前骰子為：XXX」
   - 移動後，第1個骰子變灰
3. **使用第2個骰子**：
   - 第1個骰子已使用（灰色），現在可以使用第2個
   - 移動後，第2個骰子變灰
4. **使用第3個骰子**：
   - 前2個骰子已使用（灰色），現在可以使用第3個
   - 移動後，第3個骰子變灰
5. **回合結束**：三個骰子全部使用完畢（全部灰色），自動切換到對手
6. **對手回合**：對手重新投擲三個新骰子，重複上述流程

### 特殊情況處理

- **當前骰子無法移動**：自動跳過該骰子，進入下一個骰子
- **所有骰子都無法移動**：自動跳過整個回合，切換到對手
- **國王安全**：國王不受骰子限制，可以隨時移動（避免因無法移動而送死）
- **將軍逃脫**：即使骰子中沒有國王，也可以移動國王逃離將軍
- **重複骰子**：三個骰子可能是相同的棋子（例如：[兵][兵][兵]），需要連續移動三次該棋子

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
1. **靈活布局**：保持各種棋子的機動性，因為骰子可能重複
2. **風險管理**：由於必須順序使用骰子，提前規劃三步移動
3. **控制中心**：保持多種棋子在中心位置，增加可用選項
4. **連續移動**：如果投到三個相同棋子，需要能連續移動該棋子三次
5. **預測對手**：觀察對手的骰子使用情況，預測其剩餘移動

### 開局建議
- 快速發展多種棋子
- 避免棋子過於集中
- 保持中心控制權
- 預留每種棋子的移動選項
- 準備好連續移動同一類型棋子的計畫

### 特殊技巧
- **骰子管理**：善用三個骰子的順序，優先使用最有利的移動
- **棄子策略**：如果第1個骰子不理想，可以快速移動以進入下一個
- **連擊機會**：如果投到三個相同的強力棋子（如皇后），可以發動連續進攻

## 已知限制

1. **伺服器更新**：需要伺服器支援 `diceRoll` 動作
2. **單人模式**：目前僅支援線上多人模式
3. **國王特殊規則**：國王不受骰子限制（設計決策）
4. **順序限制**：必須嚴格按照左到右順序使用骰子

## 未來改進

可能的改進方向：
1. 添加骰子滾動動畫效果
2. 支援自訂骰子規則（如允許跳過骰子）
3. 添加骰子歷史記錄顯示
4. 提供骰子統計資訊（各棋子出現頻率）
5. 支援單人模式對電腦
6. 添加「重投」特殊能力（限制使用次數）

## 技術細節

### 骰子投擲演算法（允許重複）
```cpp
void ChessBoard::rollDice() {
    m_diceRoll.clear();
    m_currentDiceIndex = 0;  // 重置索引
    
    // 可選棋子類型（排除國王）
    std::vector<PieceType> availablePieces = {
        Pawn, Knight, Bishop, Rook, Queen
    };
    
    QRandomGenerator *rng = QRandomGenerator::global();
    
    // 投擲三個骰子，每個獨立隨機（可能重複）
    for (int i = 0; i < 3; ++i) {
        int randomIndex = rng->bounded(availablePieces.size());
        m_diceRoll.push_back(availablePieces[randomIndex]);
    }
}
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

### 順序檢查演算法
```cpp
bool ChessBoard::canMovePieceType(PieceType type) const {
    if (!m_diceModeEnabled) return true;
    
    // 國王不受限制
    if (type == PieceType::King) return true;
    
    // 檢查當前骰子（從左到右順序）
    if (m_currentDiceIndex >= 0 && m_currentDiceIndex < 3) {
        return m_diceRoll[m_currentDiceIndex] == type;
    }
    
    return false;  // 所有骰子已使用完畢
}

void ChessBoard::markDiceAsUsed() {
    if (m_currentDiceIndex < 3) {
        m_currentDiceIndex++;  // 移動到下一個骰子
    }
}
```

### 視覺更新邏輯
```cpp
void Qt_Chess::updateDiceDisplay() {
    int currentIndex = m_chessBoard.getCurrentDiceIndex();
    
    // 更新每個骰子的顏色
    for (int i = 0; i < 3; i++) {
        if (currentIndex > i) {
            // 已使用：灰色
            diceLabel[i]->setStyleSheet("background: #505050; border: 2px solid #808080;");
        } else {
            // 未使用：橘色
            diceLabel[i]->setStyleSheet("background: #FF8C00; border: 2px solid #FFA500;");
        }
    }
}
```

## 測試建議

### 功能測試
- [ ] 骰子可以顯示重複的棋子類型
- [ ] 必須按順序使用骰子（左→右）
- [ ] 使用後骰子正確變灰
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
