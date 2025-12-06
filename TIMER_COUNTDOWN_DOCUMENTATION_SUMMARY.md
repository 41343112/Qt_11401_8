# 時間倒數邏輯文檔化總結 (Timer Countdown Logic Documentation Summary)

## 任務概述 (Task Overview)

本次任務為 Qt_Chess 專案新增了詳細的時間倒數邏輯說明文檔，解釋雙人模式和電腦模式的計時器行為。

## 完成的工作 (Work Completed)

### 1. 程式碼分析 (Code Analysis)

深入分析了以下關鍵函數和機制：
- `onGameTimerTick()` - 主計時器更新邏輯
- `startTimer()` / `stopTimer()` - 計時器控制
- `applyIncrement()` - 時間增量應用
- `isComputerTurn()` - 電腦回合判斷
- `onEngineBestMove()` - 電腦移動處理

### 2. 新增文檔 (New Documentation)

創建了 `docs/TIMER_COUNTDOWN_LOGIC.md`，包含以下章節：

#### 主要內容 (Main Content)

1. **計時器架構** (Timer Architecture)
   - 核心組件說明
   - 計時器初始化過程

2. **雙人模式時間倒數邏輯** (Two-Player Mode Timer Logic)
   - 啟動條件
   - 倒數機制（每 100ms 減少 100ms）
   - 時間增量應用
   - 超時處理

3. **電腦模式時間倒數邏輯** (Computer Mode Timer Logic)
   - 模式說明（Human vs Computer / Computer vs Human）
   - 時間倒數行為
   - 計時器啟動
   - 電腦思考時間
   - 與雙人模式的一致性對比

4. **線上模式時間倒數邏輯** (Online Mode Timer Logic)
   - 伺服器同步機制
   - 伺服器控制計時器

5. **計時器控制方法** (Timer Control Methods)
   - `startTimer()`
   - `stopTimer()`
   - `applyIncrement()`

6. **回放模式中的計時器** (Timer in Replay Mode)
   - 計時器暫停
   - 計時器恢復

7. **時間顯示更新** (Time Display Updates)
   - `updateTimeDisplays()`
   - 視覺提示（綠色高亮、進度條）

8. **時間設定持久化** (Time Settings Persistence)
   - 儲存和載入設定

9. **特殊情況處理** (Special Case Handling)
   - 無限時間模式
   - 新遊戲重置
   - 投降處理

10. **總結** (Summary)
    - 雙人模式和電腦模式的共同點
    - 關鍵差異
    - 設計原則

### 3. 更新 README (README Update)

在 README.md 的「時間控制」功能區塊中新增了對新文檔的引用：
```markdown
- 參閱 [TIMER_COUNTDOWN_LOGIC.md](docs/TIMER_COUNTDOWN_LOGIC.md) 以了解雙人模式和電腦模式的時間倒數邏輯
```

## 關鍵發現 (Key Findings)

### 雙人模式和電腦模式的計時器行為一致性 (Timer Behavior Consistency)

**重要發現：雙人模式和電腦模式使用完全相同的計時邏輯。**

#### 共同特點：

1. **倒數速率**：兩種模式都以每 100ms 減少 100ms 的速率進行倒數
2. **時間增量**：每次移動後，剛完成移動的玩家（人類或電腦）都會獲得相同的時間增量
3. **超時處理**：時間耗盡時，不論是人類還是電腦，都會立即判負
4. **計時器啟動**：都在第一步棋走出後自動啟動

#### 唯一差異：

差異僅在於誰操作棋子：
- **雙人模式**：兩位人類玩家輪流操作
- **電腦模式**：一位人類玩家與 AI 引擎輪流操作

但從計時器系統的角度看，電腦和人類完全平等，都受相同的時間約束。

### 電腦思考時間計入倒數 (Computer Thinking Time Counts Down)

電腦在思考時，其時間也會持續倒數：
- 電腦沒有「暫停」或「思考時間不計」的特權
- 從輪到電腦開始，計時器立即開始倒數
- 電腦必須在時間耗盡前完成移動
- 如果電腦超時，人類玩家獲勝

這種設計確保了遊戲的公平性。

## 技術細節 (Technical Details)

### 計時器更新頻率 (Timer Update Frequency)

```cpp
m_gameTimer->start(100); // 每 100ms 觸發一次
```

- 更新頻率：100 毫秒
- 每次減少：100 毫秒
- 提供平滑的倒數視覺效果

### 時間增量機制 (Time Increment Mechanism)

```cpp
void Qt_Chess::applyIncrement() {
    // 為剛完成移動的玩家添加增量
    PieceColor playerWhoMoved = (m_chessBoard.getCurrentPlayer() == PieceColor::White)
                                    ? PieceColor::Black : PieceColor::White;
    
    if (playerWhoMoved == PieceColor::White) {
        m_whiteTimeMs += m_incrementMs;
    } else {
        m_blackTimeMs += m_incrementMs;
    }
}
```

時機：在玩家成功完成移動後立即調用，在切換到對手之前執行。

### 線上模式特殊處理 (Online Mode Special Handling)

線上模式使用伺服器同步時間，確保兩位遠端玩家看到一致的經過時間：

```cpp
if (m_isOnlineGame && m_gameStartLocalTime > 0) {
    qint64 currentSyncTime = QDateTime::currentMSecsSinceEpoch() + m_serverTimeOffset;
    qint64 turnElapsedMs = currentSyncTime - m_currentTurnStartTime;
    int newTime = m_initialTimeMs - static_cast<int>(turnElapsedMs);
    // 更新時間...
}
```

## 文檔特色 (Documentation Features)

1. **中英雙語**：所有標題和重要術語都提供中英文對照
2. **程式碼範例**：包含實際源碼片段，方便理解
3. **表格對比**：使用表格清晰對比不同模式的特性
4. **完整性**：涵蓋所有相關場景和邊界情況
5. **技術準確性**：基於實際源碼分析，確保準確性

## 設計原則驗證 (Design Principles Verification)

文檔驗證了系統設計遵循以下原則：

1. **公平性** (Fairness)：所有玩家（包括電腦）都受相同的時間約束
2. **一致性** (Consistency)：不同模式使用相同的計時邏輯
3. **準確性** (Accuracy)：100ms 的更新頻率提供平滑體驗
4. **可擴展性** (Extensibility)：支援線上模式的伺服器同步擴展

## 參考文件 (Reference Files)

### 新增文件 (New Files)
- `docs/TIMER_COUNTDOWN_LOGIC.md` - 時間倒數邏輯詳細說明

### 修改文件 (Modified Files)
- `README.md` - 新增文檔引用

### 相關源碼 (Related Source Code)
- `src/qt_chess.cpp` - 計時器實現
- `src/qt_chess.h` - 計時器成員變數定義

### 相關文檔 (Related Documentation)
- `docs/TIME_CONTROL_FEATURE.md` - 時間控制功能說明

## 結論 (Conclusion)

本次文檔化工作成功地說明了 Qt_Chess 應用程式中雙人模式和電腦模式的時間倒數邏輯。

關鍵發現是：**兩種模式使用完全相同的計時邏輯**，唯一差異僅在於操作者（人類 vs 電腦），但計時器系統對所有玩家一視同仁，確保了遊戲的公平性和一致性。

文檔提供了詳細的技術說明、程式碼範例和設計原則分析，為理解和維護時間控制系統提供了完整的參考資料。
