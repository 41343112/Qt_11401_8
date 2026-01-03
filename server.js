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
            console.log('[Server] Move received for room:', roomId, 'from:', msg.fromRow, msg.fromCol, 'to:', msg.toRow, msg.toCol);
            console.log('[Server] gameTimers exists:', !!gameTimers[roomId], 'rooms exists:', !!rooms[roomId]);
            
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
                timer.lastSwitchTime = currentTime + 1;  // 加 1 秒緩衝
                
                // 如果骰子模式所有移動完成，重置或恢復骰子狀態（在廣播之前）
                if(diceRolls[roomId] && diceRolls[roomId].movesRemaining <= 0) {
                    // 檢查是否有被中斷的玩家需要恢復
                    if(diceRolls[roomId].interruptedPlayer && diceRolls[roomId].savedMovesRemaining > 0) {
                        // 有被中斷的玩家，恢復他們的回合而不是開始新回合
                        console.log('[Server] Restoring interrupted player:', diceRolls[roomId].interruptedPlayer);
                        timer.currentPlayer = diceRolls[roomId].interruptedPlayer;
                        diceRolls[roomId].currentPlayer = diceRolls[roomId].interruptedPlayer;
                        diceRolls[roomId].movesRemaining = diceRolls[roomId].savedMovesRemaining;
                        
                        // 清除中斷狀態
                        delete diceRolls[roomId].interruptedPlayer;
                        delete diceRolls[roomId].savedMovesRemaining;
                        
                        console.log('[Server] Turn restored to:', timer.currentPlayer, 'with', diceRolls[roomId].movesRemaining, 'moves remaining');
                    } else {
                        // 正常情況：重置骰子給下一個玩家
                        diceRolls[roomId].currentPlayer = timer.currentPlayer;
                        diceRolls[roomId].movesRemaining = 3;
                        console.log('[Server] Dice reset for next player:', timer.currentPlayer);
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
                        movesRemaining: diceRolls[roomId].movesRemaining
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
