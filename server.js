const WebSocket = require('ws');

// 使用 Render 自動提供的 PORT
const wss = new WebSocket.Server({ port: process.env.PORT || 3000 });

// 房間資料
const rooms = {}; // rooms[roomId] = [ws1, ws2, ...]

// 遊戲計時狀態
const gameTimers = {}; // gameTimers[roomId] = { timeA, timeB, currentPlayer, lastSwitchTime, whiteIsA }

// 生成 4 位數字房號
function generateRoomId() {
    return Math.floor(1000 + Math.random() * 9000).toString();
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
                
                // 確定哪個玩家是 A (房主) 和 B (房客)
                const whiteIsA = (hostColor === "White");
                
                // 初始化計時器狀態 (使用毫秒)
                gameTimers[roomId] = {
                    timeA: whiteIsA ? whiteTimeMs : blackTimeMs,  // 房主的時間
                    timeB: whiteIsA ? blackTimeMs : whiteTimeMs,  // 房客的時間
                    currentPlayer: "White",  // 白方先手
                    lastSwitchTime: Math.floor(Date.now() / 1000),  // UNIX 秒數
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
                    serverTimestamp: Date.now() + 500,  // 添加 500ms 緩衝以補償網路延遲
                    // 發送初始計時器狀態
                    timerState: {
                        timeA: gameTimers[roomId].timeA,
                        timeB: gameTimers[roomId].timeB,
                        currentPlayer: gameTimers[roomId].currentPlayer,
                        lastSwitchTime: gameTimers[roomId].lastSwitchTime
                    }
                };
                
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
                
                // 計算距離上次切換經過的時間（秒）
                const elapsed = currentTime - timer.lastSwitchTime;
                
                // 從當前玩家的時間中扣除經過的時間（轉換為毫秒）
                const elapsedMs = elapsed * 1000;
                
                if(timer.currentPlayer === "White"){
                    const timeToDeduct = timer.whiteIsA ? timer.timeA : timer.timeB;
                    const newTime = Math.max(0, timeToDeduct - elapsedMs);
                    
                    if(timer.whiteIsA){
                        timer.timeA = newTime + timer.incrementMs;  // 添加增量時間
                    } else {
                        timer.timeB = newTime + timer.incrementMs;
                    }
                    
                    // 切換到黑方
                    timer.currentPlayer = "Black";
                } else {
                    const timeToDeduct = timer.whiteIsA ? timer.timeB : timer.timeA;
                    const newTime = Math.max(0, timeToDeduct - elapsedMs);
                    
                    if(timer.whiteIsA){
                        timer.timeB = newTime + timer.incrementMs;
                    } else {
                        timer.timeA = newTime + timer.incrementMs;
                    }
                    
                    // 切換到白方
                    timer.currentPlayer = "White";
                }
                
                // 更新最後切換時間
                timer.lastSwitchTime = currentTime;
                
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
            if(rooms[roomId]){
                // 通知房間內其他玩家
                rooms[roomId].forEach(client => {
                    if(client !== ws && client.readyState === WebSocket.OPEN){
                        client.send(JSON.stringify({ action: "playerLeft", room: roomId }));
                    }
                });
                
                // 從房間移除離開的玩家
                rooms[roomId] = rooms[roomId].filter(c => c !== ws);
                if(rooms[roomId].length === 0){
                    delete rooms[roomId];
                    delete gameTimers[roomId];  // 清理計時器狀態
                }
            }
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
    });

    // 玩家斷線
    ws.on('close', () => {
        for(const roomId in rooms){
            // 通知房間內其他玩家有人斷線
            if(rooms[roomId] && rooms[roomId].includes(ws)){
                rooms[roomId].forEach(client => {
                    if(client !== ws && client.readyState === WebSocket.OPEN){
                        client.send(JSON.stringify({ action: "playerLeft", room: roomId }));
                    }
                });
            }
            
            rooms[roomId] = rooms[roomId].filter(c => c !== ws);
            if(rooms[roomId].length === 0){
                delete rooms[roomId];
                delete gameTimers[roomId];  // 清理計時器狀態
            }
        }
    });
});

console.log("WebSocket relay server running");
