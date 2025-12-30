const WebSocket = require('ws');

// 使用 Render 自動提供的 PORT
const wss = new WebSocket.Server({ port: process.env.PORT || 3000 });

// 房間資料
// rooms[roomId] = [ws1, ws2, ...]
// 注意：陣列中第一個元素 (index 0) 始終為房主
const rooms = {};

// 遊戲計時狀態
const gameTimers = {}; // gameTimers[roomId] = { timeA, timeB, currentPlayer, lastSwitchTime, whiteIsA }

// 骰子狀態
const diceStates = {}; // diceStates[roomId] = { diceRolled: [type1, type2, type3], diceUsed: [bool1, bool2, bool3] }

// 生成 4 位數字房號
function generateRoomId() {
    return Math.floor(1000 + Math.random() * 9000).toString();
}

// 生成隨機骰子（3個棋子類型）
function rollDice() {
    const pieceTypes = ["Pawn", "Knight", "Bishop", "Rook", "Queen"];
    const rolled = [];
    for (let i = 0; i < 3; i++) {
        const randomIndex = Math.floor(Math.random() * pieceTypes.length);
        rolled.push(pieceTypes[randomIndex]);
    }
    return rolled;
}

// 處理玩家離開房間的共用邏輯
// 此函數處理玩家明確離開或斷線的情況
// 注意：目前設計為 2 人房間，因此 includes() 的 O(n) 複雜度是可接受的
function handlePlayerLeaveRoom(ws, roomId) {
    if(!rooms[roomId] || !rooms[roomId].includes(ws)) {
        return; // 玩家不在此房間
    }
    
    // 檢查離開的玩家是否為房主 (index 0)
    const wasHost = rooms[roomId][0] === ws;
    
    // 通知房間內其他玩家
    rooms[roomId].forEach(client => {
        if(client !== ws && client.readyState === WebSocket.OPEN){
            client.send(JSON.stringify({ action: "playerLeft", room: roomId }));
        }
    });
    
    // 從房間移除離開的玩家
    rooms[roomId] = rooms[roomId].filter(c => c !== ws);
    
    // 如果房主離開且房間內還有其他玩家，通知新房主
    if(wasHost && rooms[roomId].length > 0){
        const newHost = rooms[roomId][0];
        if(newHost.readyState === WebSocket.OPEN){
            newHost.send(JSON.stringify({ 
                action: "promotedToHost", 
                room: roomId 
            }));
        }
    }
    
    // 如果房間空了，刪除房間
    if(rooms[roomId].length === 0){
        delete rooms[roomId];
        delete gameTimers[roomId];  // 清理計時器狀態
    }
}

wss.on('connection', ws => {
    ws.on('message', message => {
        const msg = JSON.parse(message);

        // 創建房間
        if(msg.action === "createRoom"){
            let roomId;
            do {
                roomId = generateRoomId();
            } while(rooms[roomId]); // 確保不重複

            rooms[roomId] = [ws];
            ws.send(JSON.stringify({ action: "roomCreated", room: roomId }));
        }

        // 加入房間
        else if(msg.action === "joinRoom"){
            const roomId = msg.room;
            if(rooms[roomId]){
                rooms[roomId].push(ws);
                ws.send(JSON.stringify({ action: "joinedRoom", room: roomId }));
                
                // 通知房主有玩家加入
                const host = rooms[roomId][0];
                if(host && host.readyState === WebSocket.OPEN){
                    host.send(JSON.stringify({ action: "playerJoined", room: roomId }));
                }
            } else {
                ws.send(JSON.stringify({ action: "error", message: "房間不存在" }));
            }
        }

        // 開始遊戲 - 伺服器廣播給房間內所有玩家以確保同步
        else if(msg.action === "startGame"){
            const roomId = msg.room;
            if(rooms[roomId] && rooms[roomId].length === 2){
                // 初始化遊戲計時狀態
                const whiteTimeMs = msg.whiteTimeMs || 0;
                const blackTimeMs = msg.blackTimeMs || 0;
                const hostColor = msg.hostColor; // "White" or "Black"
                const gameModes = msg.gameModes || {}; // 遊戲模式設定
                const minePositions = msg.minePositions || []; // 地雷位置（如果有）
                
                // 確定哪個玩家是 A (房主) 和 B (房客)
                const whiteIsA = (hostColor === "White");
                
                // 初始化計時器狀態 (使用毫秒)
                gameTimers[roomId] = {
                    timeA: whiteIsA ? whiteTimeMs : blackTimeMs,  // 房主的時間
                    timeB: whiteIsA ? blackTimeMs : whiteTimeMs,  // 房客的時間
                    currentPlayer: "White",  // 白方先手
                    lastSwitchTime: null,  // 設為 null，等待第一步棋才開始計時
                    whiteIsA: whiteIsA,  // 記錄白方是否為房主
                    incrementMs: msg.incrementMs || 0
                };
                
                // 初始化骰子狀態（如果啟用了骰子模式）
                if (gameModes["骰子"]) {
                    diceStates[roomId] = {
                        diceRolled: rollDice(),
                        diceUsed: [false, false, false],
                        diceMovesCount: 0  // 追蹤當前玩家已走的骰子數
                    };
                }
                
                // 加上伺服器時間戳記以確保同步
                // 使用未來時間（當前時間 + 500ms）以補償網路延遲
                const startMessage = {
                    action: "gameStart",
                    room: roomId,
                    whiteTimeMs: whiteTimeMs,
                    blackTimeMs: blackTimeMs,
                    incrementMs: msg.incrementMs,
                    hostColor: msg.hostColor,
                    gameModes: gameModes,  // 傳遞遊戲模式給所有玩家
                    minePositions: minePositions,  // 傳遞地雷位置給所有玩家
                    serverTimestamp: Date.now() + 500,  // 添加 500ms 緩衝以補償網路延遲
                    // 發送初始計時器狀態
                    timerState: {
                        timeA: gameTimers[roomId].timeA,
                        timeB: gameTimers[roomId].timeB,
                        currentPlayer: gameTimers[roomId].currentPlayer,
                        lastSwitchTime: gameTimers[roomId].lastSwitchTime
                    }
                };
                
                // 如果啟用骰子模式，添加骰子狀態
                if (diceStates[roomId]) {
                    startMessage.diceState = {
                        diceRolled: diceStates[roomId].diceRolled,
                        diceUsed: diceStates[roomId].diceUsed
                    };
                }
                
                // 廣播給房間內所有玩家
                rooms[roomId].forEach(client => {
                    if(client.readyState === WebSocket.OPEN){
                        client.send(JSON.stringify(startMessage));
                    }
                });
            }
        }

        // 廣播落子訊息並更新計時器
        else if(msg.action === "move"){
            const roomId = msg.room;
            if(rooms[roomId] && gameTimers[roomId]){
                const timer = gameTimers[roomId];
                const currentTime = Math.floor(Date.now() / 1000);  // UNIX 秒數
                
                // 檢查是否為第一步棋（計時器尚未啟動）
                const isFirstMove = (timer.lastSwitchTime === null);
                
                // 計算經過的時間
                let elapsedMs = 0;
                if (!isFirstMove) {
                    // 不是第一步，計算經過的時間
                    const elapsed = currentTime - timer.lastSwitchTime;
                    elapsedMs = elapsed * 1000;
                }
                // 如果是第一步，elapsedMs 保持為 0，不扣除時間
                
                // currentPlayer 表示當前輪到誰（正在思考的玩家）
                // 當收到移動訊息時，表示 currentPlayer 剛走完棋
                // 所以要從 currentPlayer 的時間中扣除 elapsed，然後切換到對手
                
                const playerWhoJustMoved = timer.currentPlayer;
                
                // 處理骰子模式
                let shouldSwitchTurn = true;  // 默認切換回合
                if (diceStates[roomId]) {
                    const diceState = diceStates[roomId];
                    const movedPieceType = msg.movedPieceType;  // 客戶端需要發送移動的棋子類型
                    
                    // 標記骰子為已使用
                    if (movedPieceType && movedPieceType !== "King") {
                        for (let i = 0; i < diceState.diceRolled.length; i++) {
                            if (diceState.diceRolled[i] === movedPieceType && !diceState.diceUsed[i]) {
                                diceState.diceUsed[i] = true;
                                diceState.diceMovesCount++;
                                break;
                            }
                        }
                    }
                    
                    // 檢查是否所有骰子都已使用
                    const allDiceUsed = diceState.diceUsed.every(used => used);
                    
                    if (!allDiceUsed) {
                        // 還有骰子未使用，不切換回合
                        shouldSwitchTurn = false;
                    } else {
                        // 所有骰子都已使用，重新骰骰子並切換回合
                        diceState.diceRolled = rollDice();
                        diceState.diceUsed = [false, false, false];
                        diceState.diceMovesCount = 0;
                        shouldSwitchTurn = true;
                    }
                }
                
                if(shouldSwitchTurn && playerWhoJustMoved === "White"){
                    // 白方剛走棋，從白方時間扣除
                    const whiteTime = timer.whiteIsA ? timer.timeA : timer.timeB;
                    const newWhiteTime = Math.max(0, whiteTime - elapsedMs) + timer.incrementMs;
                    
                    if(timer.whiteIsA){
                        timer.timeA = newWhiteTime;
                    } else {
                        timer.timeB = newWhiteTime;
                    }
                    
                    // 切換到黑方
                    timer.currentPlayer = "Black";
                } else if(shouldSwitchTurn) {
                    // 黑方剛走棋，從黑方時間扣除
                    const blackTime = timer.whiteIsA ? timer.timeB : timer.timeA;
                    const newBlackTime = Math.max(0, blackTime - elapsedMs) + timer.incrementMs;
                    
                    if(timer.whiteIsA){
                        timer.timeB = newBlackTime;
                    } else {
                        timer.timeA = newBlackTime;
                    }
                    
                    // 切換到白方
                    timer.currentPlayer = "White";
                }
                // 如果不應該切換回合（骰子模式下還有骰子未使用），則保持 currentPlayer 不變
                
                // 更新最後切換時間（如果是第一步，這裡開始計時）
                // 加上緩衝時間以補償網路延遲，確保客戶端收到訊息時不會扣錯時間
                // 注意：只有在實際切換回合時才更新 lastSwitchTime
                if (shouldSwitchTurn) {
                    timer.lastSwitchTime = currentTime + 1;  // 加 1 秒緩衝
                }
                
                // 廣播移動訊息和計時器狀態
                const moveMessage = {
                    ...msg,
                    timerState: {
                        timeA: timer.timeA,
                        timeB: timer.timeB,
                        currentPlayer: timer.currentPlayer,
                        lastSwitchTime: timer.lastSwitchTime
                    }
                };
                
                // 如果啟用骰子模式，添加骰子狀態
                if (diceStates[roomId]) {
                    moveMessage.diceState = {
                        diceRolled: diceStates[roomId].diceRolled,
                        diceUsed: diceStates[roomId].diceUsed
                    };
                }
                
                rooms[roomId].forEach(client => {
                    if(client.readyState === WebSocket.OPEN){
                        client.send(JSON.stringify(moveMessage));
                    }
                });
            } else if(rooms[roomId]){
                // 如果沒有計時器狀態，只廣播移動（向後兼容）
                rooms[roomId].forEach(client => {
                    if(client !== ws && client.readyState === WebSocket.OPEN){
                        client.send(JSON.stringify(msg));
                    }
                });
            }
        }

        // 離開房間（遊戲開始前）
        else if(msg.action === "leaveRoom"){
            const roomId = msg.room;
            handlePlayerLeaveRoom(ws, roomId);
        }

        // 廣播投降訊息
        else if(msg.action === "surrender"){
            const roomId = msg.room;
            if(rooms[roomId]){
                rooms[roomId].forEach(client => {
                    if(client !== ws && client.readyState === WebSocket.OPEN){
                        client.send(JSON.stringify(msg));
                    }
                });
            }
        }

        // 廣播和棋請求
        else if(msg.action === "drawOffer"){
            const roomId = msg.room;
            if(rooms[roomId]){
                rooms[roomId].forEach(client => {
                    if(client !== ws && client.readyState === WebSocket.OPEN){
                        client.send(JSON.stringify(msg));
                    }
                });
            }
        }

        // 廣播和棋回應
        else if(msg.action === "drawResponse"){
            const roomId = msg.room;
            if(rooms[roomId]){
                rooms[roomId].forEach(client => {
                    if(client !== ws && client.readyState === WebSocket.OPEN){
                        client.send(JSON.stringify(msg));
                    }
                });
            }
        }
    });

    // 玩家斷線
    ws.on('close', () => {
        for(const roomId in rooms){
            handlePlayerLeaveRoom(ws, roomId);
        }
    });
});

console.log("WebSocket relay server running");
