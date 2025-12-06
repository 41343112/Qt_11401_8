# 時間倒數邏輯說明 (Timer Countdown Logic)

## 概述 (Overview)

本文檔詳細說明 Qt_Chess 應用程式中雙人模式和電腦模式的時間倒數邏輯。時間控制系統為所有遊戲模式提供一致的計時機制，確保公平的遊戲體驗。

## 計時器架構 (Timer Architecture)

### 核心組件 (Core Components)

時間控制系統由以下核心組件組成：

- **`QTimer* m_gameTimer`**: 主遊戲計時器，每 100 毫秒觸發一次
- **`int m_whiteTimeMs`**: 白方剩餘時間（毫秒）
- **`int m_blackTimeMs`**: 黑方剩餘時間（毫秒）
- **`int m_incrementMs`**: 每步棋的時間增量（毫秒）
- **`bool m_timeControlEnabled`**: 時間控制是否啟用
- **`bool m_timerStarted`**: 計時器是否已啟動

### 計時器初始化 (Timer Initialization)

```cpp
// 創建計時器實例（setupTimeControlUI 函數中）
m_gameTimer = new QTimer(this);
connect(m_gameTimer, &QTimer::timeout, this, &Qt_Chess::onGameTimerTick);
```

計時器每 100 毫秒觸發一次，提供平滑的倒數效果。

## 雙人模式時間倒數邏輯 (Two-Player Mode Timer Logic)

### 啟動條件 (Start Conditions)

在雙人模式（Human vs Human）中，計時器遵循以下啟動順序：

1. **遊戲開始**: 玩家點擊「開始」按鈕後，時間設定被初始化
2. **首次移動**: 計時器在第一步棋走出後自動啟動
3. **手動標記**: 使用 `m_timerStarted = true` 標記計時器已啟動

```cpp
// onSquareClicked 函數中的計時器啟動邏輯
bool isFirstMove = m_uciMoveHistory.isEmpty();
if (isFirstMove && m_timeControlEnabled && !m_timerStarted) {
    m_timerStarted = true;
    m_gameStartLocalTime = QDateTime::currentMSecsSinceEpoch();
    m_currentTurnStartTime = m_gameStartLocalTime;
    startTimer();
}
```

### 倒數機制 (Countdown Mechanism)

雙人模式使用簡單的遞減倒數邏輯：

```cpp
// onGameTimerTick 函數 - 雙人模式部分
PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
if (currentPlayer == PieceColor::White) {
    if (m_whiteTimeMs > 0) {
        m_whiteTimeMs -= 100;  // 每次減少 100 毫秒
        if (m_whiteTimeMs <= 0) {
            m_whiteTimeMs = 0;
            updateTimeDisplays();
            stopTimer();
            m_timerStarted = false;
            QMessageBox::information(this, "時間到", "白方超時！黑方獲勝！");
            return;
        }
    }
} else {
    if (m_blackTimeMs > 0) {
        m_blackTimeMs -= 100;  // 每次減少 100 毫秒
        if (m_blackTimeMs <= 0) {
            m_blackTimeMs = 0;
            updateTimeDisplays();
            stopTimer();
            m_timerStarted = false;
            QMessageBox::information(this, "時間到", "黑方超時！白方獲勝！");
            return;
        }
    }
}
```

**關鍵特點：**
- 每個計時器週期（100ms）減少當前玩家的時間
- 只有輪到的玩家的時間會倒數
- 對手的時間保持不變
- 時間歸零時遊戲立即結束

### 時間增量 (Time Increment)

每次玩家走棋後，會為該玩家添加時間增量：

```cpp
void Qt_Chess::applyIncrement() {
    if (!m_timeControlEnabled || m_incrementMs <= 0) return;

    // 為剛完成移動的玩家添加增量
    // 注意：getCurrentPlayer() 在移動後返回對手
    PieceColor playerWhoMoved = (m_chessBoard.getCurrentPlayer() == PieceColor::White)
                                    ? PieceColor::Black : PieceColor::White;

    if (playerWhoMoved == PieceColor::White) {
        m_whiteTimeMs += m_incrementMs;
    } else {
        m_blackTimeMs += m_incrementMs;
    }
}
```

**增量時機：**
- 在玩家成功完成移動後立即應用
- 在切換到對手之前添加
- 即使玩家快速移動，也能累積時間

### 超時處理 (Timeout Handling)

當玩家時間耗盡時：

1. 將剩餘時間設為 0
2. 更新顯示
3. 停止計時器
4. 重置 `m_timerStarted` 標誌
5. 顯示勝負訊息框
6. 遊戲結束，對手獲勝

## 電腦模式時間倒數邏輯 (Computer Mode Timer Logic)

### 模式說明 (Mode Description)

電腦模式包括兩種變體：
- **Human vs Computer**: 人類執白，電腦執黑
- **Computer vs Human**: 電腦執白，人類執黑

### 時間倒數行為 (Timer Behavior)

**重要：電腦模式使用與雙人模式完全相同的計時邏輯。**

```cpp
// 電腦移動後也會應用相同的時間增量
void Qt_Chess::onEngineBestMove(const QString& move) {
    // ... 執行移動 ...
    
    // 為剛完成移動的玩家應用時間增量
    applyIncrement();
    
    updateBoard();
    updateTimeDisplays();
    updateStatus();
}
```

### 計時器啟動 (Timer Start)

電腦模式的計時器啟動方式與雙人模式相同：

1. 在第一步棋走出後啟動
2. 如果電腦執白棋且先走，計時器在電腦第一步後啟動
3. 如果人類執白棋且先走，計時器在人類第一步後啟動

### 電腦思考時間 (Computer Thinking Time)

電腦在思考時，其時間也會倒數：

- 電腦沒有「暫停」功能
- 從輪到電腦開始，其時間就開始倒數
- 電腦必須在時間耗盡前完成移動
- 如果電腦超時，人類玩家獲勝

```cpp
// 判斷是否輪到電腦
bool Qt_Chess::isComputerTurn() const {
    if (!m_chessEngine) return false;
    
    GameMode mode = getCurrentGameMode();
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    
    switch (mode) {
        case GameMode::HumanVsComputer:
            // 人執白，電腦執黑
            return currentPlayer == PieceColor::Black;
        case GameMode::ComputerVsHuman:
            // 電腦執白，人執黑
            return currentPlayer == PieceColor::White;
        default:
            return false;
    }
}
```

### 與雙人模式的一致性 (Consistency with Two-Player Mode)

電腦模式與雙人模式的計時邏輯保持一致：

| 特性 | 雙人模式 | 電腦模式 |
|------|---------|---------|
| 倒數速率 | 每 100ms 減少 100ms | 每 100ms 減少 100ms |
| 時間增量 | 支援 | 支援 |
| 超時判負 | 是 | 是 |
| 計時器啟動 | 第一步後 | 第一步後 |
| 對手時間凍結 | 是 | 是 |

## 線上模式時間倒數邏輯 (Online Mode Timer Logic)

### 特殊考量 (Special Considerations)

線上模式需要額外的同步機制，以確保兩位遠端玩家看到一致的時間：

```cpp
// 線上模式使用伺服器同步時間
if (m_isOnlineGame && m_gameStartLocalTime > 0) {
    // 獲取當前同步時間（使用伺服器時間偏移）
    qint64 currentSyncTime = QDateTime::currentMSecsSinceEpoch() + m_serverTimeOffset;
    qint64 gameStartSyncTime = m_gameStartLocalTime + m_serverTimeOffset;
    
    // 計算當前玩家在這個回合已用的時間
    qint64 turnElapsedMs = currentSyncTime - m_currentTurnStartTime;
    
    // 更新當前玩家的剩餘時間
    if (currentPlayer == PieceColor::White) {
        int newWhiteTime = m_whiteInitialTimeMs - static_cast<int>(turnElapsedMs);
        if (newWhiteTime < m_whiteTimeMs) {
            m_whiteTimeMs = newWhiteTime;
        }
    }
}
```

### 伺服器控制計時器 (Server-Controlled Timer)

線上模式還支援完全由伺服器控制的計時器：

```cpp
// 如果使用伺服器控制的計時器
if (m_useServerTimer && m_isOnlineGame) {
    updateTimeDisplaysFromServer();
    return;
}
```

這種模式下，客戶端只顯示時間，所有計時邏輯由伺服器處理。

## 計時器控制方法 (Timer Control Methods)

### startTimer()

```cpp
void Qt_Chess::startTimer() {
    if (m_timeControlEnabled && m_timerStarted && m_gameTimer && !m_gameTimer->isActive()) {
        m_gameTimer->start(100); // 每 100ms 觸發一次
    }
}
```

**功能：** 啟動遊戲計時器
**條件：**
- 時間控制已啟用
- 計時器已標記為啟動
- 計時器物件存在
- 計時器當前未運行

### stopTimer()

```cpp
void Qt_Chess::stopTimer() {
    if (m_gameTimer && m_gameTimer->isActive()) {
        m_gameTimer->stop();
    }
}
```

**功能：** 停止遊戲計時器
**使用場景：**
- 遊戲結束
- 玩家超時
- 進入回放模式
- 遊戲暫停

### applyIncrement()

```cpp
void Qt_Chess::applyIncrement() {
    if (!m_timeControlEnabled || m_incrementMs <= 0) return;

    PieceColor playerWhoMoved = (m_chessBoard.getCurrentPlayer() == PieceColor::White)
                                    ? PieceColor::Black : PieceColor::White;

    if (playerWhoMoved == PieceColor::White) {
        m_whiteTimeMs += m_incrementMs;
    } else {
        m_blackTimeMs += m_incrementMs;
    }
}
```

**功能：** 為剛完成移動的玩家添加時間增量
**時機：** 在每次成功移動後調用

## 回放模式中的計時器 (Timer in Replay Mode)

### 計時器暫停 (Timer Pause)

當進入回放模式時，計時器會自動暫停：

```cpp
void Qt_Chess::enterReplayMode() {
    // ... 其他邏輯 ...
    
    // 暫停計時器
    if (m_gameTimer && m_gameTimer->isActive()) {
        stopTimer();
    }
}
```

### 計時器恢復 (Timer Resume)

退出回放模式時，如果遊戲仍在進行中，計時器會恢復：

```cpp
void Qt_Chess::exitReplayMode() {
    // ... 恢復遊戲狀態 ...
    
    // 恢復計時器（如果遊戲仍在進行中）
    if (m_gameStarted && m_timeControlEnabled && m_timerStarted) {
        startTimer();
    }
}
```

## 時間顯示更新 (Time Display Updates)

### updateTimeDisplays()

```cpp
void Qt_Chess::updateTimeDisplays() {
    // 更新時間標籤
    m_whiteTimeLabel->setText(formatTime(m_whiteTimeMs));
    m_blackTimeLabel->setText(formatTime(m_blackTimeMs));
    
    // 更新進度條
    if (m_whiteInitialTimeMs > 0) {
        int whiteProgress = (m_whiteTimeMs * 100) / m_whiteInitialTimeMs;
        m_whiteTimeProgressBar->setValue(whiteProgress);
    }
    
    // 高亮當前玩家
    PieceColor currentPlayer = m_chessBoard.getCurrentPlayer();
    if (currentPlayer == PieceColor::White) {
        m_whiteTimeLabel->setStyleSheet("background-color: lightgreen;");
        m_blackTimeLabel->setStyleSheet("background-color: lightgray;");
    } else {
        m_whiteTimeLabel->setStyleSheet("background-color: lightgray;");
        m_blackTimeLabel->setStyleSheet("background-color: lightgreen;");
    }
}
```

### 視覺提示 (Visual Indicators)

- **綠色背景**: 當前輪到的玩家
- **灰色背景**: 等待中的玩家
- **進度條**: 顯示剩餘時間百分比
- **時間格式**: mm:ss（分:秒）

## 時間設定持久化 (Time Settings Persistence)

### 儲存設定 (Save Settings)

```cpp
void Qt_Chess::saveTimeControlSettings() {
    QSettings settings("Qt_Chess", "TimeControl");
    settings.setValue("whiteTimeSeconds", m_whiteTimeMs / 1000);
    settings.setValue("blackTimeSeconds", m_blackTimeMs / 1000);
    settings.setValue("incrementSeconds", m_incrementMs / 1000);
}
```

### 載入設定 (Load Settings)

```cpp
void Qt_Chess::loadTimeControlSettings() {
    QSettings settings("Qt_Chess", "TimeControl");
    int whiteTime = settings.value("whiteTimeSeconds", 600).toInt(); // 預設 10 分鐘
    int blackTime = settings.value("blackTimeSeconds", 600).toInt();
    int increment = settings.value("incrementSeconds", 0).toInt();
    
    m_whiteTimeMs = whiteTime * 1000;
    m_blackTimeMs = blackTime * 1000;
    m_incrementMs = increment * 1000;
}
```

## 特殊情況處理 (Special Case Handling)

### 無限時間模式 (Unlimited Time Mode)

當設定為「無限制」時：

```cpp
// 時間控制被停用
m_timeControlEnabled = (m_whiteTimeMs > 0 || m_blackTimeMs > 0);

if (!m_timeControlEnabled) {
    // 計時器不會啟動
    // 時間顯示為 "∞" 或隱藏
}
```

### 新遊戲重置 (New Game Reset)

開始新遊戲時，所有計時器狀態重置：

```cpp
void Qt_Chess::onNewGameClicked() {
    // 停止計時器
    stopTimer();
    
    // 重置時間
    m_whiteTimeMs = calculateTimeFromSliderValue(m_whiteTimeLimitSlider->value());
    m_blackTimeMs = calculateTimeFromSliderValue(m_blackTimeLimitSlider->value());
    
    // 重置計時器狀態
    m_timerStarted = false;
    m_gameStartLocalTime = 0;
    m_currentTurnStartTime = 0;
    
    // 初始化其他狀態...
}
```

### 投降處理 (Resignation Handling)

玩家投降時停止計時器：

```cpp
void Qt_Chess::onGiveUpClicked() {
    stopTimer();
    m_timerStarted = false;
    m_gameStarted = false;
    
    // 處理投降邏輯...
}
```

## 總結 (Summary)

### 雙人模式和電腦模式的共同點 (Commonalities)

1. **相同的倒數速率**: 兩種模式都以每 100ms 減少 100ms 的速率倒數
2. **相同的時間增量機制**: 走棋後都會獲得相同的時間增量
3. **相同的超時處理**: 時間耗盡時都會立即判負
4. **相同的計時器啟動邏輯**: 都在第一步棋後啟動

### 關鍵差異 (Key Differences)

唯一的差異在於玩家操作：
- **雙人模式**: 兩位人類玩家輪流操作
- **電腦模式**: 一位人類玩家與 AI 引擎輪流操作

但從計時器的角度來看，兩種模式完全一致，都將玩家視為獨立的時間實體。

### 設計原則 (Design Principles)

1. **公平性**: 所有玩家（包括電腦）都受相同的時間約束
2. **一致性**: 不同模式使用相同的計時邏輯
3. **準確性**: 100ms 的更新頻率提供平滑的倒數體驗
4. **可擴展性**: 支援線上模式的伺服器同步擴展

## 參考資料 (References)

- [TIME_CONTROL_FEATURE.md](TIME_CONTROL_FEATURE.md) - 時間控制功能的詳細說明
- 源碼檔案: `src/qt_chess.cpp` - 完整實現細節
- 源碼檔案: `src/qt_chess.h` - 時間控制相關的成員變數定義
