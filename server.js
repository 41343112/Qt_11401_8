const WebSocket = require('ws');

// 使用 Render 自動提供的 PORT
const wss = new WebSocket.Server({ port: process.env.PORT || 3000 });

// 房間資料
const rooms = {}; // rooms[roomId] = [ws1, ws2, ...]

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

        // 廣播落子訊息
        else if(msg.action === "move"){
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
            rooms[roomId] = rooms[roomId].filter(c => c !== ws);
            if(rooms[roomId].length === 0) delete rooms[roomId];
        }
    });
});

console.log("WebSocket relay server running");
