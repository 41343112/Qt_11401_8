# 線上計時器和每著加秒的 Bug 修復

## 問題摘要

本次修復解決了線上對戰模式中的時間同步、時間增量和超時處理的多個問題。

## 發現的問題

### 1. 時間同步問題 (Time Synchronization Issue)

**問題描述：**
在 `onTimerStateReceived()` 函數中，當收到伺服器的計時器狀態時，代碼會將 `lastSwitchTime` 調整為當前本地時間以"補償網路延遲"。然而這種做法實際上導致了時間去同步化。

**原始代碼：**
```cpp
if (lastSwitchTime > 0) {
    m_serverLastSwitchTime = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "[Qt_Chess::onTimerStateReceived] Adjusted lastSwitchTime to current time to compensate for network delay";
} else {
    m_serverLastSwitchTime = lastSwitchTime;
}
```

**問題分析：**
當將 `lastSwitchTime` 替換為當前時間時，會造成：
- 客戶端計算的經過時間（elapsed time）不正確
- 兩個客戶端看到的時間會不一致
- 實際經過的時間被錯誤地計算

**修復方案：**
直接使用伺服器發送的 `lastSwitchTime`，在計算經過時間時自然包含網路延遲。

```cpp
// 直接使用伺服器的 lastSwitchTime，不調整
// 客戶端在計算 elapsed 時會使用當前時間減去 lastSwitchTime
// 這樣可以正確計算經過的時間，包括網路延遲
m_serverLastSwitchTime = lastSwitchTime;
```

### 2. 時間增量重複應用 (Duplicate Increment Application)

**問題描述：**
在線上模式下，時間增量（increment）被重複應用：
1. 伺服器在處理移動時已經應用了增量（server.js 第 293, 308 行）
2. 客戶端在收到對手移動後又再次調用 `applyIncrement()`（qt_chess.cpp）
3. 客戶端在自己移動後也調用 `applyIncrement()`

**原始代碼：**
```cpp
// 在 onOpponentMove() 中
if (m_timeControlEnabled && m_timerStarted) {
    applyIncrement();  // 重複應用！
}

// 在 onSquareClicked() 和拖放處理中
} else {
    applyIncrement();  // 客戶端也應用！
}
```

**問題分析：**
使用伺服器計時器時（`m_useServerTimer = true`），所有時間計算都應該由伺服器控制。客戶端不應該修改時間值，只應該顯示伺服器提供的值。

**修復方案：**
在所有調用 `applyIncrement()` 的地方添加檢查，只在非伺服器計時器模式下應用：

```cpp
// 只在非伺服器計時器模式下應用增量
// 使用伺服器計時器時，增量已在伺服器端計算
if (!m_useServerTimer) {
    applyIncrement();
}
```

### 3. 第一步棋的增量問題 (First Move Increment Issue)

**問題描述：**
在伺服器端，第一步棋（isFirstMove = true）時，elapsedMs 為 0，但仍然添加了增量：

```javascript
const newWhiteTime = Math.max(0, whiteTime - elapsedMs) + timer.incrementMs;
// 當 elapsedMs = 0 時，實際上是 whiteTime + incrementMs
```

**問題分析：**
第一步棋不應該獲得時間增量，因為玩家還沒有消耗任何時間。這會給第一個移動的玩家不公平的額外時間。

**修復方案：**
在伺服器端，只在非第一步時添加增量：

```javascript
// 只在非第一步時添加增量
const increment = isFirstMove ? 0 : timer.incrementMs;
const newWhiteTime = Math.max(0, whiteTime - elapsedMs) + increment;
```

### 4. 超時處理的空窗期 (Timeout Detection Gap)

**問題描述：**
當使用伺服器計時器時，超時檢測存在以下問題：
1. `updateTimeDisplaysFromServer()` 檢測到超時後，只調用 `stopTimer()` 和顯示訊息
2. 沒有設置遊戲結果（GameResult）
3. 沒有調用 `handleGameEnd()` 完整處理遊戲結束邏輯
4. 可能重複觸發超時檢測

**原始代碼：**
```cpp
if (m_whiteTimeMs <= 0 && m_timeControlEnabled && m_whiteInitialTimeMs > 0) {
    stopTimer();
    m_timerStarted = false;
    showTimeControlAfterTimeout();
    showNonBlockingInfo("時間到", "白方超時！黑方獲勝！");
}
```

**問題分析：**
- 遊戲狀態沒有被正確標記為結束
- UI 元素（按鈕、面板）沒有被正確更新
- 可能在多次 timer tick 中重複觸發

**修復方案：**
1. 設置正確的遊戲結果
2. 調用 `handleGameEnd()` 統一處理遊戲結束
3. 添加 `m_gameStarted` 檢查防止重複觸發

```cpp
// 需要檢查 m_gameStarted 以避免重複觸發
if (m_whiteTimeMs <= 0 && m_timeControlEnabled && m_whiteInitialTimeMs > 0 && m_gameStarted) {
    m_chessBoard.setGameResult(GameResult::WhiteTimeout);
    handleGameEnd();
    showNonBlockingInfo("時間到", "白方超時！黑方獲勝！");
}
```

## 修改的檔案

### 客戶端 (Client Side)
- `src/qt_chess.cpp`
  - `onTimerStateReceived()`: 修復時間同步
  - `onSquareClicked()`: 添加伺服器計時器檢查
  - `mouseReleaseEvent()`: 添加伺服器計時器檢查（拖放）
  - `onEngineBestMove()`: 添加伺服器計時器檢查（AI 移動）
  - `onOpponentMove()`: 添加伺服器計時器檢查
  - `updateTimeDisplaysFromServer()`: 修復超時處理
  - `onGameTimerTick()`: 修復超時處理

### 伺服器端 (Server Side)
- `server.js`
  - Move 處理邏輯：修復第一步棋不應該獲得增量的問題

## 測試建議

### 1. 時間同步測試
- 創建一個線上房間，兩個客戶端加入
- 觀察兩個客戶端顯示的時間是否同步
- 移動棋子後，檢查時間是否一致減少

### 2. 時間增量測試
- 設置時間增量（例如：5 秒）
- 進行多步移動
- 驗證每步後時間是否正確增加
- 檢查第一步棋是否沒有獲得增量

### 3. 超時測試
- 設置較短的時間限制（例如：10 秒）
- 等待一方時間耗盡
- 驗證遊戲是否正確結束
- 檢查 UI 是否正確更新

### 4. 邊界情況測試
- 測試無限時間模式（時間設為 0）
- 測試只有一方設置時間限制
- 測試在最後一秒移動
- 測試網路延遲情況

## 技術細節

### 時間計算原理

使用伺服器計時器模式時：
1. 伺服器維護權威的計時器狀態
2. 每次移動時，伺服器計算：
   - 經過的時間：`currentTime - lastSwitchTime`
   - 新的時間：`oldTime - elapsed + increment`（第一步不加增量）
3. 客戶端接收伺服器狀態並計算顯示：
   - 經過時間：`currentLocalTime - lastSwitchTime`
   - 顯示時間：`serverTime - elapsed`

### 為什麼不調整 lastSwitchTime

如果將 `lastSwitchTime` 調整為當前本地時間：
```
Server: lastSwitchTime = 1000000 (server time)
Client receives message at local time 1000500 (500ms delay)
Client sets lastSwitchTime = 1000500

Next tick at 1000600:
elapsed = 1000600 - 1000500 = 100ms  ❌ 錯誤！實際經過了 600ms
```

如果直接使用 server 的 lastSwitchTime：
```
Server: lastSwitchTime = 1000000 (server time)
Client receives and uses 1000000

Next tick at client time 1000600 (= server time 1000100 approximately)
elapsed = 1000600 - 1000000 = 600ms  ✅ 正確！
```

## 影響範圍

這些修復影響：
- ✅ 線上對戰模式的計時器
- ✅ 時間增量功能
- ✅ 超時檢測和遊戲結束處理
- ❌ 不影響本地遊戲模式（AI 或雙人本地對戰）
- ❌ 不影響無時間限制的遊戲

## 向後兼容性

所有修改都是向後兼容的：
- 本地遊戲模式不受影響（`!m_useServerTimer` 時使用原有邏輯）
- 伺服器更新對舊客戶端仍然兼容（只是時間計算更準確）
- 新客戶端對舊伺服器也能正常工作

## 結論

這些修復解決了線上計時器的核心問題：
1. **時間同步**：客戶端正確使用伺服器時間
2. **增量邏輯**：避免重複應用增量，第一步不加增量
3. **超時處理**：正確檢測超時並結束遊戲
4. **一致性**：伺服器作為唯一真實來源

修復後，線上對戰的計時器應該能夠正確、準確、同步地工作。
