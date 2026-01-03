# 安全與錯誤審查報告 (Security and Bug Audit Report)

生成日期：2026-01-03

## 執行摘要 (Executive Summary)

本報告詳細記錄了對 Qt_11401_8 西洋棋專案的全面安全與錯誤審查結果。審查涵蓋了 C++ 客戶端代碼和 Node.js 伺服器代碼。

### 發現的問題統計
- 🔴 **嚴重 (Critical)**: 2 個
- 🟠 **高危 (High)**: 3 個
- 🟡 **中危 (Medium)**: 4 個
- 🔵 **低危 (Low)**: 3 個

---

## 🔴 嚴重問題 (Critical Issues)

### 1. 陣列越界訪問 - 缺少邊界檢查
**文件**: `src/chessboard.cpp`  
**位置**: 行 57-63  
**嚴重性**: 🔴 Critical

#### 問題描述
`getPiece()` 函數未進行邊界檢查，直接訪問陣列元素。

```cpp
const ChessPiece& ChessBoard::getPiece(int row, int col) const {
    return m_board[row][col];  // ❌ 無邊界檢查
}

ChessPiece& ChessBoard::getPiece(int row, int col) {
    return m_board[row][col];  // ❌ 無邊界檢查
}
```

#### 影響
- 可能導致記憶體越界訪問
- 未定義行為 (Undefined Behavior)
- 潛在的崩潰或安全漏洞
- 程式可能讀取或寫入無效記憶體

#### 建議修復
```cpp
const ChessPiece& ChessBoard::getPiece(int row, int col) const {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        static const ChessPiece empty(PieceType::None, PieceColor::None);
        return empty;
    }
    return m_board[row][col];
}
```

---

### 2. JSON 解析無異常處理
**文件**: `server.js`  
**位置**: 行 64  
**嚴重性**: 🔴 Critical

#### 問題描述
WebSocket 訊息解析沒有 try-catch 包裝，可能導致伺服器崩潰。

```javascript
ws.on('message', message => {
    const msg = JSON.parse(message);  // ❌ 可能拋出異常
    // ...
});
```

#### 影響
- 惡意或格式錯誤的訊息可導致伺服器崩潰
- 拒絕服務攻擊 (DoS) 風險
- 影響所有連接的玩家

#### 建議修復
```javascript
ws.on('message', message => {
    try {
        const msg = JSON.parse(message);
        // 處理訊息...
    } catch (error) {
        console.error('[Server] JSON parse error:', error);
        ws.send(JSON.stringify({ action: "error", message: "Invalid message format" }));
        return;
    }
});
```

---

## 🟠 高危問題 (High Severity Issues)

### 3. WebSocket 訊息未驗證
**文件**: `server.js`  
**位置**: 多處  
**嚴重性**: 🟠 High

#### 問題描述
伺服器未驗證接收到的訊息內容的有效性和完整性。

```javascript
if(msg.action === "move"){
    const roomId = msg.room;
    // ❌ 未檢查 fromRow, fromCol, toRow, toCol 是否存在或有效
    // ❌ 未檢查房間是否存在
    // ❌ 未檢查發送者是否在該房間
}
```

#### 影響
- 玩家可能發送無效的棋步
- 可能導致遊戲狀態不一致
- 潛在的作弊可能

#### 建議修復
添加輸入驗證：
```javascript
if(msg.action === "move"){
    const roomId = msg.room;
    
    // 驗證房間存在且發送者在房間內
    if(!rooms[roomId] || !rooms[roomId].includes(ws)) {
        ws.send(JSON.stringify({ action: "error", message: "Invalid room" }));
        return;
    }
    
    // 驗證移動數據
    if(typeof msg.fromRow !== 'number' || typeof msg.fromCol !== 'number' ||
       typeof msg.toRow !== 'number' || typeof msg.toCol !== 'number') {
        ws.send(JSON.stringify({ action: "error", message: "Invalid move data" }));
        return;
    }
    
    // 驗證範圍
    if(msg.fromRow < 0 || msg.fromRow >= 8 || msg.fromCol < 0 || msg.fromCol >= 8 ||
       msg.toRow < 0 || msg.toRow >= 8 || msg.toCol < 0 || msg.toCol >= 8) {
        ws.send(JSON.stringify({ action: "error", message: "Move out of bounds" }));
        return;
    }
    
    // 繼續處理...
}
```

---

### 4. 記憶體管理 - 潛在的雙重釋放
**文件**: `src/chessengine.cpp`  
**位置**: 行 54, 79  
**嚴重性**: 🟠 High

#### 問題描述
在錯誤處理路徑中，`m_process` 可能被多次刪除。

```cpp
if (!m_process->waitForStarted(5000)) {
    emit engineError(QString("無法啟動引擎：%1").arg(m_process->errorString()));
    delete m_process;  // ❌ 刪除但未設為 nullptr
    m_process = nullptr;
    return false;
}
```

雖然立即設為 nullptr，但在析構函數中可能再次訪問：

```cpp
void ChessEngine::stopEngine() {
    if (m_process) {
        // ...
        delete m_process;  // 可能重複刪除（雖然這裡有檢查）
        m_process = nullptr;
    }
}
```

#### 影響
- 雙重釋放可能導致崩潰
- 記憶體損壞

#### 建議
當前代碼實際上是安全的，因為有 nullptr 檢查，但建議使用智慧指標：

```cpp
// 在標頭檔中
std::unique_ptr<QProcess> m_process;

// 使用時
m_process = std::make_unique<QProcess>();
// 無需手動 delete，自動管理
```

---

### 5. 計時器同步問題
**文件**: `server.js`  
**位置**: 行 173-179  
**嚴重性**: 🟠 High

#### 問題描述
計時器計算使用整數除法，可能導致精度損失。

```javascript
const currentTime = Math.floor(Date.now() / 1000);  // UNIX 秒數
```

#### 影響
- 計時器精度僅為秒級
- 快速移動時可能出現時間計算錯誤
- 在高頻率移動場景下累積誤差

#### 建議
保持毫秒精度：
```javascript
const currentTime = Date.now();  // 保持毫秒
```

---

## 🟡 中危問題 (Medium Severity Issues)

### 6. 缺少房間大小限制
**文件**: `server.js`  
**位置**: 行 78-82  
**嚴重性**: 🟡 Medium

#### 問題描述
房間可以無限制地加入玩家。

```javascript
if(rooms[roomId]){
    rooms[roomId].push(ws);  // ❌ 無大小檢查
}
```

#### 影響
- 多於 2 名玩家可能加入同一房間
- 遊戲邏輯僅支援 2 人遊戲
- 可能導致混亂的遊戲狀態

#### 建議修復
```javascript
if(rooms[roomId]){
    if(rooms[roomId].length >= 2){
        ws.send(JSON.stringify({ action: "error", message: "房間已滿" }));
        return;
    }
    rooms[roomId].push(ws);
    // ...
}
```

---

### 7. 缺少速率限制
**文件**: `server.js`  
**位置**: 全局  
**嚴重性**: 🟡 Medium

#### 問題描述
沒有實施速率限制，玩家可以快速發送大量訊息。

#### 影響
- 拒絕服務攻擊風險
- 伺服器資源耗盡
- 影響其他玩家體驗

#### 建議修復
實施簡單的速率限制：
```javascript
const rateLimits = new Map(); // ws -> { count, resetTime }

function checkRateLimit(ws) {
    const now = Date.now();
    const limit = rateLimits.get(ws) || { count: 0, resetTime: now + 1000 };
    
    if(now > limit.resetTime) {
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
```

---

### 8. Qt 物件父子關係管理
**文件**: `src/qt_chess.cpp`, `src/boardcolorsettingsdialog.cpp`  
**位置**: 多處  
**嚴重性**: 🟡 Medium

#### 問題描述
許多 Qt 物件使用 `new` 分配但依賴父物件管理，如果父物件關係不正確可能洩漏。

```cpp
QLabel* label = new QLabel(this);  // 依賴父物件 'this' 管理
```

#### 影響
- 如果父物件關係設置錯誤，可能導致記憶體洩漏
- 但 Qt 的父子系統通常會正確處理

#### 建議
當前實現是 Qt 標準做法，無需修改。但要確保所有 Qt 物件都有適當的父物件。

---

### 9. 網路錯誤處理不完整
**文件**: `src/networkmanager.cpp`  
**位置**: 行 422  
**嚴重性**: 🟡 Medium

#### 問題描述
網路錯誤後可能導致程式處於不一致狀態。

```cpp
void NetworkManager::onError(QAbstractSocket::SocketError socketError)
{
    QString errorString = m_webSocket ? m_webSocket->errorString() : "Unknown error";
    qDebug() << "[NetworkManager] Socket error:" << errorString;
    m_status = ConnectionStatus::Error;
    emit connectionError(errorString);
}
```

#### 影響
- WebSocket 可能仍保持部分連接狀態
- 房間號和角色未清理

#### 建議修復
```cpp
void NetworkManager::onError(QAbstractSocket::SocketError socketError)
{
    QString errorString = m_webSocket ? m_webSocket->errorString() : "Unknown error";
    qDebug() << "[NetworkManager] Socket error:" << errorString;
    
    // 清理狀態
    m_status = ConnectionStatus::Error;
    m_roomNumber.clear();
    m_role = NetworkRole::None;
    
    emit connectionError(errorString);
}
```

---

## 🔵 低危問題 (Low Severity Issues)

### 10. 調試輸出包含敏感資訊
**文件**: 多個 `.cpp` 文件  
**位置**: 多處  
**嚴重性**: 🔵 Low

#### 問題描述
qDebug() 輸出可能包含遊戲狀態和房間資訊。

```cpp
qDebug() << "[NetworkManager] Connecting to server:" << m_serverUrl << "to join room:" << roomNumber;
```

#### 影響
- 在發佈版本中暴露內部狀態
- 可能洩漏房間號（雖然是臨時的）

#### 建議
在發佈版本中禁用調試輸出：
```cpp
#ifndef QT_NO_DEBUG
    qDebug() << "[NetworkManager] Connecting to server:" << m_serverUrl;
#endif
```

---

### 11. 房間號碰撞風險
**文件**: `server.js`  
**位置**: 行 18-20  
**嚴重性**: 🔵 Low

#### 問題描述
4 位數房號可能碰撞（雖然有檢查）。

```javascript
function generateRoomId() {
    return Math.floor(1000 + Math.random() * 9000).toString();
}
```

#### 影響
- 9000 個可能的房號
- 大量並發房間時可能碰撞
- 雖然有 do-while 檢查，但效率可能降低

#### 建議
增加房號長度或使用 UUID：
```javascript
function generateRoomId() {
    // 6 位數：900,000 個可能性
    return Math.floor(100000 + Math.random() * 900000).toString();
}
```

---

### 12. 缺少輸入清理
**文件**: `src/qt_chess.cpp` (間接通過網路)  
**位置**: 多處  
**嚴重性**: 🔵 Low

#### 問題描述
從網路接收的資料未經充分清理就用於邏輯判斷。

#### 影響
- 格式錯誤的資料可能導致邏輯錯誤
- 客戶端相對受信任（對等連接），風險較低

#### 建議
添加額外驗證層。

---

## 正面發現 (Positive Findings)

### ✅ 良好實踐
1. **無使用不安全的 C 函數**：未發現 `strcpy`, `strcat`, `sprintf`, `gets` 等
2. **使用 Qt 的記憶體管理**：大部分使用 Qt 父子物件系統
3. **使用 std::vector**：而非原始指標陣列
4. **清理資源**：析構函數中正確清理資源
5. **使用 QWebSocket**：而非原始 socket，提供更好的安全性

---

## 優先修復建議

### 立即修復 (Critical)
1. ✅ 在 `getPiece()` 中添加邊界檢查
2. ✅ 在 `server.js` 中添加 JSON.parse 異常處理

### 短期修復 (High)
3. 添加 WebSocket 訊息驗證
4. 實施房間大小限制
5. 添加基本速率限制

### 中期改進 (Medium)
6. 改進錯誤處理和狀態清理
7. 考慮使用智慧指標
8. 增加房號長度

### 長期改進 (Low)
9. 在發佈版本中禁用調試輸出
10. 添加更完整的輸入驗證

---

## 測試建議

### 安全測試
1. **模糊測試**：發送格式錯誤的 JSON 到伺服器
2. **邊界測試**：使用超出範圍的棋盤座標
3. **負載測試**：模擬多個並發連接
4. **記憶體測試**：使用 Valgrind 檢查記憶體洩漏

### 功能測試
1. 測試異常斷線場景
2. 測試快速連續移動
3. 測試房間管理（創建、加入、離開）
4. 測試計時器精度

---

## 總結

本專案整體代碼質量良好，使用了現代 C++ 和 Qt 最佳實踐。發現的主要問題集中在：
- 輸入驗證和邊界檢查
- 伺服器端異常處理
- 網路資料驗證

建議優先修復標記為 Critical 和 High 的問題，以提高程式的穩定性和安全性。

---

**審查人員**: AI Security Audit System  
**審查日期**: 2026-01-03  
**版本**: 1.0
