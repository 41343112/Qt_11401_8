# Timer Synchronization Flow - Before and After Fix

## Before Fix (有問題的流程)

```
Player A (Host) makes move at server time 1000ms
↓
Server processes:
  - elapsed = 1000 - 0 = 1000ms
  - newTime = 600000 - 1000 + 5000 = 604000ms  ⚠️ 第一步就加增量！
  - lastSwitchTime = 1000ms
  - Sends to clients: {timeA: 604000, lastSwitchTime: 1000, ...}
↓
Client A receives at local time 1500ms (500ms network delay)
  - Adjusts: lastSwitchTime = 1500ms  ⚠️ 錯誤！覆蓋了伺服器時間
  - Stores: serverTimeA = 604000
↓
Client A timer tick at 1600ms:
  - elapsed = 1600 - 1500 = 100ms  ⚠️ 實際經過了 600ms！
  - display = 604000 - 100 = 603900ms
↓
Client B receives at local time 1550ms (different delay)
  - Adjusts: lastSwitchTime = 1550ms  ⚠️ 與 Client A 不同！
  - Stores: serverTimeA = 604000
↓
Client B timer tick at 1600ms:
  - elapsed = 1600 - 1550 = 50ms  ⚠️ 與 Client A 算出的不同！
  - display = 604000 - 50 = 603950ms
  
結果：兩個客戶端顯示不同的時間！ ❌
```

## After Fix (修復後的流程)

```
Player A (Host) makes move at server time 1000ms
↓
Server processes:
  - elapsed = 1000 - 0 = 1000ms (first move)
  - increment = 0  ✅ 第一步不加增量
  - newTime = 600000 - 1000 + 0 = 599000ms
  - lastSwitchTime = 1000ms
  - Sends to clients: {timeA: 599000, lastSwitchTime: 1000, ...}
↓
Client A receives at local time 1500ms (500ms network delay)
  - Keeps: lastSwitchTime = 1000ms  ✅ 保留伺服器時間
  - Stores: serverTimeA = 599000
  - Does NOT call applyIncrement()  ✅ 伺服器已處理
↓
Client A timer tick at 1600ms:
  - elapsed = 1600 - 1000 = 600ms  ✅ 正確！
  - display = 599000 - 600 = 598400ms
↓
Client B receives at local time 1550ms (different delay)
  - Keeps: lastSwitchTime = 1000ms  ✅ 相同的伺服器時間
  - Stores: serverTimeA = 599000
  - Does NOT call applyIncrement()  ✅ 伺服器已處理
↓
Client B timer tick at 1600ms:
  - elapsed = 1600 - 1000 = 600ms  ✅ 與 Client A 相同！
  - display = 599000 - 600 = 598400ms
  
結果：兩個客戶端顯示相同的時間！ ✅
```

## 第二步棋的增量應用

```
Player B makes move at server time 5000ms
↓
Server processes:
  - elapsed = 5000 - 1000 = 4000ms
  - isFirstMove = false
  - increment = 5000ms  ✅ 第二步開始加增量
  - newTime = 599000 - 4000 + 5000 = 600000ms
  - lastSwitchTime = 5000ms
  - Sends: {timeB: 600000, lastSwitchTime: 5000, ...}
↓
Clients receive and display:
  - Both use lastSwitchTime = 5000ms
  - Both calculate same elapsed time
  - Both show same display time
  
結果：時間同步 + 正確的增量 ✅
```

## 超時處理流程

```
Player A's time runs out
↓
Client detects timeout in updateTimeDisplaysFromServer():
  - timeA reaches 0
  - Checks: m_gameStarted == true  ✅ 防止重複觸發
↓
Proper game end sequence:
  1. setGameResult(GameResult::WhiteTimeout)  ✅ 設置結果
  2. handleGameEnd()  ✅ 統一處理
     - stopTimer()
     - Hide buttons
     - Show end game panels
     - Move UI elements
  3. showNonBlockingInfo()  ✅ 顯示訊息
  
結果：遊戲正確結束，無空窗期 ✅
```

## 關鍵修復點總結

| 問題 | 修復前 | 修復後 |
|------|--------|--------|
| 時間同步 | 調整 lastSwitchTime 為本地時間 | 使用伺服器的 lastSwitchTime |
| 客戶端看到的時間 | 不一致（依賴網路延遲） | 一致（同一個基準時間） |
| 第一步增量 | 錯誤地加了增量 | 正確地不加增量 |
| 增量應用次數 | 重複（伺服器 + 客戶端） | 只在伺服器端一次 |
| 超時處理 | 只停止計時器 | 完整遊戲結束流程 |
| 重複觸發保護 | 無 | 檢查 m_gameStarted |

## 代碼變更位置

### 客戶端 (qt_chess.cpp)
```cpp
// 1. onTimerStateReceived() - 第 7334 行
- m_serverLastSwitchTime = QDateTime::currentMSecsSinceEpoch();
+ m_serverLastSwitchTime = lastSwitchTime;

// 2. onSquareClicked() - 第 2571 行
- if (m_isOnlineGame && m_gameStartLocalTime > 0) {
+ if (m_isOnlineGame && m_gameStartLocalTime > 0 && !m_useServerTimer) {

// 3. applyIncrement() 調用 - 多處
- applyIncrement();
+ if (!m_useServerTimer) {
+     applyIncrement();
+ }

// 4. 超時處理 - 第 4356 行
- stopTimer();
- m_timerStarted = false;
- showTimeControlAfterTimeout();
+ m_chessBoard.setGameResult(GameResult::WhiteTimeout);
+ handleGameEnd();
```

### 伺服器端 (server.js)
```javascript
// Move 處理 - 第 293 行
- const newWhiteTime = Math.max(0, whiteTime - elapsedMs) + timer.incrementMs;
+ const increment = isFirstMove ? 0 : timer.incrementMs;
+ const newWhiteTime = Math.max(0, whiteTime - elapsedMs) + increment;
```

## 測試場景

1. **基本同步測試**
   - 兩個客戶端同時觀看
   - 驗證顯示時間相同

2. **網路延遲測試**
   - 模擬不同網路延遲
   - 驗證時間仍然同步

3. **增量測試**
   - 設置 5 秒增量
   - 第一步：不加增量
   - 第二步開始：每步 +5 秒

4. **超時測試**
   - 設置短時間限制
   - 等待超時
   - 驗證遊戲正確結束
