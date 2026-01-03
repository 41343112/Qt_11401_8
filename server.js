const WebSocket = require('ws');

// 使用 Render 自動提供的 PORT
const wss = new WebSocket.Server({ port: process.env.PORT || 3000 });

// 房間資料
// rooms[roomId] = [ws1, ws2, ...]
// 注意：陣列中第一個元素 (index 0) 始終為房主
const rooms = {};

// 遊戲計時狀態
const gameTimers = {}; // gameTimers[roomId] = { timeA, timeB, currentPlayer, lastSwitchTime, whiteIsA }

// 骰子模式狀態 (Dice Mode State)
const diceRolls = {}; // diceRolls[roomId] = { currentPlayer, rolls: [{row, col}], movesRemaining }

// 速率限制狀態 (Rate Limiting)
const rateLimits = new Map(); // ws -> { count, resetTime }

// 速率限制檢查函數
function checkRateLimit(ws) {
    const now = Date.now();
    const limit = rateLimits.get(ws) || { count: 0, resetTime: now + 1000 };
    
    if(now > limit.resetTime) {
        // 重置計數器
        limit.count = 1;
        limit.resetTime = now + 1000;
    } else {
        limit.count++;
        if(limit.count > 50) { // 每秒最多 50 條訊息
            return false;
        }
    }
    
    rateLimits.set(ws, limit);
    return true;
}

// 生成 4 位數字房號
function generateRoomId() {
    return Math.floor(1000 + Math.random() * 9000).toString();
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
        delete diceRolls[roomId];   // 清理骰子狀態
    }
}

wss.on('connection', ws => {
    ws.on('message', message => {
        // 速率限制檢查
        if(!checkRateLimit(ws)) {
            console.log('[Server] Rate limit exceeded');
            ws.send(JSON.stringify({ action: "error", message: "訊息發送過快，請稍後再試" }));
            return;
        }
        
        let msg;
        try {
            msg = JSON.parse(message);
        } catch (error) {
            console.error('[Server] JSON parse error:', error.message);
            ws.send(JSON.stringify({ action: "error", message: "無效的訊息格式" }));
            return;
        }

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
            if(!roomId || typeof roomId !== 'string'){
                ws.send(JSON.stringify({ action: "error", message: "無效的房間號" }));
                return;
            }
            if(rooms[roomId]){
                // 檢查房間是否已滿（限制2人）
                if(rooms[roomId].length >= 2){
                    ws.send(JSON.stringify({ action: "error", message: "房間已滿" }));
                    return;
                }
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
                
                // 如果啟用骰子模式，初始化骰子狀態
                if(gameModes && gameModes['骰子']) {
                    diceRolls[roomId] = {
                        currentPlayer: "White",  // 白方先手
                        movesRemaining: 3
                    };
                    console.log('[Server] Dice mode initialized for room', roomId);
                    // 不在這裡發送骰子，等待客戶端請求
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
            
            // 驗證房間和發送者
            if(!rooms[roomId] || !rooms[roomId].includes(ws)) {
                console.log('[Server] ERROR: Invalid room or sender not in room');
                ws.send(JSON.stringify({ action: "error", message: "無效的房間或未加入該房間" }));
                return;
            }
            
            // 驗證移動數據的存在性和類型
            if(typeof msg.fromRow !== 'number' || typeof msg.fromCol !== 'number' ||
               typeof msg.toRow !== 'number' || typeof msg.toCol !== 'number') {
                console.log('[Server] ERROR: Invalid move data types');
                ws.send(JSON.stringify({ action: "error", message: "無效的移動數據格式" }));
                return;
            }
            
            // 驗證座標範圍（0-7）
            if(msg.fromRow < 0 || msg.fromRow >= 8 || msg.fromCol < 0 || msg.fromCol >= 8 ||
               msg.toRow < 0 || msg.toRow >= 8 || msg.toCol < 0 || msg.toCol >= 8) {
                console.log('[Server] ERROR: Move coordinates out of bounds');
                ws.send(JSON.stringify({ action: "error", message: "移動座標超出範圍" }));
                return;
            }
            
            console.log('[Server] Move received for room:', roomId, 'from:', msg.fromRow, msg.fromCol, 'to:', msg.toRow, msg.toCol);
            console.log('[Server] gameTimers exists:', !!gameTimers[roomId], 'rooms exists:', !!rooms[roomId]);
            
            if(rooms[roomId] && gameTimers[roomId]){
                const timer = gameTimers[roomId];
                const currentTime = Date.now();  // 保持毫秒精度
                
                // 檢查是否為第一步棋（計時器尚未啟動）
                const isFirstMove = (timer.lastSwitchTime === null);
                
                // 計算經過的時間（毫秒）
                let elapsedMs = 0;
                if (!isFirstMove) {
                    // 不是第一步，計算經過的時間
                    elapsedMs = currentTime - timer.lastSwitchTime;
                }
                // 如果是第一步，elapsedMs 保持為 0，不扣除時間
                
                // currentPlayer 表示當前輪到誰（正在思考的玩家）
                // 當收到移動訊息時，表示 currentPlayer 剛走完棋
                // 所以要從 currentPlayer 的時間中扣除 elapsed，然後切換到對手
                
                const playerWhoJustMoved = timer.currentPlayer;
                
                // 檢查是否在骰子模式中還有剩餘移動，或者移動造成將軍中斷
                let shouldSwitchPlayer = true;
                let checkInterruptionOccurred = false;
                
                if(diceRolls[roomId]) {
                    console.log('[Server] Dice mode: checking if should switch player. Moves remaining before:', diceRolls[roomId].movesRemaining);
                    
                    // 檢查是否有將軍中斷標記
                    if(msg.diceCheckInterruption && msg.savedDiceMoves > 0) {
                        console.log('[Server] Dice check interruption detected! Saved moves:', msg.savedDiceMoves);
                        checkInterruptionOccurred = true;
                        
                        // 保存被中斷的玩家和剩餘移動次數
                        diceRolls[roomId].interruptedPlayer = playerWhoJustMoved;
                        diceRolls[roomId].savedMovesRemaining = msg.savedDiceMoves;
                        diceRolls[roomId].movesRemaining = 0;  // 設為0以強制切換回合
                        
                        // 強制切換到對手
                        shouldSwitchPlayer = true;
                        console.log('[Server] Forcing turn switch for check interruption');
                    } else {
                        // 正常骰子邏輯：先扣除這次移動
                        if(diceRolls[roomId].movesRemaining > 0) {
                            diceRolls[roomId].movesRemaining--;
                        }
                        // 檢查扣除後是否還有剩餘移動
                        if(diceRolls[roomId].movesRemaining > 0) {
                            shouldSwitchPlayer = false;
                            console.log('[Server] Dice moves remaining:', diceRolls[roomId].movesRemaining, '- NOT switching player');
                        } else {
                            console.log('[Server] All dice moved - will switch player');
                        }
                    }
                }
                
                if(playerWhoJustMoved === "White"){
                    // 白方剛走棋，從白方時間扣除
                    const whiteTime = timer.whiteIsA ? timer.timeA : timer.timeB;
                    const newWhiteTime = Math.max(0, whiteTime - elapsedMs) + timer.incrementMs;
                    
                    if(timer.whiteIsA){
                        timer.timeA = newWhiteTime;
                    } else {
                        timer.timeB = newWhiteTime;
                    }
                    
                    // 只在應該切換時才切換到黑方
                    if(shouldSwitchPlayer) {
                        timer.currentPlayer = "Black";
                    }
                } else {
                    // 黑方剛走棋，從黑方時間扣除
                    const blackTime = timer.whiteIsA ? timer.timeB : timer.timeA;
                    const newBlackTime = Math.max(0, blackTime - elapsedMs) + timer.incrementMs;
                    
                    if(timer.whiteIsA){
                        timer.timeB = newBlackTime;
                    } else {
                        timer.timeA = newBlackTime;
                    }
                    
                    // 只在應該切換時才切換到白方
                    if(shouldSwitchPlayer) {
                        timer.currentPlayer = "White";
                    }
                }
                
                // 更新最後切換時間（如果是第一步，這裡開始計時）
                // 加上緩衝時間以補償網路延遲，確保客戶端收到訊息時不會扣錯時間
                timer.lastSwitchTime = currentTime + 1000;  // 加 1000 毫秒 (1 秒) 緩衝
                
                // 如果骰子模式所有移動完成，檢查是否需要恢復中斷的玩家（在廣播之前）
                if(diceRolls[roomId] && diceRolls[roomId].movesRemaining <= 0) {
                    // 檢查是否有被中斷的玩家需要恢復
                    // 重要：只在「防守方完成防禦」時恢復，不是在「攻擊方剛將軍」時
                    // 判斷依據：interruptedPlayer存在 AND 當前移動的玩家不是interruptedPlayer（即防守方剛移動完）
                    if(diceRolls[roomId].interruptedPlayer && 
                       diceRolls[roomId].savedMovesRemaining > 0 &&
                       playerWhoJustMoved !== diceRolls[roomId].interruptedPlayer &&
                       !checkInterruptionOccurred) {
                        // 防守方剛完成防禦，恢復被中斷的攻擊方的回合
                        console.log('[Server] Defender just moved, restoring interrupted player:', diceRolls[roomId].interruptedPlayer);
                        timer.currentPlayer = diceRolls[roomId].interruptedPlayer;
                        diceRolls[roomId].currentPlayer = diceRolls[roomId].interruptedPlayer;
                        diceRolls[roomId].movesRemaining = diceRolls[roomId].savedMovesRemaining;
                        
                        // 清除中斷狀態
                        delete diceRolls[roomId].interruptedPlayer;
                        delete diceRolls[roomId].savedMovesRemaining;
                        
                        console.log('[Server] Turn restored to:', timer.currentPlayer, 'with', diceRolls[roomId].movesRemaining, 'moves remaining');
                    } else {
                        // 正常情況：保持 movesRemaining = 0，讓新玩家在收到訊息後骰新骰子
                        // 更新 currentPlayer 為新的當前玩家（已在上面切換）
                        diceRolls[roomId].currentPlayer = timer.currentPlayer;
                        // 不要在這裡重置 movesRemaining，讓客戶端檢測到 0 後自己骰骰子
                        console.log('[Server] Turn switched to:', timer.currentPlayer, 'movesRemaining stays 0 for dice roll');
                    }
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
                
                // 如果是骰子模式，添加骰子狀態
                if(diceRolls[roomId]) {
                    moveMessage.diceState = {
                        movesRemaining: diceRolls[roomId].movesRemaining,
                        hasInterruption: !!diceRolls[roomId].interruptedPlayer  // 告訴客戶端是否有中斷狀態
                    };
                }
                
                rooms[roomId].forEach(client => {
                    if(client.readyState === WebSocket.OPEN){
                        console.log('[Server] Broadcasting move to client in room', roomId);
                        client.send(JSON.stringify(moveMessage));
                    }
                });
                console.log('[Server] Move broadcast complete. Sent to', rooms[roomId].length, 'clients');
            } else if(rooms[roomId]){
                // 如果沒有計時器狀態，只廣播移動（向後兼容）
                console.log('[Server] No game timer - using fallback broadcast for room:', roomId);
                rooms[roomId].forEach(client => {
                    if(client !== ws && client.readyState === WebSocket.OPEN){
                        console.log('[Server] Fallback: Broadcasting move to other client');
                        client.send(JSON.stringify(msg));
                    }
                });
            } else {
                console.log('[Server] ERROR: Room not found for move:', roomId);
            }
        }

        // 處理骰子請求
        else if(msg.action === "requestDice"){
            const roomId = msg.room;
            console.log('[Server] Dice request for room:', roomId, 'Dice state exists:', !!diceRolls[roomId]);
            
            if(rooms[roomId] && diceRolls[roomId]){
                const numMovablePieces = msg.numMovablePieces || 1;
                console.log('[Server] Generating dice for', numMovablePieces, 'piece types');
                
                // 生成3個隨機索引（可重複）
                const rolls = [];
                for(let i = 0; i < 3; i++){
                    rolls.push(Math.floor(Math.random() * numMovablePieces));
                }
                
                console.log('[Server] Generated rolls:', rolls, 'for player:', diceRolls[roomId].currentPlayer);
                
                // 更新骰子剩餘移動次數為3（新回合開始）
                diceRolls[roomId].movesRemaining = 3;
                console.log('[Server] Reset movesRemaining to 3 after dice roll');
                
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
            } else {
                console.log('[Server] ERROR: Cannot process dice request - room or dice state not found');
                console.log('[Server] Room exists:', !!rooms[roomId], 'Dice state exists:', !!diceRolls[roomId]);
            }
        }

        // 骰子模式：將軍中斷通知
        else if(msg.action === "diceCheckInterruption"){
            const roomId = msg.room;
            console.log('[Server] Dice check interruption for room:', roomId, 'Saved moves:', msg.savedMovesRemaining);
            
            if(rooms[roomId] && diceRolls[roomId] && gameTimers[roomId]){
                // 保存被中斷的玩家和剩餘移動次數
                diceRolls[roomId].interruptedPlayer = diceRolls[roomId].currentPlayer;
                diceRolls[roomId].savedMovesRemaining = msg.savedMovesRemaining;
                diceRolls[roomId].movesRemaining = 0;  // 清空當前移動次數，允許對手移動
                
                // 強制切換到對手（被將軍的玩家）
                const timer = gameTimers[roomId];
                timer.currentPlayer = (timer.currentPlayer === "White") ? "Black" : "White";
                diceRolls[roomId].currentPlayer = timer.currentPlayer;
                
                console.log('[Server] Turn switched to:', timer.currentPlayer, 'to respond to check');
                
                // 廣播給所有客戶端
                rooms[roomId].forEach(client => {
                    if(client.readyState === WebSocket.OPEN){
                        client.send(JSON.stringify({
                            action: "diceCheckInterrupted",
                            room: roomId,
                            currentPlayer: timer.currentPlayer
                        }));
                    }
                });
            }
        }

        // 骰子模式：將軍解除通知
        else if(msg.action === "diceCheckResolved"){
            const roomId = msg.room;
            console.log('[Server] Dice check resolved for room:', roomId);
            
            if(rooms[roomId] && diceRolls[roomId] && gameTimers[roomId]){
                // 恢復被中斷玩家的回合和剩餘移動次數
                const interruptedPlayer = diceRolls[roomId].interruptedPlayer;
                const savedMoves = diceRolls[roomId].savedMovesRemaining || 0;
                
                if(interruptedPlayer && savedMoves > 0){
                    const timer = gameTimers[roomId];
                    timer.currentPlayer = interruptedPlayer;
                    diceRolls[roomId].currentPlayer = interruptedPlayer;
                    diceRolls[roomId].movesRemaining = savedMoves;
                    
                    console.log('[Server] Turn restored to:', interruptedPlayer, 'with', savedMoves, 'moves remaining');
                    
                    // 清除中斷狀態
                    delete diceRolls[roomId].interruptedPlayer;
                    delete diceRolls[roomId].savedMovesRemaining;
                    
                    // 廣播給所有客戶端
                    rooms[roomId].forEach(client => {
                        if(client.readyState === WebSocket.OPEN){
                            client.send(JSON.stringify({
                                action: "diceCheckRestored",
                                room: roomId,
                                currentPlayer: timer.currentPlayer,
                                movesRemaining: savedMoves
                            }));
                        }
                    });
                }
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

        // 廣播遊戲結束訊息（將殺）
        else if(msg.action === "gameOver"){
            const roomId = msg.room;
            console.log('[Server] Game over received for room:', roomId, 'result:', msg.result);
            if(rooms[roomId]){
                // 廣播給房間內所有其他玩家
                rooms[roomId].forEach(client => {
                    if(client !== ws && client.readyState === WebSocket.OPEN){
                        console.log('[Server] Forwarding game over to opponent');
                        client.send(JSON.stringify(msg));
                    }
                });
            } else {
                console.log('[Server] ERROR: Room not found for gameOver:', roomId);
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
        // 清理速率限制資料
        rateLimits.delete(ws);
    });
});

console.log("WebSocket relay server running");
