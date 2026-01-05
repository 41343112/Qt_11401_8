# 骰子模式程式碼說明 (Dice Mode Code Documentation)

## 目錄 (Table of Contents)

1. [概述 (Overview)](#概述-overview)
2. [核心機制 (Core Mechanics)](#核心機制-core-mechanics)
3. [客戶端實現 (Client Implementation)](#客戶端實現-client-implementation)
4. [伺服器端實現 (Server Implementation)](#伺服器端實現-server-implementation)
5. [將軍中斷規則 (Check Interruption Rule)](#將軍中斷規則-check-interruption-rule)
6. [同步機制 (Synchronization Mechanism)](#同步機制-synchronization-mechanism)
7. [UI 顯示 (UI Display)](#ui-顯示-ui-display)
8. [完整流程 (Complete Flow)](#完整流程-complete-flow)

---

## 概述 (Overview)

### 中文說明

骰子模式是 Qt_Chess 的特殊遊戲變體，玩家每回合需要擲出 3 個骰子，每個骰子代表一種棋子類型，玩家必須依序移動這 3 種類型的棋子各一次。

**核心規則**：
1. 每回合開始時擲 3 個骰子
2. 每個骰子隨機選擇一種可移動的棋子類型
3. 玩家必須移動這 3 種類型的棋子各一次
4. **將軍中斷規則**：當玩家將對手將軍（但不將死）時，對手必須先解除將軍，然後原玩家繼續完成剩餘移動

### English Description

Dice mode is a special game variant of Qt_Chess where players roll 3 dice each turn. Each die represents a piece type, and players must move each of these 3 piece types once.

**Core Rules**:
1. Roll 3 dice at the start of each turn
2. Each die randomly selects a movable piece type
3. Players must move each of the 3 piece types once
4. **Check Interruption Rule**: When a player puts the opponent in check (but not checkmate), the opponent must first escape check, then the original player continues their remaining moves

---

## 核心機制 (Core Mechanics)

### 資料結構 (Data Structures)

#### 客戶端狀態 (Client State)

```cpp
// 在 Qt_Chess 類別中
class Qt_Chess : public QMainWindow {
private:
    // 骰子模式狀態
    bool m_diceMode;                                // 是否啟用骰子模式
    std::vector<PieceType> m_rolledPieceTypes;      // 骰出的棋子類型
    std::vector<int> m_rolledPieceTypeCounts;       // 每種類型的剩餘次數
    int m_diceMovesRemaining;                        // 剩餘移動次數（0-3）
    
    // 將軍中斷狀態
    bool m_diceCheckInterrupted;                     // 是否因將軍而中斷
    PieceColor m_diceInterruptedPlayer;              // 被中斷的玩家
    std::vector<PieceType> m_diceSavedPieceTypes;    // 保存的骰子類型
    std::vector<int> m_diceSavedPieceTypeCounts;     // 保存的剩餘次數
    int m_diceSavedMovesRemaining;                   // 保存的剩餘移動次數
    
    // UI 元件
    QLabel* m_diceLabel;                             // 骰子顯示標籤
};
```

#### 伺服器端狀態 (Server State)

```javascript
// 在 server.js 中
const diceRolls = {};
// diceRolls[roomId] = {
//   currentPlayer: string,           // "White" 或 "Black"
//   movesRemaining: number,          // 剩餘移動次數 (0-3)
//   interruptedPlayer: string,       // 被中斷的玩家（可選）
//   savedMovesRemaining: number      // 保存的剩餘移動次數（可選）
// }
```

### 骰子生成算法 (Dice Generation Algorithm)

```cpp
void Qt_Chess::rollDice() {
    // 1. 獲取所有可移動的棋子類型
    std::vector<PieceType> movablePieces = getMovablePieceTypes(m_myColor);
    
    if (movablePieces.empty()) {
        QMessageBox::warning(this, "骰子模式", "沒有可移動的棋子！");
        return;
    }
    
    // 2. 清空舊的骰子結果
    m_rolledPieceTypes.clear();
    m_rolledPieceTypeCounts.clear();
    
    // 3. 擲 3 個骰子
    for (int i = 0; i < 3; i++) {
        // 隨機選擇一種棋子類型
        int index = QRandomGenerator::global()->bounded(movablePieces.size());
        PieceType type = movablePieces[index];
        
        // 檢查是否已經有這種類型
        auto it = std::find(m_rolledPieceTypes.begin(), 
                           m_rolledPieceTypes.end(), type);
        
        if (it != m_rolledPieceTypes.end()) {
            // 已有此類型，增加計數
            int idx = std::distance(m_rolledPieceTypes.begin(), it);
            m_rolledPieceTypeCounts[idx]++;
        } else {
            // 新類型，添加到列表
            m_rolledPieceTypes.push_back(type);
            m_rolledPieceTypeCounts.push_back(1);
        }
    }
    
    // 4. 設定剩餘移動次數
    m_diceMovesRemaining = 3;
    
    // 5. 更新 UI 顯示
    updateDiceDisplay();
}
```

### 獲取可移動棋子類型 (Get Movable Piece Types)

```cpp
std::vector<PieceType> Qt_Chess::getMovablePieceTypes(PieceColor color) {
    std::set<PieceType> uniqueTypes;
    
    // 遍歷棋盤上所有該顏色的棋子
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            const ChessPiece& piece = m_chessBoard.getPiece(row, col);
            
            if (piece.getColor() == color && piece.getType() != PieceType::None) {
                // 檢查這個棋子是否有合法移動
                QPoint from(col, row);
                
                for (int toRow = 0; toRow < 8; toRow++) {
                    for (int toCol = 0; toCol < 8; toCol++) {
                        QPoint to(toCol, toRow);
                        if (m_chessBoard.isValidMove(from, to)) {
                            // 這個類型的棋子可以移動
                            uniqueTypes.insert(piece.getType());
                            goto next_piece;  // 跳到下一個棋子
                        }
                    }
                }
                next_piece:;
            }
        }
    }
    
    // 轉換為 vector
    return std::vector<PieceType>(uniqueTypes.begin(), uniqueTypes.end());
}
```

---

## 客戶端實現 (Client Implementation)

### 初始化骰子模式 (Initialize Dice Mode)

```cpp
void Qt_Chess::initializeDiceMode() {
    m_diceMode = true;
    m_diceMovesRemaining = 0;
    m_rolledPieceTypes.clear();
    m_rolledPieceTypeCounts.clear();
    
    // 重置中斷狀態
    m_diceCheckInterrupted = false;
    m_diceInterruptedPlayer = PieceColor::None;
    m_diceSavedPieceTypes.clear();
    m_diceSavedPieceTypeCounts.clear();
    m_diceSavedMovesRemaining = 0;
    
    // 如果是白方先手，立即擲骰子
    if (m_myColor == PieceColor::White) {
        rollDice();
    }
}
```

### 回合開始處理 (Turn Start Handling)

```cpp
void Qt_Chess::onTurnStart() {
    if (!m_diceMode) {
        return;  // 非骰子模式
    }
    
    // 檢查是否需要擲新骰子
    if (m_diceMovesRemaining == 0 && !m_diceCheckInterrupted) {
        // 線上模式：向伺服器請求骰子
        if (m_networkManager && m_networkManager->isConnected()) {
            requestDiceFromServer();
        } else {
            // 本地模式：自己擲骰子
            rollDice();
        }
    }
}
```

### 線上模式請求骰子 (Request Dice from Server)

```cpp
void Qt_Chess::requestDiceFromServer() {
    if (!m_networkManager || !m_networkManager->isConnected()) {
        return;
    }
    
    // 獲取可移動的棋子類型數量
    std::vector<PieceType> movablePieces = getMovablePieceTypes(m_myColor);
    int numMovablePieces = movablePieces.size();
    
    // 發送請求到伺服器
    QJsonObject request;
    request["action"] = "requestDice";
    request["room"] = m_roomNumber;
    request["numMovablePieces"] = numMovablePieces;
    
    m_networkManager->sendMessage(request);
}
```

### 接收骰子結果 (Receive Dice Results)

```cpp
void Qt_Chess::onDiceRolled(const QJsonArray& rolls, const QString& currentPlayer) {
    // 檢查是否是我的回合
    bool isMyTurn = (currentPlayer == "White" && m_myColor == PieceColor::White) ||
                    (currentPlayer == "Black" && m_myColor == PieceColor::Black);
    
    if (!isMyTurn) {
        return;  // 不是我的回合，忽略
    }
    
    // 獲取可移動的棋子類型
    std::vector<PieceType> movablePieces = getMovablePieceTypes(m_myColor);
    
    // 解析骰子結果
    m_rolledPieceTypes.clear();
    m_rolledPieceTypeCounts.clear();
    
    for (const QJsonValue& value : rolls) {
        int index = value.toInt();
        if (index >= 0 && index < movablePieces.size()) {
            PieceType type = movablePieces[index];
            
            // 檢查是否已有此類型
            auto it = std::find(m_rolledPieceTypes.begin(), 
                               m_rolledPieceTypes.end(), type);
            
            if (it != m_rolledPieceTypes.end()) {
                int idx = std::distance(m_rolledPieceTypes.begin(), it);
                m_rolledPieceTypeCounts[idx]++;
            } else {
                m_rolledPieceTypes.push_back(type);
                m_rolledPieceTypeCounts.push_back(1);
            }
        }
    }
    
    m_diceMovesRemaining = 3;
    updateDiceDisplay();
}
```

### 移動驗證 (Move Validation)

```cpp
bool Qt_Chess::canMovePieceInDiceMode(const ChessPiece& piece) {
    if (!m_diceMode) {
        return true;  // 非骰子模式，允許所有移動
    }
    
    if (m_diceMovesRemaining == 0) {
        return false;  // 沒有剩餘移動
    }
    
    PieceType type = piece.getType();
    
    // 檢查是否在骰子列表中
    for (size_t i = 0; i < m_rolledPieceTypes.size(); i++) {
        if (m_rolledPieceTypes[i] == type && m_rolledPieceTypeCounts[i] > 0) {
            return true;  // 可以移動
        }
    }
    
    return false;  // 不在骰子列表中
}
```

### 移動後處理 (Post-Move Processing)

```cpp
void Qt_Chess::processDiceMoveComplete(PieceType movedPieceType) {
    // 1. 扣除這個類型的計數
    for (size_t i = 0; i < m_rolledPieceTypes.size(); i++) {
        if (m_rolledPieceTypes[i] == movedPieceType && 
            m_rolledPieceTypeCounts[i] > 0) {
            m_rolledPieceTypeCounts[i]--;
            break;
        }
    }
    
    // 2. 扣除剩餘移動次數
    m_diceMovesRemaining--;
    
    // 3. 更新 UI
    updateDiceDisplay();
    
    // 4. 檢查是否還有剩餘移動
    if (m_diceMovesRemaining == 0) {
        // 所有骰子移動完成
        onAllDiceMovesComplete();
    }
}
```

---

## 伺服器端實現 (Server Implementation)

### 初始化骰子狀態 (Initialize Dice State)

```javascript
// 在 startGame 處理中
if(msg.action === "startGame"){
    const roomId = msg.room;
    if(rooms[roomId] && rooms[roomId].length === 2){
        // ... 其他初始化 ...
        
        // 如果啟用骰子模式
        if(msg.gameModes && msg.gameModes['骰子']) {
            diceRolls[roomId] = {
                currentPlayer: "White",  // 白方先手
                movesRemaining: 3
            };
            console.log('[Server] Dice mode initialized for room', roomId);
        }
        
        // ... 廣播遊戲開始 ...
    }
}
```

### 處理骰子請求 (Handle Dice Request)

```javascript
if(msg.action === "requestDice"){
    const roomId = msg.room;
    
    if(rooms[roomId] && diceRolls[roomId]){
        const numMovablePieces = msg.numMovablePieces || 1;
        
        // 生成3個隨機索引（可重複）
        const rolls = [];
        for(let i = 0; i < 3; i++){
            rolls.push(Math.floor(Math.random() * numMovablePieces));
        }
        
        console.log('[Server] Generated rolls:', rolls, 
                    'for player:', diceRolls[roomId].currentPlayer);
        
        // 重置剩餘移動次數
        diceRolls[roomId].movesRemaining = 3;
        
        // 廣播給房間內所有玩家
        const diceMessage = {
            action: "diceRolled",
            room: roomId,
            rolls: rolls,
            currentPlayer: diceRolls[roomId].currentPlayer
        };
        
        rooms[roomId].forEach(client => {
            if(client.readyState === WebSocket.OPEN){
                client.send(JSON.stringify(diceMessage));
            }
        });
    }
}
```

### 處理移動與骰子狀態 (Handle Move with Dice State)

```javascript
if(msg.action === "move"){
    const roomId = msg.room;
    
    // ... 驗證和計時器邏輯 ...
    
    // 骰子模式邏輯
    let shouldSwitchPlayer = true;
    
    if(diceRolls[roomId]) {
        console.log('[Server] Dice mode: movesRemaining before:', 
                    diceRolls[roomId].movesRemaining);
        
        // 檢查是否有將軍中斷
        if(msg.diceCheckInterruption && msg.savedDiceMoves > 0) {
            console.log('[Server] Dice check interruption! Saved moves:', 
                        msg.savedDiceMoves);
            
            // 保存被中斷的玩家和剩餘移動
            diceRolls[roomId].interruptedPlayer = playerWhoJustMoved;
            diceRolls[roomId].savedMovesRemaining = msg.savedDiceMoves;
            diceRolls[roomId].movesRemaining = 0;
            
            shouldSwitchPlayer = true;  // 強制切換
        } else {
            // 正常扣除移動次數
            if(diceRolls[roomId].movesRemaining > 0) {
                diceRolls[roomId].movesRemaining--;
            }
            
            // 檢查是否還有剩餘移動
            if(diceRolls[roomId].movesRemaining > 0) {
                shouldSwitchPlayer = false;  // 不切換玩家
            }
        }
    }
    
    // 根據 shouldSwitchPlayer 決定是否切換玩家
    if(shouldSwitchPlayer) {
        timer.currentPlayer = (timer.currentPlayer === "White") ? "Black" : "White";
    }
    
    // ... 廣播移動 ...
}
```

---

## 將軍中斷規則 (Check Interruption Rule)

### 檢測將軍中斷 (Detect Check Interruption)

```cpp
void Qt_Chess::checkForDiceCheckInterruption() {
    if (!m_diceMode || m_diceMovesRemaining == 0) {
        return;  // 不在骰子模式或已完成所有移動
    }
    
    // 檢查對手是否被將軍
    PieceColor opponentColor = (m_myColor == PieceColor::White) ? 
                                PieceColor::Black : PieceColor::White;
    bool opponentInCheck = m_chessBoard.isInCheck(opponentColor);
    bool opponentInCheckmate = m_chessBoard.isCheckmate(opponentColor);
    
    // 如果將軍但不將死，且還有剩餘移動
    if (opponentInCheck && !opponentInCheckmate && m_diceMovesRemaining > 0) {
        // 保存當前狀態
        m_diceCheckInterrupted = true;
        m_diceInterruptedPlayer = m_myColor;
        m_diceSavedPieceTypes = m_rolledPieceTypes;
        m_diceSavedPieceTypeCounts = m_rolledPieceTypeCounts;
        m_diceSavedMovesRemaining = m_diceMovesRemaining;
        
        // 清空當前骰子狀態
        m_rolledPieceTypes.clear();
        m_rolledPieceTypeCounts.clear();
        m_diceMovesRemaining = 0;
        
        // 更新 UI
        updateDiceDisplay();
        
        // 顯示提示
        m_statusLabel->setText("將軍！對手必須先解除將軍");
        
        // 通知伺服器（線上模式）
        if (m_networkManager && m_networkManager->isConnected()) {
            QJsonObject msg;
            msg["action"] = "diceCheckInterruption";
            msg["room"] = m_roomNumber;
            msg["savedMovesRemaining"] = m_diceSavedMovesRemaining;
            m_networkManager->sendMessage(msg);
        }
    }
}
```

### 恢復中斷的回合 (Restore Interrupted Turn)

```cpp
void Qt_Chess::checkForDiceCheckRestoration() {
    if (!m_diceCheckInterrupted) {
        return;  // 沒有中斷
    }
    
    // 檢查我是否是被中斷的玩家
    if (m_diceInterruptedPlayer != m_myColor) {
        return;  // 不是我被中斷
    }
    
    // 檢查是否還在被將軍狀態
    bool stillInCheck = m_chessBoard.isInCheck(m_myColor);
    
    if (!stillInCheck) {
        // 將軍已解除，恢復骰子狀態
        m_rolledPieceTypes = m_diceSavedPieceTypes;
        m_rolledPieceTypeCounts = m_diceSavedPieceTypeCounts;
        m_diceMovesRemaining = m_diceSavedMovesRemaining;
        
        // 清除中斷標記
        m_diceCheckInterrupted = false;
        m_diceInterruptedPlayer = PieceColor::None;
        m_diceSavedPieceTypes.clear();
        m_diceSavedPieceTypeCounts.clear();
        m_diceSavedMovesRemaining = 0;
        
        // 更新 UI
        updateDiceDisplay();
        
        // 切換回我的回合
        m_chessBoard.setCurrentPlayer(m_myColor);
        
        // 顯示提示
        m_statusLabel->setText("將軍已解除，繼續完成剩餘移動");
    }
}
```

### 伺服器端恢復邏輯 (Server-Side Restoration)

```javascript
// 在 move 處理中，所有骰子移動完成後
if(diceRolls[roomId] && diceRolls[roomId].movesRemaining <= 0) {
    // 檢查是否需要恢復中斷的玩家
    if(diceRolls[roomId].interruptedPlayer && 
       diceRolls[roomId].savedMovesRemaining > 0 &&
       playerWhoJustMoved !== diceRolls[roomId].interruptedPlayer &&
       !checkInterruptionOccurred) {
        
        // 恢復被中斷玩家的回合
        console.log('[Server] Restoring interrupted player:', 
                    diceRolls[roomId].interruptedPlayer);
        
        timer.currentPlayer = diceRolls[roomId].interruptedPlayer;
        diceRolls[roomId].currentPlayer = diceRolls[roomId].interruptedPlayer;
        diceRolls[roomId].movesRemaining = diceRolls[roomId].savedMovesRemaining;
        
        // 清除中斷狀態
        delete diceRolls[roomId].interruptedPlayer;
        delete diceRolls[roomId].savedMovesRemaining;
        
        console.log('[Server] Turn restored with', 
                    diceRolls[roomId].movesRemaining, 'moves remaining');
    }
}
```

---

## 同步機制 (Synchronization Mechanism)

### 客戶端-伺服器同步 (Client-Server Sync)

#### 1. 骰子請求同步

```cpp
// 客戶端
void Qt_Chess::onMyTurnStart() {
    if (m_diceMode && m_diceMovesRemaining == 0) {
        requestDiceFromServer();
    }
}
```

```javascript
// 伺服器
if(msg.action === "requestDice"){
    // 生成骰子並廣播給雙方
    const rolls = generateDiceRolls(msg.numMovablePieces);
    broadcastToRoom(roomId, {
        action: "diceRolled",
        rolls: rolls,
        currentPlayer: diceRolls[roomId].currentPlayer
    });
}
```

#### 2. 移動狀態同步

```cpp
// 客戶端發送移動
QJsonObject moveMsg;
moveMsg["action"] = "move";
moveMsg["room"] = m_roomNumber;
moveMsg["fromRow"] = from.y();
moveMsg["fromCol"] = from.x();
moveMsg["toRow"] = to.y();
moveMsg["toCol"] = to.x();

// 添加骰子狀態
if (m_diceMode) {
    moveMsg["diceMovesRemaining"] = m_diceMovesRemaining;
    
    // 如果有將軍中斷
    if (needsCheckInterruption) {
        moveMsg["diceCheckInterruption"] = true;
        moveMsg["savedDiceMoves"] = m_diceMovesRemaining;
    }
}

m_networkManager->sendMessage(moveMsg);
```

```javascript
// 伺服器接收並廣播
if(msg.action === "move"){
    // 更新骰子狀態
    if(diceRolls[roomId]) {
        if(msg.diceCheckInterruption) {
            // 處理中斷
            diceRolls[roomId].interruptedPlayer = currentPlayer;
            diceRolls[roomId].savedMovesRemaining = msg.savedDiceMoves;
        } else {
            // 正常扣除
            diceRolls[roomId].movesRemaining--;
        }
    }
    
    // 廣播移動和骰子狀態
    const broadcastMsg = {
        ...msg,
        diceState: {
            movesRemaining: diceRolls[roomId].movesRemaining,
            hasInterruption: !!diceRolls[roomId].interruptedPlayer
        }
    };
    
    broadcastToRoom(roomId, broadcastMsg);
}
```

---

## UI 顯示 (UI Display)

### 骰子顯示 (Dice Display)

```cpp
void Qt_Chess::updateDiceDisplay() {
    if (!m_diceLabel) {
        return;
    }
    
    if (!m_diceMode || m_diceMovesRemaining == 0) {
        m_diceLabel->setVisible(false);
        return;
    }
    
    // 構建顯示文字
    QString text = "🎲 骰子: ";
    
    for (size_t i = 0; i < m_rolledPieceTypes.size(); i++) {
        if (i > 0) text += ", ";
        
        // 棋子圖標
        text += getPieceIcon(m_rolledPieceTypes[i]);
        
        // 剩餘次數
        if (m_rolledPieceTypeCounts[i] > 1) {
            text += QString(" x%1").arg(m_rolledPieceTypeCounts[i]);
        }
    }
    
    text += QString(" | 剩餘: %1").arg(m_diceMovesRemaining);
    
    m_diceLabel->setText(text);
    m_diceLabel->setVisible(true);
}

QString Qt_Chess::getPieceIcon(PieceType type) {
    switch (type) {
        case PieceType::King:   return "♔";
        case PieceType::Queen:  return "♕";
        case PieceType::Rook:   return "♖";
        case PieceType::Bishop: return "♗";
        case PieceType::Knight: return "♘";
        case PieceType::Pawn:   return "♙";
        default:                return "?";
    }
}
```

### 狀態提示 (Status Messages)

```cpp
void Qt_Chess::updateDiceStatusMessage() {
    if (!m_diceMode) {
        return;
    }
    
    QString status;
    
    if (m_diceCheckInterrupted) {
        if (m_diceInterruptedPlayer == m_myColor) {
            status = "對手正在解除將軍...";
        } else {
            status = "你被將軍了！請先解除將軍";
        }
    } else if (m_diceMovesRemaining > 0) {
        status = QString("請移動骰子指定的棋子（剩餘 %1 次）")
                    .arg(m_diceMovesRemaining);
    } else if (m_chessBoard.getCurrentPlayer() == m_myColor) {
        status = "正在擲骰子...";
    } else {
        status = "對手回合";
    }
    
    m_statusLabel->setText(status);
}
```

### 高亮顯示 (Highlighting)

```cpp
void Qt_Chess::paintEvent(QPaintEvent* event) {
    // ... 繪製棋盤和棋子 ...
    
    // 在骰子模式下高亮可移動的棋子
    if (m_diceMode && m_diceMovesRemaining > 0 && 
        m_chessBoard.getCurrentPlayer() == m_myColor) {
        
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                const ChessPiece& piece = m_chessBoard.getPiece(row, col);
                
                if (piece.getColor() == m_myColor && 
                    canMovePieceInDiceMode(piece)) {
                    // 繪製金色邊框
                    QRect rect = getCellRect(row, col);
                    painter.setPen(QPen(QColor(255, 215, 0), 3));  // 金色
                    painter.drawRect(rect);
                }
            }
        }
    }
}
```

---

## 完整流程 (Complete Flow)

### 單回合完整流程 (Complete Turn Flow)

```
1. 回合開始
   ↓
2. 檢查骰子狀態
   - 如果 movesRemaining == 0: 擲新骰子
   - 如果 movesRemaining > 0: 使用現有骰子
   ↓
3. 玩家選擇並移動棋子
   - 驗證棋子類型是否在骰子列表中
   - 執行移動
   ↓
4. 移動後處理
   - 扣除骰子計數
   - movesRemaining--
   - 更新 UI
   ↓
5. 檢查特殊情況
   a. 將軍（非將死）且 movesRemaining > 0:
      → 保存狀態
      → 切換到對手
      → 對手解除將軍後恢復
   
   b. 將死:
      → 遊戲結束
   
   c. movesRemaining == 0:
      → 正常切換到對手
      → 對手擲新骰子
   
   d. movesRemaining > 0:
      → 繼續當前玩家回合
      → 移動下一個骰子指定的棋子
```

### 將軍中斷流程 (Check Interruption Flow)

```
玩家 A 回合（骰子: 3個）
   ↓
移動第 1 個棋子 ✓
   ↓
移動第 2 個棋子，將玩家 B 將軍（但不將死）
   ↓
保存狀態:
   - interruptedPlayer = A
   - savedMovesRemaining = 1
   - movesRemaining = 0
   ↓
切換到玩家 B
   ↓
玩家 B 移動解除將軍
   ↓
檢測到將軍已解除
   ↓
恢復玩家 A 的回合:
   - currentPlayer = A
   - movesRemaining = 1（從保存的狀態）
   ↓
玩家 A 繼續移動第 3 個棋子
   ↓
movesRemaining = 0，正常結束回合
   ↓
切換到玩家 B，開始新回合
```

---

## 測試場景 (Test Scenarios)

### 基本測試 (Basic Tests)

1. **正常三次移動**
   - 擲骰子得到 3 種不同棋子
   - 依序移動 3 個棋子
   - 確認回合正確切換

2. **重複棋子類型**
   - 擲骰子得到相同類型（例如：兵、兵、兵）
   - 移動同類型棋子 3 次
   - 確認計數正確扣除

3. **將軍中斷**
   - 第 2 次移動時將對手將軍
   - 確認回合切換到對手
   - 對手解除將軍後恢復
   - 確認剩餘移動次數正確

4. **將死結束**
   - 移動造成將死
   - 確認遊戲立即結束
   - 確認不會保存或恢復狀態

### 邊界測試 (Edge Cases)

1. **第一次移動就將軍**
   - 確認正確處理
   - 確認剩餘 2 次移動被保存

2. **最後一次移動將軍**
   - 確認回合切換
   - 確認不會保存狀態（movesRemaining = 0）

3. **連續將軍**
   - 第一次將軍，對手解除
   - 恢復後再次將軍
   - 確認多次中斷處理正確

---

## 相關文件 (Related Documentation)

- [伺服器文件](SERVER_DOCUMENTATION.md)
- [遊戲規則文件](GAME_RULES_DOCUMENTATION.md)
- [骰子模式將軍規則](DICE_MODE_CHECK_RULE.md)
- [線上對戰功能](ONLINE_MODE_FEATURE.md)
- [遊戲模式使用指南](GAME_MODES_USAGE.md)
