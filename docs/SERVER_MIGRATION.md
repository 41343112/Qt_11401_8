# 中央伺服器遷移說明

## 概述

本次更新將線上對戰功能從 P2P（點對點）連線模式遷移到中央伺服器模式，使用位於 `https://chess-server-mjg6.onrender.com` 的 WebSocket 伺服器。

## 主要變更

### 1. 網路架構變更

**舊版（P2P模式）：**
- 使用 QTcpServer/QTcpSocket
- 房主在本機開啟TCP伺服器監聽端口
- 房客直接連接到房主的IP地址和端口
- 需要在同一區域網路或設定端口轉發

**新版（中央伺服器模式）：**
- 使用 QWebSocket
- 雙方都連接到中央 WebSocket 伺服器
- 伺服器負責房間管理和訊息轉發
- 無需設定網路，可跨網路連線

### 2. 連線方式簡化

**舊版：**
- 需要IP地址和房號（4位數字）
- 連線碼格式：`192.168.1.100:1234`
- 房號由本地生成（1000-9999）
- 端口號 = 10000 + 房號

**新版：**
- 只需要房號（由伺服器生成）
- 房號格式：`ABCD1234`（字母數字組合）
- 無需知道IP地址
- 無需計算端口號

### 3. 程式碼變更

#### NetworkManager.h
- 移除 `QTcpServer* m_server`
- 移除 `QTcpSocket* m_socket`
- 移除 `QTcpSocket* m_clientSocket`
- 移除 `quint16 m_port`
- 移除 `QByteArray m_receiveBuffer`
- 新增 `QWebSocket* m_webSocket`
- 新增 `QString m_serverUrl`
- 更新 `NetworkRole` 從 `Server/Client` 改為 `Host/Guest`

#### NetworkManager.cpp
- `createRoom()`: 不再建立本地伺服器，改為連接WebSocket並發送CreateRoom請求
- `joinRoom()`: 參數從 `(hostAddress, port)` 改為 `(roomNumber)`
- `onConnected()`: WebSocket連接成功後發送CreateRoom或JoinRoom請求
- `onTextMessageReceived()`: 處理WebSocket文字訊息
- 新增處理 `MessageType::CreateRoom`, `MessageType::RoomCreated`, `MessageType::JoinRejected`
- 移除 `onNewConnection()`, `onReadyRead()`, `onServerError()`, `getActiveSocket()`, `generateRoomNumber()`

#### OnlineDialog
- 移除IP地址輸入欄位
- 簡化UI，只保留房號輸入
- 更新說明文字，強調使用中央伺服器的優勢
- `getRoomNumber()`: 返回房號字串
- 移除 `getHostAddress()` 和 `getPort()`

#### Qt_Chess
- `onRoomCreated()`: 參數從 `(roomNumber, port)` 改為 `(roomNumber)`
- `showRoomInfoDialog()`: 只顯示房號，移除IP地址顯示
- 房間資訊標籤顯示伺服器名稱而非IP

## 伺服器通訊協定

### 訊息格式
所有訊息使用 JSON 格式，透過 WebSocket 傳輸。

### 訊息類型

#### 1. CreateRoom（創建房間）
客戶端 → 伺服器
```json
{
  "type": "CreateRoom"
}
```

#### 2. RoomCreated（房間已創建）
伺服器 → 客戶端
```json
{
  "type": "RoomCreated",
  "roomNumber": "ABCD1234"
}
```

#### 3. JoinRoom（加入房間）
客戶端 → 伺服器
```json
{
  "type": "JoinRoom",
  "roomNumber": "ABCD1234"
}
```

#### 4. JoinAccepted（加入成功）
伺服器 → 客戶端
```json
{
  "type": "JoinAccepted"
}
```

#### 5. JoinRejected（加入失敗）
伺服器 → 客戶端
```json
{
  "type": "JoinRejected",
  "reason": "房間不存在"
}
```

#### 6. Move（棋步移動）
客戶端 → 伺服器 → 對手
```json
{
  "type": "Move",
  "roomNumber": "ABCD1234",
  "fromRow": 6,
  "fromCol": 4,
  "toRow": 4,
  "toCol": 4,
  "promotion": 5
}
```

#### 7. 其他訊息
所有遊戲訊息（GameStart, StartGame, TimeSettings, Surrender, Chat, PlayerDisconnected等）都需包含 `roomNumber` 欄位，以便伺服器轉發到正確的房間。

## 依賴項變更

### Qt_Chess.pro
新增 WebSocket 模組：
```qmake
QT += core gui multimedia multimediawidgets network websockets
```

## 使用者體驗改善

### 優點
1. **更簡單**：只需要房號，不需要IP地址
2. **更方便**：不需要設定路由器或防火牆
3. **更廣泛**：可以跨網路、跨地區連線
4. **更穩定**：專業伺服器處理連線
5. **更安全**：中央伺服器統一管理

### 缺點
1. **依賴伺服器**：需要伺服器正常運作
2. **網路延遲**：可能略高於區域網路
3. **隱私考量**：遊戲資料經過中央伺服器

## 測試建議

### 基本功能測試
1. 創建房間
   - 驗證WebSocket連接成功
   - 驗證收到房號
   - 驗證房號可複製

2. 加入房間
   - 驗證可使用房號加入
   - 驗證房號錯誤時顯示錯誤訊息
   - 驗證連接成功後遊戲開始

3. 遊戲進行
   - 驗證棋步正確同步
   - 驗證時間控制正常運作
   - 驗證投降、認輸等功能

4. 斷線處理
   - 驗證對手斷線時顯示通知
   - 驗證自己斷線後可重連

### 效能測試
1. 延遲測試：測量棋步同步延遲
2. 穩定性測試：長時間遊戲不斷線
3. 併發測試：多個房間同時運作

## 未來改進方向

1. **重連機制**：支援斷線後自動重連
2. **房間列表**：顯示可用房間供選擇
3. **觀戰模式**：允許觀眾觀看對局
4. **聊天功能**：完善玩家間通訊
5. **排名系統**：記錄玩家戰績
6. **匹配系統**：自動配對玩家

## 技術備註

### 伺服器要求
- 必須支援 WebSocket (WSS)
- 必須能處理 JSON 訊息
- 必須能管理房間和轉發訊息
- 建議使用 HTTPS/WSS 加密連線

### 安全考量
- 建議實作訊息驗證避免作弊
- 建議實作速率限制避免濫用
- 建議實作身份驗證（未來版本）
- 建議加密敏感資料（未來版本）

## 版本相容性

此更新**不向下相容**舊版P2P模式。使用新版本的玩家只能與使用新版本的玩家對弈。

建議所有使用者更新到最新版本以享受新功能。

## 參考資料

- WebSocket RFC: https://tools.ietf.org/html/rfc6455
- Qt WebSocket: https://doc.qt.io/qt-5/qwebsocket.html
- JSON 格式: https://www.json.org/

---

**更新日期**: 2025-12-05  
**伺服器地址**: wss://chess-server-mjg6.onrender.com
