# 伺服器程式碼說明 (Server Code Documentation)

## 目錄 (Table of Contents)

1. [概述 (Overview)](#概述-overview)
2. [伺服器架構 (Server Architecture)](#伺服器架構-server-architecture)
3. [核心功能 (Core Functions)](#核心功能-core-functions)
4. [訊息處理 (Message Handling)](#訊息處理-message-handling)
5. [計時器同步 (Timer Synchronization)](#計時器同步-timer-synchronization)
6. [骰子模式邏輯 (Dice Mode Logic)](#骰子模式邏輯-dice-mode-logic)
7. [安全機制 (Security Mechanisms)](#安全機制-security-mechanisms)
8. [程式碼結構 (Code Structure)](#程式碼結構-code-structure)

---

## 概述 (Overview)

### 中文說明

Qt_Chess 的線上對戰功能使用 Node.js WebSocket 伺服器實現。伺服器負責：
- 房間管理（創建、加入、離開）
- 訊息轉發（棋步、遊戲狀態）
- 計時器同步
- 骰子模式邏輯處理
- 連線狀態管理

伺服器檔案位於 `server.js`，使用 `ws` 模組實現 WebSocket 通訊。

### English Description

The Qt_Chess online gameplay uses a Node.js WebSocket server. The server is responsible for:
- Room management (create, join, leave)
- Message routing (moves, game state)
- Timer synchronization
- Dice mode logic handling
- Connection state management

The server file is located at `server.js` and uses the `ws` module for WebSocket communication.

---

## 伺服器架構 (Server Architecture)

### 技術棧 (Technology Stack)

```javascript
const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: process.env.PORT || 3000 });
```

- **WebSocket 伺服器**：基於 `ws` 模組
- **埠號**：使用環境變數 `PORT` 或預設 3000
- **部署平台**：Render (https://chess-server-mjg6.onrender.com)

### 資料結構 (Data Structures)

#### 1. 房間資料 (Room Data)

```javascript
const rooms = {};  // rooms[roomId] = [ws1, ws2, ...]
```

- **鍵 (Key)**：4位數字房號 (1000-9999)
- **值 (Value)**：WebSocket 連線陣列
- **重要**：陣列第一個元素 (index 0) 始終為房主

#### 2. 遊戲計時器 (Game Timers)

```javascript
const gameTimers = {};  
// gameTimers[roomId] = {
//   timeA: number,           // 房主的時間（毫秒）
//   timeB: number,           // 房客的時間（毫秒）
//   currentPlayer: string,   // "White" 或 "Black"
//   lastSwitchTime: number,  // 最後切換時間戳（毫秒）
//   whiteIsA: boolean,       // 白方是否為房主
//   incrementMs: number      // 每步加秒（毫秒）
// }
```

#### 3. 骰子模式狀態 (Dice Mode State)

```javascript
const diceRolls = {};
// diceRolls[roomId] = {
//   currentPlayer: string,        // "White" 或 "Black"
//   movesRemaining: number,       // 剩餘移動次數 (0-3)
//   interruptedPlayer: string,    // 被中斷的玩家（將軍中斷時）
//   savedMovesRemaining: number   // 保存的剩餘移動次數
// }
```

#### 4. 速率限制 (Rate Limiting)

```javascript
const rateLimits = new Map();
// ws -> { count: number, resetTime: number }
```

- **限制**：每秒最多 50 條訊息
- **目的**：防止濫用和 DoS 攻擊

---

## 核心功能 (Core Functions)

### 1. 生成房號 (Generate Room ID)

```javascript
function generateRoomId() {
    return Math.floor(1000 + Math.random() * 9000).toString();
}
```

**功能說明**：
- 生成 4 位數字房號 (1000-9999)
- 使用隨機數確保唯一性
- 創建房間時會檢查是否重複

### 2. 處理玩家離開 (Handle Player Leave)

```javascript
function handlePlayerLeaveRoom(ws, roomId) {
    if(!rooms[roomId] || !rooms[roomId].includes(ws)) {
        return;  // 玩家不在此房間
    }
    
    const wasHost = rooms[roomId][0] === ws;
    
    // 通知其他玩家
    rooms[roomId].forEach(client => {
        if(client !== ws && client.readyState === WebSocket.OPEN){
            client.send(JSON.stringify({ 
                action: "playerLeft", 
                room: roomId 
            }));
        }
    });
    
    // 移除玩家
    rooms[roomId] = rooms[roomId].filter(c => c !== ws);
    
    // 如果房主離開，提升新房主
    if(wasHost && rooms[roomId].length > 0){
        const newHost = rooms[roomId][0];
        if(newHost.readyState === WebSocket.OPEN){
            newHost.send(JSON.stringify({ 
                action: "promotedToHost", 
                room: roomId 
            }));
        }
    }
    
    // 如果房間空了，清理資源
    if(rooms[roomId].length === 0){
        delete rooms[roomId];
        delete gameTimers[roomId];
        delete diceRolls[roomId];
    }
}
```

**功能說明**：
- 處理玩家明確離開或斷線
- 通知房間內其他玩家
- 如果房主離開，自動提升新房主
- 清理空房間和相關資源

### 3. 速率限制檢查 (Check Rate Limit)

```javascript
function checkRateLimit(ws) {
    const now = Date.now();
    const limit = rateLimits.get(ws) || { count: 0, resetTime: now + 1000 };
    
    if(now > limit.resetTime) {
        limit.count = 1;
        limit.resetTime = now + 1000;
    } else {
        limit.count++;
        if(limit.count > 50) {
            return false;  // 超過限制
        }
    }
    
    rateLimits.set(ws, limit);
    return true;
}
```

**功能說明**：
- 每秒最多 50 條訊息
- 使用滑動視窗算法
- 超過限制時返回錯誤

---

## 訊息處理 (Message Handling)

### 訊息格式 (Message Format)

所有訊息使用 JSON 格式：

```json
{
    "action": "訊息類型",
    "room": "房間號",
    ...其他欄位
}
```

### 1. 創建房間 (Create Room)

**客戶端 → 伺服器**：

```json
{ "action": "createRoom" }
```

**伺服器處理**：

```javascript
if(msg.action === "createRoom"){
    let roomId;
    do {
        roomId = generateRoomId();
    } while(rooms[roomId]);  // 確保不重複
    
    rooms[roomId] = [ws];  // 房主為第一個元素
    ws.send(JSON.stringify({ 
        action: "roomCreated", 
        room: roomId 
    }));
}
```

### 2. 加入房間 (Join Room)

**客戶端 → 伺服器**：

```json
{
    "action": "joinRoom",
    "room": "1234"
}
```

**伺服器處理**：

```javascript
if(msg.action === "joinRoom"){
    const roomId = msg.room;
    
    // 驗證房號
    if(!roomId || typeof roomId !== 'string'){
        ws.send(JSON.stringify({ 
            action: "error", 
            message: "無效的房間號" 
        }));
        return;
    }
    
    // 檢查房間是否存在
    if(rooms[roomId]){
        // 檢查是否已滿（限制2人）
        if(rooms[roomId].length >= 2){
            ws.send(JSON.stringify({ 
                action: "error", 
                message: "房間已滿" 
            }));
            return;
        }
        
        rooms[roomId].push(ws);
        ws.send(JSON.stringify({ 
            action: "joinedRoom", 
            room: roomId 
        }));
        
        // 通知房主
        const host = rooms[roomId][0];
        if(host && host.readyState === WebSocket.OPEN){
            host.send(JSON.stringify({ 
                action: "playerJoined", 
                room: roomId 
            }));
        }
    } else {
        ws.send(JSON.stringify({ 
            action: "error", 
            message: "房間不存在" 
        }));
    }
}
```

### 3. 開始遊戲 (Start Game)

**客戶端 → 伺服器**：

```json
{
    "action": "startGame",
    "room": "1234",
    "whiteTimeMs": 600000,
    "blackTimeMs": 600000,
    "incrementMs": 0,
    "hostColor": "White",
    "gameModes": {
        "霧戰": false,
        "骰子": true
    },
    "minePositions": []
}
```

**伺服器處理**：

```javascript
if(msg.action === "startGame"){
    const roomId = msg.room;
    if(rooms[roomId] && rooms[roomId].length === 2){
        // 初始化計時器
        const whiteIsA = (msg.hostColor === "White");
        gameTimers[roomId] = {
            timeA: whiteIsA ? msg.whiteTimeMs : msg.blackTimeMs,
            timeB: whiteIsA ? msg.blackTimeMs : msg.whiteTimeMs,
            currentPlayer: "White",
            lastSwitchTime: null,  // 等待第一步才開始計時
            whiteIsA: whiteIsA,
            incrementMs: msg.incrementMs || 0
        };
        
        // 如果啟用骰子模式，初始化骰子狀態
        if(msg.gameModes && msg.gameModes['骰子']) {
            diceRolls[roomId] = {
                currentPlayer: "White",
                movesRemaining: 3
            };
        }
        
        // 廣播給所有玩家
        const startMessage = {
            action: "gameStart",
            room: roomId,
            whiteTimeMs: msg.whiteTimeMs,
            blackTimeMs: msg.blackTimeMs,
            incrementMs: msg.incrementMs,
            hostColor: msg.hostColor,
            gameModes: msg.gameModes,
            minePositions: msg.minePositions,
            serverTimestamp: Date.now() + 500,  // 補償網路延遲
            timerState: gameTimers[roomId]
        };
        
        rooms[roomId].forEach(client => {
            if(client.readyState === WebSocket.OPEN){
                client.send(JSON.stringify(startMessage));
            }
        });
    }
}
```

### 4. 移動棋子 (Move)

**客戶端 → 伺服器**：

```json
{
    "action": "move",
    "room": "1234",
    "fromRow": 6,
    "fromCol": 4,
    "toRow": 4,
    "toCol": 4,
    "promotion": null
}
```

**伺服器處理** - 參見下一節「計時器同步」

---

## 計時器同步 (Timer Synchronization)

### 計時器原理 (Timer Mechanics)

伺服器端計時器同步確保雙方時間一致：

1. **初始化**：遊戲開始時創建計時器，但 `lastSwitchTime` 設為 `null`
2. **第一步**：第一步棋不扣時間，不加增量，設定 `lastSwitchTime`
3. **後續步**：計算經過時間，扣除時間，加上增量，切換玩家

### 移動處理邏輯 (Move Handling Logic)

```javascript
if(msg.action === "move"){
    const roomId = msg.room;
    
    // 驗證房間和發送者
    if(!rooms[roomId] || !rooms[roomId].includes(ws)) {
        ws.send(JSON.stringify({ 
            action: "error", 
            message: "無效的房間" 
        }));
        return;
    }
    
    // 驗證座標 (0-7)
    if(msg.fromRow < 0 || msg.fromRow >= 8 || 
       msg.toRow < 0 || msg.toRow >= 8) {
        ws.send(JSON.stringify({ 
            action: "error", 
            message: "座標超出範圍" 
        }));
        return;
    }
    
    if(rooms[roomId] && gameTimers[roomId]){
        const timer = gameTimers[roomId];
        const currentTime = Date.now();
        
        // 檢查是否為第一步
        const isFirstMove = (timer.lastSwitchTime === null);
        
        // 計算經過時間
        let elapsedMs = 0;
        if (!isFirstMove) {
            elapsedMs = currentTime - timer.lastSwitchTime;
        }
        
        const playerWhoJustMoved = timer.currentPlayer;
        
        // 更新時間
        if(playerWhoJustMoved === "White"){
            const whiteTime = timer.whiteIsA ? timer.timeA : timer.timeB;
            let newWhiteTime;
            
            if (isFirstMove) {
                newWhiteTime = whiteTime;  // 第一步不扣時間
            } else {
                newWhiteTime = Math.max(0, whiteTime - elapsedMs) + timer.incrementMs;
            }
            
            if(timer.whiteIsA){
                timer.timeA = newWhiteTime;
            } else {
                timer.timeB = newWhiteTime;
            }
            
            timer.currentPlayer = "Black";  // 切換到黑方
        } else {
            // 黑方邏輯類似
            const blackTime = timer.whiteIsA ? timer.timeB : timer.timeA;
            let newBlackTime;
            
            if (isFirstMove) {
                newBlackTime = blackTime;
            } else {
                newBlackTime = Math.max(0, blackTime - elapsedMs) + timer.incrementMs;
            }
            
            if(timer.whiteIsA){
                timer.timeB = newBlackTime;
            } else {
                timer.timeA = newBlackTime;
            }
            
            timer.currentPlayer = "White";  // 切換到白方
        }
        
        // 更新最後切換時間
        timer.lastSwitchTime = currentTime;
        
        // 廣播移動和計時器狀態
        const moveMessage = {
            ...msg,
            timerState: {
                timeA: timer.timeA,
                timeB: timer.timeB,
                currentPlayer: timer.currentPlayer,
                lastSwitchTime: timer.lastSwitchTime
            }
        };
        
        rooms[roomId].forEach(client => {
            if(client.readyState === WebSocket.OPEN){
                client.send(JSON.stringify(moveMessage));
            }
        });
    }
}
```

### 關鍵點 (Key Points)

1. **第一步特殊處理**：不扣時間，不加增量
2. **毫秒精度**：使用毫秒確保精確計時
3. **時間補償**：伺服器時間戳加 500ms 補償網路延遲
4. **防止負數**：使用 `Math.max(0, ...)` 確保時間不為負

---

## 骰子模式邏輯 (Dice Mode Logic)

### 骰子模式概述 (Dice Mode Overview)

骰子模式中，玩家每回合擲 3 個骰子，必須移動 3 種不同類型的棋子。

### 1. 請求骰子 (Request Dice)

**客戶端 → 伺服器**：

```json
{
    "action": "requestDice",
    "room": "1234",
    "numMovablePieces": 6
}
```

**伺服器處理**：

```javascript
if(msg.action === "requestDice"){
    const roomId = msg.room;
    
    if(rooms[roomId] && diceRolls[roomId]){
        const numMovablePieces = msg.numMovablePieces || 1;
        
        // 生成3個隨機索引
        const rolls = [];
        for(let i = 0; i < 3; i++){
            rolls.push(Math.floor(Math.random() * numMovablePieces));
        }
        
        // 重置剩餘移動次數
        diceRolls[roomId].movesRemaining = 3;
        
        // 廣播骰子結果
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

### 2. 將軍中斷 (Check Interruption)

當玩家在骰子回合中將對手將軍（但不將死）時，必須先讓對手解除將軍。

**中斷邏輯**：

```javascript
// 在 move 處理中
if(diceRolls[roomId]) {
    // 檢查是否有將軍中斷
    if(msg.diceCheckInterruption && msg.savedDiceMoves > 0) {
        // 保存被中斷的玩家
        diceRolls[roomId].interruptedPlayer = playerWhoJustMoved;
        diceRolls[roomId].savedMovesRemaining = msg.savedDiceMoves;
        diceRolls[roomId].movesRemaining = 0;
        
        // 強制切換到對手
        shouldSwitchPlayer = true;
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
```

**恢復邏輯**：

```javascript
// 在 move 處理中，所有骰子移動完成後
if(diceRolls[roomId] && diceRolls[roomId].movesRemaining <= 0) {
    // 檢查是否需要恢復中斷的玩家
    if(diceRolls[roomId].interruptedPlayer && 
       diceRolls[roomId].savedMovesRemaining > 0 &&
       playerWhoJustMoved !== diceRolls[roomId].interruptedPlayer &&
       !checkInterruptionOccurred) {
        
        // 恢復被中斷玩家的回合
        timer.currentPlayer = diceRolls[roomId].interruptedPlayer;
        diceRolls[roomId].currentPlayer = diceRolls[roomId].interruptedPlayer;
        diceRolls[roomId].movesRemaining = diceRolls[roomId].savedMovesRemaining;
        
        // 清除中斷狀態
        delete diceRolls[roomId].interruptedPlayer;
        delete diceRolls[roomId].savedMovesRemaining;
    }
}
```

### 3. 骰子狀態同步 (Dice State Sync)

在移動訊息中附加骰子狀態：

```javascript
if(diceRolls[roomId]) {
    moveMessage.diceState = {
        movesRemaining: diceRolls[roomId].movesRemaining,
        hasInterruption: !!diceRolls[roomId].interruptedPlayer
    };
}
```

---

## 安全機制 (Security Mechanisms)

### 1. 速率限制 (Rate Limiting)

```javascript
function checkRateLimit(ws) {
    const now = Date.now();
    const limit = rateLimits.get(ws) || { count: 0, resetTime: now + 1000 };
    
    if(now > limit.resetTime) {
        limit.count = 1;
        limit.resetTime = now + 1000;
    } else {
        limit.count++;
        if(limit.count > 50) {
            return false;
        }
    }
    
    rateLimits.set(ws, limit);
    return true;
}
```

### 2. 輸入驗證 (Input Validation)

#### JSON 解析驗證

```javascript
let msg;
try {
    msg = JSON.parse(message);
} catch (error) {
    ws.send(JSON.stringify({ 
        action: "error", 
        message: "無效的訊息格式" 
    }));
    return;
}
```

#### 房號驗證

```javascript
if(!roomId || typeof roomId !== 'string'){
    ws.send(JSON.stringify({ 
        action: "error", 
        message: "無效的房間號" 
    }));
    return;
}
```

#### 座標驗證

```javascript
if(typeof msg.fromRow !== 'number' || typeof msg.fromCol !== 'number' ||
   typeof msg.toRow !== 'number' || typeof msg.toCol !== 'number') {
    ws.send(JSON.stringify({ 
        action: "error", 
        message: "無效的移動數據格式" 
    }));
    return;
}

if(msg.fromRow < 0 || msg.fromRow >= 8 || msg.fromCol < 0 || msg.fromCol >= 8 ||
   msg.toRow < 0 || msg.toRow >= 8 || msg.toCol < 0 || msg.toCol >= 8) {
    ws.send(JSON.stringify({ 
        action: "error", 
        message: "移動座標超出範圍" 
    }));
    return;
}
```

### 3. 連線管理 (Connection Management)

#### 斷線處理

```javascript
ws.on('close', () => {
    for(const roomId in rooms){
        handlePlayerLeaveRoom(ws, roomId);
    }
    rateLimits.delete(ws);  // 清理速率限制資料
});
```

#### 房間清理

```javascript
if(rooms[roomId].length === 0){
    delete rooms[roomId];
    delete gameTimers[roomId];
    delete diceRolls[roomId];
}
```

---

## 程式碼結構 (Code Structure)

### 檔案位置 (File Location)

```
server.js  (主伺服器檔案)
```

### 主要區塊 (Main Sections)

```javascript
// 1. 依賴和初始化
const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: process.env.PORT || 3000 });

// 2. 資料結構
const rooms = {};
const gameTimers = {};
const diceRolls = {};
const rateLimits = new Map();

// 3. 工具函數
function generateRoomId() { ... }
function checkRateLimit(ws) { ... }
function handlePlayerLeaveRoom(ws, roomId) { ... }

// 4. WebSocket 連線處理
wss.on('connection', ws => {
    ws.on('message', message => {
        // 訊息處理邏輯
    });
    
    ws.on('close', () => {
        // 斷線處理
    });
});
```

### 訊息處理流程 (Message Flow)

```
客戶端 → WebSocket → 伺服器
   ↓
速率限制檢查
   ↓
JSON 解析
   ↓
訊息類型路由
   ↓
業務邏輯處理
   ↓
廣播給房間內玩家
   ↓
WebSocket → 客戶端
```

---

## 部署說明 (Deployment)

### Render 部署 (Render Deployment)

伺服器部署在 Render 平台：

- **URL**: wss://chess-server-mjg6.onrender.com
- **埠號**: 由 `process.env.PORT` 環境變數指定
- **協議**: WebSocket (WSS)

### 本地測試 (Local Testing)

```bash
# 安裝依賴
npm install ws

# 啟動伺服器
node server.js

# 伺服器將在 http://localhost:3000 運行
```

---

## 效能考量 (Performance Considerations)

### 1. 速率限制 (Rate Limiting)

- 每秒最多 50 條訊息
- 防止 DoS 攻擊
- 使用 Map 資料結構快速查找

### 2. 房間限制 (Room Limits)

- 每個房間最多 2 人
- 房間空時自動清理
- 避免記憶體洩漏

### 3. 訊息廣播 (Message Broadcasting)

- 只廣播給房間內玩家
- 檢查 WebSocket 狀態 (`readyState === WebSocket.OPEN`)
- 避免發送到已關閉的連線

---

## 錯誤處理 (Error Handling)

### 常見錯誤 (Common Errors)

1. **JSON 解析錯誤**：
   ```javascript
   catch (error) {
       ws.send(JSON.stringify({ 
           action: "error", 
           message: "無效的訊息格式" 
       }));
   }
   ```

2. **房間不存在**：
   ```javascript
   ws.send(JSON.stringify({ 
       action: "error", 
       message: "房間不存在" 
   }));
   ```

3. **房間已滿**：
   ```javascript
   ws.send(JSON.stringify({ 
       action: "error", 
       message: "房間已滿" 
   }));
   ```

4. **速率限制**：
   ```javascript
   ws.send(JSON.stringify({ 
       action: "error", 
       message: "訊息發送過快，請稍後再試" 
   }));
   ```

---

## 未來改進 (Future Improvements)

1. **持久化**：將房間和遊戲狀態存儲到資料庫
2. **重連機制**：支援斷線後自動重連
3. **身份驗證**：添加使用者登入和身份驗證
4. **排名系統**：記錄玩家戰績和積分
5. **觀戰模式**：允許其他玩家觀看對局
6. **聊天功能**：完善玩家間通訊
7. **反作弊**：伺服器端驗證移動合法性
8. **日誌記錄**：記錄所有遊戲事件用於分析

---

## 相關文件 (Related Documentation)

- [伺服器遷移說明](SERVER_MIGRATION.md)
- [線上對戰功能](ONLINE_MODE_FEATURE.md)
- [骰子模式文件](DICE_MODE_DOCUMENTATION.md)
- [遊戲規則文件](GAME_RULES_DOCUMENTATION.md)
