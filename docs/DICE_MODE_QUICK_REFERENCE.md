# 骰子模式快速參考手冊
# Dice Mode Quick Reference Guide

---

## 一、核心概念 Core Concepts

### 骰子模式是什麼？ What is Dice Mode?
一種特殊的西洋棋遊戲模式，透過擲骰子決定每回合可以移動的棋子類型。
A special chess game mode where dice rolling determines which piece types can be moved each turn.

### 基本規則 Basic Rules
- 每回合開始時擲出 3 個骰子 Roll 3 dice at turn start
- 每個骰子代表一種棋子類型 Each die represents a piece type
- 必須移動這 3 種類型的棋子各一次 Must move each of the 3 types once
- 王棋每回合最多骰出一次 King can only be rolled once per turn

---

## 二、關鍵資料結構 Key Data Structures

```cpp
// 骰子狀態 Dice State
std::vector<PieceType> m_rolledPieceTypes;      // 骰出的類型
std::vector<int> m_rolledPieceTypeCounts;       // 每種剩餘次數
int m_diceMovesRemaining;                        // 總剩餘次數

// 中斷狀態 Interruption State
bool m_diceCheckInterrupted;                     // 是否中斷
PieceColor m_diceInterruptedPlayer;              // 被中斷者
std::vector<PieceType> m_diceSavedPieceTypes;   // 備份的骰子
int m_diceSavedMovesRemaining;                   // 備份的次數
```

---

## 三、主要函數速查 Function Quick Reference

| 函數名稱 | 位置 | 功能 |
|---------|------|------|
| `rollDiceForTurn()` | qt_chess.cpp:8933 | 骰出棋子 Roll dice |
| `onDiceRolled()` | qt_chess.cpp:8974 | 處理骰子結果 Process results |
| `updateDiceDisplay()` | qt_chess.cpp:9152 | 更新顯示 Update display |
| `isPieceTypeInRolledList()` | qt_chess.cpp:9232 | 驗證類型 Validate type |
| `markPieceTypeAsMoved()` | qt_chess.cpp:9252 | 標記已用 Mark as used |
| `canPieceTypeMove()` | qt_chess.cpp:8917 | 檢查可否移動 Check movable |

---

## 四、流程圖 Flow Charts

### 骰子擲出流程 Dice Rolling Flow
```
rollDiceForTurn() 
    → requestDiceRoll() [網路請求 Network]
    → Server generates random
    → onDiceRolled() [處理結果 Process]
    → updateDiceDisplay() [更新UI Update UI]
```

### 移動驗證流程 Move Validation Flow
```
Player clicks piece
    → isPieceTypeInRolledList() [檢查 Check]
    → YES: movePiece() → markPieceTypeAsMoved()
    → NO: Reject move
```

### 將軍中斷流程 Check Interruption Flow
```
Move causes check
    → Save dice state [保存狀態]
    → Switch to opponent [切換對手]
    → Opponent escapes check [解除將軍]
    → Restore dice state [恢復狀態]
    → Continue interrupted turn [繼續回合]
```

---

## 五、顯示狀態 Display States

| 狀態 | 外觀 | 條件 |
|------|------|------|
| 可用 Available | 🟦 藍色 Blue | 剩餘次數 > 0 且可移動 |
| 不可用 Unavailable | ⬛ 灰色 Grey | 剩餘次數 = 0 或無法移動 |
| 空白 Empty | -- | 沒有骰出 |

### 灰階條件 Greyscale Conditions
```cpp
if (remainingMoves <= 0 || !canPieceTypeMove(type, color)) {
    // 顯示灰色 Show grey
}
```

---

## 六、網路訊息 Network Messages

### 請求骰子 Request Dice
```json
{
  "type": "requestDiceRoll",
  "room": "ROOM_ID",
  "numPieces": 6
}
```

### 發送移動 Send Move
```json
{
  "type": "move",
  "room": "ROOM_ID",
  "from": {"x": 4, "y": 1},
  "to": {"x": 4, "y": 3},
  "diceCheckInterruption": true,    // 中斷標記
  "savedDiceMoves": 2                // 保存的次數
}
```

### 骰子結果 Dice Result
```json
{
  "type": "diceRolled",
  "rolls": [2, 4, 0],               // 隨機索引
  "currentPlayer": "White"
}
```

---

## 七、常見場景程式碼 Common Scenarios Code

### 場景 1：檢查棋子是否可移動
```cpp
bool Qt_Chess::canPieceTypeMove(PieceType type, PieceColor color) const {
    std::vector<QPoint> movablePieces = getMovablePieces(color);
    for (const auto& pos : movablePieces) {
        const ChessPiece& piece = m_chessBoard.getPiece(pos.y(), pos.x());
        if (piece.getType() == type) {
            return true;  // 找到可移動的該類型棋子
        }
    }
    return false;  // 沒有可移動的該類型棋子
}
```

### 場景 2：標記棋子已移動
```cpp
void Qt_Chess::markPieceTypeAsMoved(PieceType type) {
    for (size_t i = 0; i < m_rolledPieceTypes.size(); ++i) {
        if (m_rolledPieceTypes[i] == type && 
            m_rolledPieceTypeCounts[i] > 0) {
            m_rolledPieceTypeCounts[i]--;  // 減少次數
            m_diceMovesRemaining--;         // 減少總數
            updateDiceDisplay();            // 更新顯示
            return;
        }
    }
}
```

### 場景 3：檢測將軍中斷
```cpp
// 移動後檢查
bool opponentInCheck = m_chessBoard.isInCheck(opponentColor);
bool opponentInCheckmate = m_chessBoard.isCheckmate(opponentColor);

if (opponentInCheck && !opponentInCheckmate && 
    m_diceMovesRemaining - 1 > 0) {
    // 發送帶中斷標記的移動
    m_networkManager->sendMove(from, to, prom, final, 
                               true,                    // 中斷
                               m_diceMovesRemaining-1); // 保存
}
```

### 場景 4：恢復中斷回合
```cpp
if (m_diceCheckInterrupted && 
    m_diceInterruptedPlayer == myColor) {
    bool stillInCheck = m_chessBoard.isInCheck(myColor);
    if (!stillInCheck) {
        // 恢復狀態
        m_rolledPieceTypes = m_diceSavedPieceTypes;
        m_diceMovesRemaining = m_diceSavedMovesRemaining;
        
        // 清除標記
        m_diceCheckInterrupted = false;
        
        // 切換回合
        m_chessBoard.setCurrentPlayer(myColor);
        updateDiceDisplay();
    }
}
```

---

## 八、除錯技巧 Debugging Tips

### 關鍵 Debug 點 Key Debug Points
```cpp
// 1. 骰子擲出
qDebug() << "[rollDiceForTurn] Rolling for" << currentPlayer;

// 2. 移動驗證
qDebug() << "[isPieceTypeInRolledList] Type:" << type 
         << "Result:" << result;

// 3. 狀態更新
qDebug() << "[markPieceTypeAsMoved] Remaining:" 
         << m_diceMovesRemaining;

// 4. 中斷檢測
qDebug() << "[Check Interruption] opponentInCheck:" << opponentInCheck
         << "remaining:" << m_diceMovesRemaining;
```

### 常見問題排查 Common Issues
1. **骰子沒有顯示** → 檢查 `m_diceModeEnabled` 和 `m_isOnlineGame`
2. **移動被拒絕** → 檢查 `isPieceTypeInRolledList()` 返回值
3. **灰階不正確** → 檢查 `canPieceTypeMove()` 邏輯
4. **中斷失敗** → 檢查將軍檢測條件和 `m_diceMovesRemaining`

---

## 九、效能考量 Performance Considerations

### canPieceTypeMove() 複雜度
- **最壞情況**: O(64) - 遍歷所有格子
- **優化**: 使用 `getMovablePieces()` 只檢查可移動棋子
- **調用頻率**: 每次 `updateDiceDisplay()` 時（每個骰子）

### 建議 Recommendations
- ✅ 使用早期返回 (early return)
- ✅ 快取可移動棋子列表
- ✅ 僅在狀態變化時更新顯示

---

## 十、測試檢查清單 Testing Checklist

### 基本功能 Basic Functions
- [ ] 骰子正確擲出（3個）
- [ ] 顯示面板正確更新
- [ ] 只能移動骰出的類型
- [ ] 移動後正確變灰
- [ ] 所有骰子用完後換邊

### 將軍中斷 Check Interruption
- [ ] 將軍時正確中斷
- [ ] 對手能夠應對
- [ ] 解除將軍後恢復
- [ ] 剩餘骰子正確恢復
- [ ] 將死時不中斷（遊戲結束）

### 動態顯示 Dynamic Display
- [ ] 棋子被吃後變灰
- [ ] 棋子被擋後變灰
- [ ] 棋子解除阻擋後變彩色
- [ ] 雙方都能看到骰子

### 網路同步 Network Sync
- [ ] 骰子結果雙方一致
- [ ] 移動狀態正確同步
- [ ] 中斷狀態正確傳遞
- [ ] 斷線重連後狀態正確

---

## 十一、快速故障排除 Quick Troubleshooting

| 問題 Problem | 可能原因 Cause | 解決方案 Solution |
|-------------|---------------|------------------|
| 骰子不顯示 | 模式未啟用 | 檢查 `m_diceModeEnabled` |
| 不能移動 | 類型不匹配 | 檢查 `isPieceTypeInRolledList()` |
| 灰階錯誤 | 檢查邏輯錯誤 | 驗證 `canPieceTypeMove()` |
| 中斷失敗 | 條件不符 | 檢查將軍檢測和剩餘次數 |
| 恢復失敗 | 狀態未保存 | 驗證保存邏輯 |
| 網路不同步 | 訊息丟失 | 檢查網路連接和訊息處理 |

---

## 十二、檔案位置總覽 File Locations Overview

```
Qt_11401_8/
├── src/
│   ├── qt_chess.h              // 骰子模式變數宣告 (279-294行)
│   ├── qt_chess.cpp            // 骰子模式實現 (8917-9317行)
│   ├── networkmanager.h        // 網路函數宣告
│   └── networkmanager.cpp      // 網路函數實現 (281-311行)
├── server.js                   // 伺服器邏輯 (191-213行)
└── docs/
    ├── 骰子模式程式介紹.md        // 完整簡報 (中文)
    ├── Dice_Mode_Code_Presentation_EN.md  // 完整簡報 (英文)
    ├── DICE_MODE_DOCUMENTATION_INDEX.md   // 文件索引
    ├── DICE_MODE_CHECK_RULE.md           // 將軍規則
    └── FEATURE_DYNAMIC_DICE_DISPLAY.md   // 動態顯示
```

---

## 十三、相關命令 Related Commands

### 建置專案 Build Project
```bash
qmake Qt_Chess.pro
make
./Qt_Chess
```

### 搜尋骰子相關程式碼 Search Dice Code
```bash
grep -r "m_rolledPieceTypes" src/
grep -r "rollDice" src/
grep -n "updateDiceDisplay" src/qt_chess.cpp
```

### 查看變更歷史 View Change History
```bash
git log --grep="dice" --oneline
git log --grep="骰子" --oneline
```

---

## 十四、參考資料 References

### 主要文件 Main Documents
- 📘 [骰子模式程式介紹.md](骰子模式程式介紹.md) - 完整技術簡報
- 📗 [Dice_Mode_Code_Presentation_EN.md](Dice_Mode_Code_Presentation_EN.md) - English version
- 📙 [DICE_MODE_CHECK_RULE.md](DICE_MODE_CHECK_RULE.md) - 將軍規則詳解
- 📕 [DICE_MODE_DOCUMENTATION_INDEX.md](DICE_MODE_DOCUMENTATION_INDEX.md) - 文件索引

### 相關功能 Related Features
- [FEATURE_DYNAMIC_DICE_DISPLAY.md](../FEATURE_DYNAMIC_DICE_DISPLAY.md)
- [IMPLEMENTATION_SUMMARY_DICE_CHECK.md](../IMPLEMENTATION_SUMMARY_DICE_CHECK.md)
- [BUGFIX_DICE_*.md](../) - 三個修復文件

---

## 十五、版本更新記錄 Version History

| 版本 | 日期 | 更新內容 |
|------|------|---------|
| 1.0 | 2026-01-04 | 初始版本 - 完整骰子模式實現 |
| | | - 骰子擲出功能 |
| | | - 移動驗證機制 |
| | | - 動態顯示更新 |
| | | - 將軍中斷規則 |
| | | - 網路同步機制 |

---

**快速參考手冊結束**  
**End of Quick Reference Guide**

需要更詳細的說明，請參閱完整的程式介紹文件。  
For more detailed explanation, please refer to the complete code presentation documents.

---

*最後更新 Last Updated: 2026-01-04*  
*文件版本 Document Version: 1.0*
