# 骰子模式將軍中斷規則 (Dice Mode Check Interruption Rule)

## 概述 (Overview)

本文檔描述骰子模式中新增的「將軍中斷」規則。當玩家在骰子回合中將對手的王將軍（但不是將死）時，必須先讓對手移動一步來解決將軍，然後被中斷的玩家才能繼續完成剩餘的骰子移動。

This document describes the new "check interruption" rule added to dice mode. When a player puts the opponent's king in check (but not checkmate) during their dice turn, the opponent must first move to escape check before the interrupted player can finish their remaining dice moves.

## 規則說明 (Rule Description)

### 中文說明

在骰子模式中：
1. 玩家擲出3個棋子類型的骰子
2. 玩家必須移動這3個類型的棋子各一次
3. **新規則**：如果玩家在移動過程中將對手的王將軍（但不是將死）：
   - 系統會保存當前玩家剩餘的骰子移動
   - 回合立即切換到對手
   - 對手必須先移動一步來解決將軍
   - 對手移動後，回合會自動切換回被中斷的玩家
   - 被中斷的玩家可以繼續完成剩餘的骰子移動

### English Description

In dice mode:
1. Players roll 3 dice representing piece types
2. Players must move each of these 3 piece types once
3. **New Rule**: If a player puts the opponent's king in check (but not checkmate) during their moves:
   - The system saves the current player's remaining dice moves
   - The turn immediately switches to the opponent
   - The opponent must move first to escape check
   - After the opponent moves, the turn automatically switches back to the interrupted player
   - The interrupted player can then finish their remaining dice moves

## 範例情境 (Example Scenarios)

### 情境 1：成功中斷和恢復

1. 白方擲骰：騎士、主教、城堡
2. 白方移動騎士 ✓
3. 白方移動主教，將黑方王將軍（但不是將死）
   - 系統保存：白方還需移動城堡
   - 回合切換到黑方
4. 黑方移動國王逃離將軍
5. 回合自動切換回白方
6. 白方繼續移動城堡完成回合

### Scenario 1: Successful Interruption and Restoration

1. White rolls dice: Knight, Bishop, Rook
2. White moves Knight ✓
3. White moves Bishop, putting Black king in check (not checkmate)
   - System saves: White still needs to move Rook
   - Turn switches to Black
4. Black moves King to escape check
5. Turn automatically switches back to White
6. White continues to move Rook to complete turn

### 情境 2：將死時不中斷

1. 白方擲骰：皇后、騎士、兵
2. 白方移動皇后，將黑方王將死
   - 遊戲立即結束
   - 不需要中斷或恢復

### Scenario 2: No Interruption on Checkmate

1. White rolls dice: Queen, Knight, Pawn
2. White moves Queen, putting Black king in checkmate
   - Game ends immediately
   - No interruption or restoration needed

## 技術實現 (Technical Implementation)

### 狀態變數 (State Variables)

系統使用以下變數追蹤中斷狀態：

```cpp
bool m_diceCheckInterrupted;         // 是否因將軍而中斷
PieceColor m_diceInterruptedPlayer;  // 被中斷的玩家顏色
std::vector<PieceType> m_diceSavedPieceTypes;      // 保存的骰子類型
std::vector<int> m_diceSavedPieceTypeCounts;       // 保存的剩餘次數
int m_diceSavedMovesRemaining;       // 保存的剩餘移動次數
```

### 中斷流程 (Interruption Flow)

1. **檢測將軍 (Check Detection)**
   ```cpp
   PieceColor opponentColor = m_chessBoard.getCurrentPlayer();
   bool opponentInCheck = m_chessBoard.isInCheck(opponentColor);
   bool opponentInCheckmate = m_chessBoard.isCheckmate(opponentColor);
   ```

2. **保存狀態 (Save State)**
   ```cpp
   if (opponentInCheck && !opponentInCheckmate && !allRolledPiecesMoved()) {
       m_diceCheckInterrupted = true;
       m_diceInterruptedPlayer = currentPlayer;
       m_diceSavedPieceTypes = m_rolledPieceTypes;
       m_diceSavedPieceTypeCounts = m_rolledPieceTypeCounts;
       m_diceSavedMovesRemaining = m_diceMovesRemaining;
       
       // 清空當前骰子狀態
       m_rolledPieceTypes.clear();
       m_rolledPieceTypeCounts.clear();
       m_diceMovesRemaining = 0;
   }
   ```

3. **恢復回合 (Restore Turn)**
   ```cpp
   if (m_diceCheckInterrupted && m_diceInterruptedPlayer == myColor) {
       bool stillInCheck = m_chessBoard.isInCheck(myColor);
       if (!stillInCheck) {
           // 恢復骰子狀態
           m_rolledPieceTypes = m_diceSavedPieceTypes;
           m_rolledPieceTypeCounts = m_diceSavedPieceTypeCounts;
           m_diceMovesRemaining = m_diceSavedMovesRemaining;
           
           // 清除中斷標記
           m_diceCheckInterrupted = false;
           m_diceInterruptedPlayer = PieceColor::None;
           
           // 切換回合
           m_chessBoard.setCurrentPlayer(myColor);
       }
   }
   ```

## 程式碼位置 (Code Locations)

### 修改的檔案 (Modified Files)

1. **src/qt_chess.h**
   - 新增中斷狀態變數（第 288-293 行）

2. **src/qt_chess.cpp**
   - 初始化變數（第 262-264 行）
   - 點擊移動的中斷邏輯（第 2567-2593 行）
   - 拖放移動的中斷邏輯（第 3702-3728 行）
   - 對手移動後的恢復邏輯（第 6267-6296 行）
   - 重置狀態邏輯（第 1824-1828 行，第 6715-6719 行）

## 測試建議 (Testing Recommendations)

### 需要測試的情境 (Scenarios to Test)

1. **基本中斷和恢復**
   - 在骰子回合中將對手將軍
   - 驗證對手可以移動
   - 驗證回合恢復到原玩家
   - 驗證剩餘骰子可以移動

2. **將死不中斷**
   - 在骰子回合中將對手將死
   - 驗證遊戲立即結束
   - 驗證不會保存或恢復狀態

3. **多次將軍**
   - 測試連續多次將軍的情況
   - 驗證每次都正確處理

4. **邊界情況**
   - 最後一個骰子移動時將軍
   - 第一個骰子移動時將軍
   - 對手無法解除將軍（應該是將死）

### 測試步驟 (Test Steps)

1. 啟動兩個 Qt_Chess 實例
2. 創建線上房間並啟用骰子模式
3. 開始遊戲
4. 故意製造將軍但不將死的局面
5. 觀察行為：
   - 被中斷的玩家的骰子顯示是否清空
   - 對手是否可以移動
   - 解除將軍後回合是否正確恢復
   - 骰子顯示是否正確恢復

## 注意事項 (Notes)

1. **僅在線上骰子模式啟用**：此規則僅在線上對戰的骰子模式中生效
2. **將死優先**：如果是將死，遊戲直接結束，不會觸發中斷
3. **狀態同步**：中斷和恢復狀態需要與對手保持同步
4. **UI 更新**：骰子顯示會在中斷和恢復時自動更新

## 未來改進 (Future Improvements)

1. **視覺提示**：添加明顯的 UI 提示顯示「將軍中斷」狀態
2. **通知對手**：明確告知對手為何突然輪到他們移動
3. **伺服器驗證**：在伺服器端也實現此規則的驗證
4. **回放支援**：確保遊戲回放功能正確顯示中斷和恢復

## 相關文件 (Related Documentation)

- [遊戲模式功能](GAME_MODES_FEATURE.md)
- [線上對戰功能](ONLINE_MODE_FEATURE.md)
- [遊戲模式使用指南](GAME_MODES_USAGE.md)
