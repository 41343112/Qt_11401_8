# src 目錄程式碼詳細說明

本文檔詳細解釋 Qt_Chess 專案中 `src/` 目錄下所有程式檔案的內容、功能和架構。

## 目錄

1. [程式架構總覽](#程式架構總覽)
2. [主程式入口](#主程式入口)
3. [核心遊戲邏輯](#核心遊戲邏輯)
4. [主視窗與 UI](#主視窗與-ui)
5. [設定對話框](#設定對話框)
6. [網路功能](#網路功能)
7. [輔助功能](#輔助功能)

---

## 程式架構總覽

```
Qt_Chess 應用程式架構
├── main.cpp                      # 應用程式進入點
├── qt_chess.h/cpp                # 主視窗類別（UI 和遊戲流程控制）
│
├── 核心遊戲邏輯
│   ├── chesspiece.h/cpp          # 棋子類別（棋子類型、顏色、移動規則）
│   ├── chessboard.h/cpp          # 棋盤類別（遊戲狀態、規則驗證）
│   └── chessengine.h/cpp         # AI 引擎介面（Stockfish 整合）
│
├── 設定對話框
│   ├── soundsettingsdialog.h/cpp       # 音效設定對話框
│   ├── pieceiconsettingsdialog.h/cpp   # 棋子圖示設定對話框
│   └── boardcolorsettingsdialog.h/cpp  # 棋盤顏色設定對話框
│
├── 網路功能
│   ├── networkmanager.h/cpp      # WebSocket 網路管理器（線上對戰）
│   └── onlinedialog.h/cpp        # 線上遊戲設定對話框
│
└── 輔助功能
    ├── updatechecker.h/cpp       # 版本更新檢查器
    └── theme.h                   # 主題顏色常數定義
```

---

## 主程式入口

### main.cpp

**位置**: `src/main.cpp`

**功能**: 應用程式的進入點，初始化 Qt 應用程式並啟動主視窗。

**主要程式碼**:
```cpp
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Qt_Chess w;
    w.showFullScreen();
    return a.exec();
}
```

**說明**:
- 創建 QApplication 物件處理應用程式生命週期
- 創建主視窗 Qt_Chess 實例
- 以全螢幕模式顯示視窗
- 啟動事件循環 (exec())

---

## 核心遊戲邏輯

### 1. chesspiece.h/cpp

**位置**: `src/chesspiece.h`, `src/chesspiece.cpp`

**功能**: 定義西洋棋棋子的類別，包含棋子類型、顏色和移動規則驗證。

#### 主要列舉型別 (Enums)

**PieceType** - 棋子類型:
- `None` - 空格（無棋子）
- `Pawn` - 兵
- `Rook` - 城堡
- `Knight` - 騎士
- `Bishop` - 主教
- `Queen` - 皇后
- `King` - 國王

**PieceColor** - 棋子顏色:
- `None` - 無顏色
- `White` - 白色
- `Black` - 黑色

#### ChessPiece 類別

**成員變數**:
- `m_type: PieceType` - 棋子類型
- `m_color: PieceColor` - 棋子顏色
- `m_hasMoved: bool` - 是否已移動過（用於王車易位和兵的首步兩格移動）

**主要公開方法**:

| 方法 | 功能 | 返回值 |
|-----|------|--------|
| `ChessPiece(type, color)` | 建構函式 | - |
| `getType()` | 取得棋子類型 | `PieceType` |
| `getColor()` | 取得棋子顏色 | `PieceColor` |
| `hasMoved()` | 檢查是否已移動 | `bool` |
| `setMoved(bool)` | 設定移動狀態 | `void` |
| `getSymbol()` | 取得 Unicode 符號 | `QString` |
| `isValidMove(from, to, board, enPassantTarget)` | 驗證移動是否合法 | `bool` |

**私有方法（移動規則驗證）**:
- `isValidPawnMove()` - 驗證兵的移動（包含吃過路兵）
- `isValidRookMove()` - 驗證城堡的移動
- `isValidKnightMove()` - 驗證騎士的移動
- `isValidBishopMove()` - 驗證主教的移動
- `isValidQueenMove()` - 驗證皇后的移動
- `isValidKingMove()` - 驗證國王的移動（包含王車易位）
- `isPathClear()` - 檢查移動路徑是否暢通

**特殊規則實現**:
1. **兵的移動**:
   - 向前一格或從起始位置向前兩格
   - 斜向吃子
   - 吃過路兵 (En Passant)
   
2. **王車易位**:
   - 國王水平移動兩格
   - 額外驗證在 ChessBoard 中完成

3. **路徑暢通檢查**:
   - 用於城堡、主教、皇后的移動
   - 確保移動路徑上沒有其他棋子

**Unicode 符號**:
- 白色: ♔ ♕ ♖ ♗ ♘ ♙
- 黑色: ♚ ♛ ♜ ♝ ♞ ♟

---

### 2. chessboard.h/cpp

**位置**: `src/chessboard.h`, `src/chessboard.cpp`

**功能**: 管理棋盤狀態、遊戲規則、棋步記錄和遊戲結果。

#### 主要列舉型別

**GameResult** - 遊戲結果:
- `InProgress` - 進行中
- `WhiteWins` - 白方獲勝
- `BlackWins` - 黑方獲勝
- `Draw` - 和局
- `WhiteResigns` - 白方認輸
- `BlackResigns` - 黑方認輸

#### MoveRecord 結構

記錄每一步棋的詳細資訊:
- `from/to: QPoint` - 起始和目標位置
- `pieceType: PieceType` - 移動的棋子類型
- `pieceColor: PieceColor` - 移動的棋子顏色
- `isCapture: bool` - 是否為吃子
- `isCastling: bool` - 是否為王車易位
- `isEnPassant: bool` - 是否為吃過路兵
- `isPromotion: bool` - 是否為兵升變
- `promotionType: PieceType` - 升變後的棋子類型
- `isCheck: bool` - 是否造成將軍
- `isCheckmate: bool` - 是否為將死
- `algebraicNotation: QString` - 代數記譜法表示

#### ChessBoard 類別

**成員變數**:
- `m_board: vector<vector<ChessPiece>>` - 8x8 棋盤陣列
- `m_currentPlayer: PieceColor` - 當前玩家
- `m_enPassantTarget: QPoint` - 吃過路兵目標位置
- `m_moveHistory: vector<MoveRecord>` - 棋步歷史記錄
- `m_gameResult: GameResult` - 遊戲結果
- `m_capturedWhite/m_capturedBlack: vector<ChessPiece>` - 被吃的棋子

**主要公開方法**:

| 方法 | 功能 |
|-----|------|
| `initializeBoard()` | 初始化棋盤為標準起始狀態 |
| `getPiece(row, col)` | 取得指定位置的棋子 |
| `setPiece(row, col, piece)` | 設定指定位置的棋子 |
| `movePiece(from, to)` | 移動棋子（執行移動並更新狀態） |
| `isValidMove(from, to)` | 檢查移動是否合法 |
| `getCurrentPlayer()` | 取得當前玩家 |
| `isInCheck(color)` | 檢查指定顏色的國王是否被將軍 |
| `isCheckmate(color)` | 檢查是否為將死 |
| `isStalemate(color)` | 檢查是否為僵局 |
| `isInsufficientMaterial()` | 檢查是否子力不足（和局） |
| `findKing(color)` | 尋找指定顏色的國王位置 |
| `needsPromotion(to)` | 檢查兵是否需要升變 |
| `promotePawn(pos, newType)` | 執行兵升變 |
| `getMoveHistory()` | 取得棋步歷史 |
| `getMoveNotation(index)` | 取得指定棋步的記譜法 |
| `getAllMoveNotations()` | 取得所有棋步的記譜法列表 |
| `getGameResult()` | 取得遊戲結果 |
| `getCapturedPieces(color)` | 取得被吃的棋子列表 |

**私有方法**:
- `switchPlayer()` - 切換當前玩家
- `wouldBeInCheck()` - 檢查移動後是否會被將軍
- `hasAnyValidMoves()` - 檢查是否有任何合法移動
- `canCastle()` - 檢查王車易位的所有條件
- `recordMove()` - 記錄棋步到歷史
- `generateAlgebraicNotation()` - 生成代數記譜法

**棋盤初始化**:
- 黑色棋子在第 0-1 行
- 白色棋子在第 6-7 行
- 標準西洋棋起始佈局

**特殊規則處理**:
1. **將軍檢測**: 檢查對手是否可以吃掉國王
2. **合法移動**: 確保移動後不會讓自己的國王被將軍
3. **王車易位**: 驗證國王和城堡都未移動、路徑暢通、不在將軍狀態
4. **吃過路兵**: 追蹤上一步移動，判斷是否可以吃過路兵
5. **兵升變**: 當兵到達對面底線時自動觸發升變

---

### 3. chessengine.h/cpp

**位置**: `src/chessengine.h`, `src/chessengine.cpp`

**功能**: 整合 Stockfish 西洋棋引擎，提供 AI 對手功能。使用 UCI (Universal Chess Interface) 協議與引擎通訊。

#### 主要列舉型別

**EngineDifficulty** - 引擎難度:
- `VeryEasy = 1` - 非常簡單
- `Easy = 5` - 簡單
- `Medium = 10` - 中等
- `Hard = 15` - 困難
- `VeryHard = 20` - 非常困難

**GameMode** - 遊戲模式:
- `HumanVsHuman` - 雙人對弈
- `HumanVsComputer` - 人機對弈（玩家執白）
- `ComputerVsHuman` - 人機對弈（玩家執黑）
- `OnlineGame` - 線上對戰

#### ChessEngine 類別

**成員變數**:
- `m_process: QProcess*` - 引擎進程
- `m_enginePath: QString` - 引擎執行檔路徑
- `m_bestMove: QString` - 引擎計算的最佳走法
- `m_gameMode: GameMode` - 當前遊戲模式
- `m_skillLevel: int` - 技能等級 (0-20)
- `m_thinkingTimeMs: int` - 思考時間（毫秒）
- `m_searchDepth: int` - 搜尋深度 (1-30)
- `m_isReady: bool` - 引擎是否就緒
- `m_isThinking: bool` - 引擎是否正在思考

**主要方法**:

**引擎控制**:
- `startEngine(path)` - 啟動引擎進程
- `stopEngine()` - 停止引擎進程
- `isEngineRunning()` - 檢查引擎是否運行中

**遊戲設定**:
- `setGameMode(mode)` - 設定遊戲模式
- `setDifficulty(level)` - 設定難度等級
- `setThinkingTime(ms)` - 設定思考時間
- `setSearchDepth(depth)` - 設定搜尋深度

**棋局控制**:
- `newGame()` - 開始新遊戲
- `setPosition(fen)` - 設定棋盤位置（FEN 格式）
- `setPositionFromMoves(moves)` - 從移動列表設定位置
- `requestMove()` - 請求引擎計算最佳走法
- `stop()` - 停止當前計算

**工具函數**:
- `boardToFEN(board)` - 將棋盤轉換為 FEN 格式
- `moveToUCI(from, to, promotion)` - 將移動轉換為 UCI 格式
- `uciToMove(uci, from, to, promotion)` - 解析 UCI 格式移動

**信號 (Signals)**:
- `engineReady()` - 引擎就緒
- `bestMoveFound(move)` - 找到最佳走法
- `engineError(error)` - 引擎錯誤
- `thinkingStarted()` - 開始思考
- `thinkingStopped()` - 停止思考

**UCI 協議通訊**:

引擎使用標準 UCI 協議：

1. **初始化**:
   ```
   → uci
   ← uciok
   → isready
   ← readyok
   ```

2. **設定選項**:
   ```
   → setoption name Skill Level value 10
   → setoption name Hash value 16
   → setoption name Ponder value false
   ```

3. **設定位置和請求移動**:
   ```
   → position startpos moves e2e4 e7e5
   → go movetime 1000 depth 10
   ← bestmove g1f3
   ```

**FEN (Forsyth–Edwards Notation)**:
- 標準的棋盤位置表示法
- 包含：棋子位置、輪到誰下、王車易位權利、吃過路兵目標、半步計數、全步計數
- 範例: `rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1`

**難度調整**:
- Skill Level 0: 初學者水平
- Skill Level 10: 中級水平
- Skill Level 20: 大師水平
- 透過限制思考時間和搜尋深度進一步調整難度

---

## 主視窗與 UI

### qt_chess.h/cpp

**位置**: `src/qt_chess.h`, `src/qt_chess.cpp`

**功能**: 主視窗類別，管理整個應用程式的 UI 和遊戲流程。這是最大也最複雜的類別，包含超過 500 個成員變數和方法。

#### 主要系統模組

Qt_Chess 類別被組織為以下功能模組：

#### 1. 核心遊戲狀態
- `m_chessBoard: ChessBoard` - 棋盤邏輯
- `m_currentGameMode: GameMode` - 當前遊戲模式
- `m_gameStarted: bool` - 遊戲是否開始
- `m_selectedSquare: QPoint` - 選中的格子
- `m_pieceSelected: bool` - 是否有棋子被選中
- `m_lastMoveFrom/m_lastMoveTo: QPoint` - 上一步移動的位置

#### 2. UI 元件 - 棋盤
- `m_squares: vector<vector<QPushButton*>>` - 8x8 按鈕格子陣列
- `m_buttonCoordinates: QMap<QPushButton*, QPoint>` - 按鈕到座標的映射
- 拖放狀態變數（`m_isDragging`, `m_dragLabel` 等）

#### 3. UI 元件 - 遊戲控制
- `m_newGameButton: QPushButton*` - 新遊戲按鈕
- `m_resignButton: QPushButton*` - 認輸按鈕
- `m_requestDrawButton: QPushButton*` - 請求和棋按鈕
- `m_exitButton: QPushButton*` - 退出按鈕
- `m_startButton: QPushButton*` - 開始按鈕

#### 4. UI 元件 - 主選單
- `m_mainMenuWidget: QWidget*` - 主選單容器
- `m_mainMenuLocalPlayButton: QPushButton*` - 本地遊玩按鈕
- `m_mainMenuComputerPlayButton: QPushButton*` - 電腦對戰按鈕
- `m_mainMenuOnlinePlayButton: QPushButton*` - 線上遊玩按鈕
- `m_mainMenuSettingsButton: QPushButton*` - 設定按鈕

#### 5. 時間控制系統
完整的時間控制功能，支援：
- 獨立的白方/黑方時間設定
- 每步增量時間
- 進度條視覺化
- 伺服器同步計時（線上模式）

**相關變數**:
- `m_whiteTimeMs/m_blackTimeMs: int` - 剩餘時間
- `m_incrementMs: int` - 增量時間
- `m_gameTimer: QTimer*` - 遊戲計時器
- `m_timeControlEnabled: bool` - 時間控制是否啟用

#### 6. 棋譜系統
- `m_moveListWidget: QListWidget*` - 棋譜列表顯示
- `m_exportPGNButton: QPushButton*` - 匯出 PGN 按鈕
- `m_copyPGNButton: QPushButton*` - 複製棋譜按鈕

#### 7. 被吃棋子顯示
- `m_capturedWhitePanel/m_capturedBlackPanel: QWidget*` - 被吃棋子面板
- `m_capturedWhiteLabels/m_capturedBlackLabels: QList<QLabel*>` - 棋子標籤
- `m_whiteScoreDiffLabel/m_blackScoreDiffLabel: QLabel*` - 分差顯示

#### 8. 回放系統
- `m_isReplayMode: bool` - 是否在回放模式
- `m_replayMoveIndex: int` - 當前回放索引
- 回放控制按鈕（首、上、下、末）

#### 9. 電腦對弈系統
- `m_chessEngine: ChessEngine*` - 引擎實例
- `m_difficultySlider: QSlider*` - 難度滑桿
- `m_thinkingLabel: QLabel*` - "電腦思考中..." 標籤
- `m_uciMoveHistory: QStringList` - UCI 格式移動歷史

#### 10. 線上對戰系統
- `m_networkManager: NetworkManager*` - 網路管理器
- `m_isOnlineGame: bool` - 是否為線上遊戲
- `m_waitingForOpponent: bool` - 等待對手
- 線上模式相關按鈕和標籤

#### 11. 音效系統
- 移動、吃子、王車易位、將軍、將死音效
- 背景音樂播放器
- 音效設定管理

#### 12. 動畫系統
- 遊戲開始動畫
- 啟動動畫
- 動畫疊加層和標籤

#### 13. 設定系統
- 棋子圖示設定
- 棋盤顏色設定
- 音效設定
- 棋盤翻轉設定

#### 14. 更新檢查系統
- `m_updateChecker: UpdateChecker*` - 更新檢查器
- 自動和手動更新檢查

#### 主要方法分類

**UI 設置**:
- `setupUI()` - 初始化所有 UI 元件
- `setupMenuBar()` - 設置選單欄
- `setupMainMenu()` - 設置主選單
- `setupTimeControlUI()` - 設置時間控制 UI
- `setupEngineUI()` - 設置引擎 UI
- `applyModernStylesheet()` - 應用現代樣式表

**棋盤顯示**:
- `updateBoard()` - 更新棋盤顯示
- `updateSquareColor()` - 更新格子顏色
- `displayPieceOnSquare()` - 在格子上顯示棋子
- `highlightValidMoves()` - 高亮有效移動
- `clearHighlights()` - 清除高亮

**遊戲流程**:
- `onSquareClicked()` - 格子點擊處理
- `onNewGameClicked()` - 新遊戲處理
- `onResignClicked()` - 認輸處理
- `handleGameEnd()` - 遊戲結束處理

**時間控制**:
- `startTimer()` - 啟動計時器
- `stopTimer()` - 停止計時器
- `onGameTimerTick()` - 計時器滴答處理
- `updateTimeDisplays()` - 更新時間顯示

**回放功能**:
- `enterReplayMode()` - 進入回放模式
- `exitReplayMode()` - 退出回放模式
- `replayToMove()` - 回放到指定步數

**電腦對弈**:
- `initializeEngine()` - 初始化引擎
- `requestEngineMove()` - 請求引擎移動
- `onEngineBestMove()` - 處理引擎返回的最佳走法
- `isComputerTurn()` - 檢查是否輪到電腦

**線上對戰**:
- `initializeNetwork()` - 初始化網路
- `onCreateRoomButtonClicked()` - 創建房間
- `onJoinRoomButtonClicked()` - 加入房間
- `onOpponentMove()` - 處理對手移動

**音效**:
- `initializeSounds()` - 初始化音效
- `playSoundForMove()` - 為移動播放音效
- `startBackgroundMusic()` - 開始背景音樂

**設定**:
- `loadPieceIconSettings()` - 載入棋子圖示設定
- `loadBoardColorSettings()` - 載入棋盤顏色設定
- `loadSoundSettings()` - 載入音效設定

**事件處理**:
- `mousePressEvent()` - 滑鼠按下事件
- `mouseMoveEvent()` - 滑鼠移動事件（拖放）
- `mouseReleaseEvent()` - 滑鼠釋放事件
- `keyPressEvent()` - 鍵盤事件
- `resizeEvent()` - 視窗大小調整事件

---

## 設定對話框

### 1. soundsettingsdialog.h/cpp

**位置**: `src/soundsettingsdialog.h`, `src/soundsettingsdialog.cpp`

**功能**: 音效設定對話框，允許使用者自訂各種遊戲音效。

#### SoundSettings 結構

包含所有音效設定：
- 5 種音效檔案路徑（移動、吃子、王車易位、將軍、將死）
- 5 種音效音量 (0.0-1.0)
- 5 種音效啟用狀態
- 全部音效啟用狀態

**功能特點**:
- 瀏覽並選擇自訂音效檔案（WAV、MP3、OGG）
- 獨立調整每種音效的音量（滑桿控制）
- 啟用/停用個別音效
- 預覽音效
- 重設個別音效或全部重設為預設值
- 設定持久化儲存

**UI 元件**:
- 音效檔案路徑編輯框
- 瀏覽檔案按鈕
- 音量滑桿和標籤
- 啟用/停用核取方塊
- 預覽按鈕
- 重設按鈕

**預設音效**:
位於 `resources/sounds/` 目錄下的內建音效檔案。

---

### 2. pieceiconsettingsdialog.h/cpp

**位置**: `src/pieceiconsettingsdialog.h`, `src/pieceiconsettingsdialog.cpp`

**功能**: 棋子圖示設定對話框，允許使用者自訂棋子的外觀。

#### IconSetType 列舉

- `Unicode` - 預設 Unicode 符號（♔♕♖♗♘♙）
- `Preset1/2/3` - 三組預設圖示集
- `Custom` - 使用者自訂圖示

#### PieceIconSettings 結構

包含：
- 12 個棋子圖示路徑（白色 6 個 + 黑色 6 個）
- `useCustomIcons: bool` - 是否使用自訂圖示
- `iconSetType: IconSetType` - 圖示集類型
- `pieceScale: int` - 棋子縮放比例 (60-100%)

**功能特點**:
- 從下拉選單選擇圖示集
- 視覺化預覽不同圖示集
- 為每個棋子上傳自訂圖片
- 棋子大小滑桿調整（60%-100%）
- 預覽個別棋子圖示
- 重設為預設 Unicode 符號
- 儲存/載入自訂圖示集

**支援的圖片格式**:
PNG、JPG、SVG、BMP（建議使用透明背景 PNG）

**圖示集目錄結構**:
```
resources/pieces/
├── set1/         # 預設圖示集 1
├── set2/         # 預設圖示集 2
└── set3/         # 預設圖示集 3
```

---

### 3. boardcolorsettingsdialog.h/cpp

**位置**: `src/boardcolorsettingsdialog.h`, `src/boardcolorsettingsdialog.cpp`

**功能**: 棋盤顏色設定對話框，允許使用者自訂棋盤的配色方案。

#### ColorScheme 列舉

預設配色方案：
- `Classic` - 經典棕褐色
- `BlueGray` - 藍灰配色
- `GreenWhite` - 綠白配色
- `PurplePink` - 紫粉配色
- `WoodDark` - 木紋深色
- `OceanBlue` - 海洋藍
- `LightTheme` - 淺色系
- `Custom1-7` - 7 個自訂配色插槽

#### BoardColorSettings 結構

包含：
- `lightSquareColor: QColor` - 淺色格子顏色
- `darkSquareColor: QColor` - 深色格子顏色
- `scheme: ColorScheme` - 配色方案類型
- `customName: QString` - 自訂配色名稱

**功能特點**:
- 預設配色方案縮圖預覽
- 2×2 預覽網格即時顯示效果
- 顏色選擇器自訂任意顏色
- 儲存自訂配色到插槽
- 重設為預設配色
- 設定持久化儲存

**UI 組件**:
- 淺色/深色格子顏色按鈕
- 預設方案預覽按鈕網格
- 2×2 棋盤預覽
- 自訂配色插槽

---

## 網路功能

### 1. networkmanager.h/cpp

**位置**: `src/networkmanager.h`, `src/networkmanager.cpp`

**功能**: 使用 WebSocket 實現線上對戰功能，連接到中央伺服器進行雙人對弈。

#### NetworkRole 列舉
- `None` - 無角色
- `Host` - 房主
- `Guest` - 加入者

#### ConnectionStatus 列舉
- `Disconnected` - 未連接
- `Connecting` - 連接中
- `Connected` - 已連接
- `Error` - 錯誤

#### MessageType 列舉

定義所有網路訊息類型：
- `CreateRoom` - 創建房間請求
- `RoomCreated` - 房間創建確認
- `JoinRoom` - 加入房間請求
- `JoinAccepted` - 加入被接受
- `JoinRejected` - 加入被拒絕
- `GameStart` - 遊戲開始
- `StartGame` - 房主通知開始遊戲
- `TimeSettings` - 時間設定更新
- `Move` - 棋步
- `GameOver` - 遊戲結束
- `Surrender` - 投降
- `Chat` - 聊天訊息
- `PlayerDisconnected` - 玩家斷線
- `Ping/Pong` - 心跳包

#### NetworkManager 類別

**成員變數**:
- `m_webSocket: QWebSocket*` - WebSocket 連接
- `m_role: NetworkRole` - 當前角色
- `m_status: ConnectionStatus` - 連接狀態
- `m_roomNumber: QString` - 房間號碼
- `m_serverUrl: QString` - 伺服器 URL
- `m_playerColor/m_opponentColor: PieceColor` - 玩家和對手顏色

**主要方法**:

**房間管理**:
- `createRoom()` - 創建房間（伺服器生成房號）
- `joinRoom(roomNumber)` - 加入指定房間
- `leaveRoom()` - 離開房間（通知伺服器）
- `closeConnection()` - 關閉連接

**遊戲同步**:
- `sendMove(from, to, promotion)` - 發送棋步
- `sendGameStart(color)` - 發送遊戲開始
- `sendStartGame(...)` - 房主發送開始遊戲通知（含時間設定）
- `sendTimeSettings(...)` - 發送時間設定更新
- `sendSurrender()` - 發送投降
- `sendDrawOffer()` - 發送和棋請求
- `sendDrawResponse(accepted)` - 回應和棋請求
- `sendGameOver(result)` - 發送遊戲結束

**訊息處理**:
- `sendMessage(json)` - 發送 JSON 訊息
- `processMessage(json)` - 處理接收的訊息

**信號 (Signals)**:
- `connected()` - 連接成功
- `disconnected()` - 斷開連接
- `connectionError(error)` - 連接錯誤
- `roomCreated(roomNumber)` - 房間創建成功
- `opponentJoined()` - 對手加入
- `playerLeft()` - 對手離開
- `opponentMove(from, to, promotion)` - 收到對手棋步
- `startGameReceived(...)` - 收到開始遊戲通知
- `timeSettingsReceived(...)` - 收到時間設定
- `timerStateReceived(...)` - 收到計時器狀態
- `surrenderReceived()` - 收到投降
- `drawOfferReceived()` - 收到和棋請求
- `drawResponseReceived(accepted)` - 收到和棋回應
- `opponentDisconnected()` - 對手斷線

**伺服器通訊**:
使用 WebSocket 與中央伺服器通訊：
- 預設伺服器: `chess-server-mjg6.onrender.com`
- JSON 格式訊息交換
- 自動房號生成（4 位數字）
- 即時雙向通訊

**房間流程**:
1. **創建房間**: Host 請求 → 伺服器生成房號 → 返回房號
2. **加入房間**: Guest 提供房號 → 伺服器驗證 → 配對成功
3. **遊戲進行**: 棋步即時同步
4. **斷線處理**: 自動偵測並通知對手

---

### 2. onlinedialog.h/cpp

**位置**: `src/onlinedialog.h`, `src/onlinedialog.cpp`

**功能**: 線上遊戲設定對話框，用於選擇遊戲模式和規則（未來擴展功能）。

**遊戲模式核取方塊**:
- 標準模式 (Standard)
- 快棋模式 (Rapid)
- 閃電模式 (Blitz)
- 讓子模式 (Handicap)
- 自訂規則 (Custom Rules)

**功能**:
- 選擇想要支援的遊戲模式
- 返回選擇的模式映射
- 為未來的進階線上功能預留介面

---

## 輔助功能

### 1. updatechecker.h/cpp

**位置**: `src/updatechecker.h`, `src/updatechecker.cpp`

**功能**: 檢查 GitHub 上是否有新版本可用。

#### UpdateChecker 類別

**成員變數**:
- `m_networkManager: QNetworkAccessManager*` - 網路管理器
- `m_latestVersion: QString` - 最新版本號
- `m_releaseUrl: QString` - 發行頁面 URL
- `m_releaseNotes: QString` - 更新說明

**主要方法**:
- `checkForUpdates()` - 檢查更新
- `getCurrentVersion()` - 取得當前版本（靜態方法）
- `getLatestVersion()` - 取得最新版本號
- `getReleaseUrl()` - 取得發行 URL
- `getReleaseNotes()` - 取得更新說明

**信號**:
- `updateCheckFinished(updateAvailable)` - 檢查完成
- `updateCheckFailed(error)` - 檢查失敗

**功能流程**:
1. 向 GitHub API 發送請求
2. 解析 JSON 回應取得最新版本資訊
3. 比較版本號（使用 QVersionNumber）
4. 發出信號通知結果

**GitHub API**:
- 端點: `https://api.github.com/repos/[owner]/[repo]/releases/latest`
- 取得最新發行版本的標籤、URL 和說明

**版本比較**:
使用語意版本控制 (Semantic Versioning)：
- 格式: `v主版本.次版本.修訂版本` (例如: v1.2.3)
- 智慧比較確保正確檢測更新

---

### 2. theme.h

**位置**: `src/theme.h`

**功能**: 定義應用程式的主題顏色常數，確保視覺一致性。

**主題風格**: 現代科技風格 - 基於霓虹電路板主題

#### 主題命名空間常數

**背景顏色**:
- `BG_DARK = "#0A1628"` - 深色背景（深藍黑色）
- `BG_MEDIUM = "#0D1F3C"` - 中等深度背景（深藍色）
- `BG_PANEL = "#0F2940"` - 面板背景（霓虹藍色調）

**強調色**:
- `ACCENT_PRIMARY = "#00FFFF"` - 主要強調色（霓虹青色）
- `ACCENT_SECONDARY = "#FF9955"` - 次要強調色（霓虹橙色）
- `ACCENT_SUCCESS = "#00D9FF"` - 成功色（青色）
- `ACCENT_WARNING = "#FFB366"` - 警告色（霓虹橙黃色）

**文字顏色**:
- `TEXT_PRIMARY = "#E8F4F8"` - 主要文字顏色（淺青白色）

**邊框顏色**:
- `BORDER = "#1A3F5C"` - 邊框顏色（深青藍色）

**懸停效果**:
- `HOVER_BORDER = "#00FFFF"` - 懸停邊框
- `HOVER_BG = "rgba(0, 255, 255, 0.2)"` - 懸停背景（半透明）
- `PRESSED_BG = "rgba(0, 255, 255, 0.4)"` - 按下背景

**使用方式**:
```cpp
#include "theme.h"
// 在樣式表中使用
button->setStyleSheet(
    QString("background-color: %1; border: 2px solid %2;")
    .arg(Theme::BG_PANEL)
    .arg(Theme::ACCENT_PRIMARY)
);
```

**設計理念**:
- 深色主題減少眼睛疲勞
- 霓虹色調營造現代科技感
- 高對比度確保可讀性
- 一致的顏色使用提升使用者體驗

---

## 程式碼組織結構

### 依賴關係圖

```
main.cpp
    └── Qt_Chess (主視窗)
        ├── ChessBoard (遊戲邏輯)
        │   └── ChessPiece (棋子)
        ├── ChessEngine (AI 引擎)
        │   └── ChessBoard (讀取棋盤狀態)
        ├── NetworkManager (網路功能)
        │   └── WebSocket 通訊
        ├── UpdateChecker (更新檢查)
        │   └── HTTP 請求
        ├── SoundSettingsDialog (音效設定)
        ├── PieceIconSettingsDialog (圖示設定)
        ├── BoardColorSettingsDialog (顏色設定)
        ├── OnlineDialog (線上設定)
        └── Theme (主題常數)
```

### 檔案大小和複雜度

| 檔案 | 行數 (約) | 複雜度 | 主要功能 |
|------|----------|--------|---------|
| qt_chess.cpp | ~8700 | 非常高 | UI 和遊戲流程控制 |
| chessboard.cpp | ~700 | 高 | 遊戲邏輯和規則 |
| networkmanager.cpp | ~800 | 中高 | 網路通訊 |
| chessengine.cpp | ~420 | 中 | AI 引擎整合 |
| pieceiconsettingsdialog.cpp | ~1300 | 中 | 圖示設定 UI |
| boardcolorsettingsdialog.cpp | ~800 | 中 | 顏色設定 UI |
| soundsettingsdialog.cpp | ~700 | 中 | 音效設定 UI |
| chesspiece.cpp | ~150 | 低 | 棋子移動規則 |
| updatechecker.cpp | ~100 | 低 | 版本檢查 |
| onlinedialog.cpp | ~120 | 低 | 線上模式選單 |
| main.cpp | ~10 | 極低 | 程式進入點 |
| theme.h | ~35 | 極低 | 常數定義 |

### 設計模式

**使用的設計模式**:

1. **Model-View-Controller (MVC)**:
   - Model: ChessBoard, ChessPiece
   - View: Qt_Chess UI 元件
   - Controller: Qt_Chess 事件處理

2. **Strategy Pattern**:
   - 不同遊戲模式 (HumanVsHuman, HumanVsComputer, OnlineGame)
   - 不同移動驗證策略（每種棋子）

3. **Observer Pattern**:
   - Qt 信號/槽機制
   - UI 元件監聽遊戲狀態變化

4. **Singleton Pattern**:
   - Theme 主題常數（命名空間）

5. **Factory Pattern**:
   - ChessPiece 創建
   - 對話框創建

---

## 關鍵技術點

### 1. Qt 框架使用

**核心 Qt 模組**:
- `QtCore` - 基礎類別和容器
- `QtWidgets` - UI 元件
- `QtMultimedia` - 音效播放
- `QtNetwork` - HTTP 請求
- `QtWebSockets` - WebSocket 通訊

**Qt 特性**:
- 信號/槽機制 (Signals/Slots)
- 事件系統 (Event System)
- 佈局管理 (Layout Management)
- 資源系統 (Resource System)
- 樣式表 (Stylesheets)

### 2. 西洋棋規則實現

**完整規則支援**:
- ✅ 所有棋子的標準移動
- ✅ 王車易位 (Castling)
- ✅ 兵升變 (Pawn Promotion)
- ✅ 吃過路兵 (En Passant)
- ✅ 將軍檢測 (Check)
- ✅ 將死檢測 (Checkmate)
- ✅ 僵局檢測 (Stalemate)
- ✅ 子力不足和局

**移動驗證層級**:
1. 棋子基本移動規則 (ChessPiece)
2. 路徑暢通檢查
3. 不能吃自己的棋子
4. 不能讓自己的國王被將軍 (ChessBoard)
5. 特殊規則驗證（王車易位、吃過路兵）

### 3. UCI 協議

**Universal Chess Interface (UCI)**:
- 標準的引擎通訊協議
- 文本基礎的命令/回應
- 支援位置設定、移動搜尋、引擎選項

**主要命令**:
- `uci` - 初始化
- `isready` - 檢查就緒
- `ucinewgame` - 新遊戲
- `position` - 設定位置
- `go` - 開始搜尋
- `stop` - 停止搜尋
- `setoption` - 設定選項

### 4. WebSocket 通訊

**即時雙向通訊**:
- 持久連接
- JSON 格式訊息
- 低延遲同步

**訊息結構**:
```json
{
    "type": "Move",
    "from": {"x": 4, "y": 6},
    "to": {"x": 4, "y": 4},
    "promotionType": "None"
}
```

### 5. 代數記譜法 (Algebraic Notation)

**標準記譜法**:
- 短記法: `e4`, `Nf3`, `O-O`
- 吃子: `exd5`, `Nxf7`
- 將軍: `Qh5+`
- 將死: `Qh7#`
- 王車易位: `O-O` (王翼), `O-O-O` (后翼)
- 兵升變: `e8=Q`

**PGN (Portable Game Notation)**:
- 標準的棋譜格式
- 包含標頭（事件、日期、玩家等）
- 完整的移動列表
- 遊戲結果

### 6. 時間控制實現

**計時系統**:
- 獨立的白方/黑方計時器
- 每步增量時間 (Increment)
- 進度條視覺化
- 暫停/恢復功能
- 超時處理

**線上模式同步**:
- 伺服器控制的計時器
- 時間偏移補償
- 網路延遲處理

---

## 編譯和建置

### 專案檔案結構

```
Qt_Chess/
├── Qt_Chess.pro          # qmake 專案檔
├── main.cpp
├── src/                  # 所有原始碼
│   ├── *.h
│   └── *.cpp
├── resources/            # 資源檔案
│   ├── sounds/          # 音效檔案
│   ├── pieces/          # 棋子圖示
│   └── music/           # 背景音樂
├── resources.qrc        # Qt 資源檔
├── engine/              # Stockfish 引擎
└── docs/                # 文檔
```

### 建置步驟

```bash
# 生成 Makefile
qmake Qt_Chess.pro

# 編譯
make

# 執行
./Qt_Chess
```

### 依賴項

**必須**:
- Qt5 (5.12+)
- Qt5WebSockets
- C++17 編譯器

**可選**:
- Stockfish 引擎（用於 AI 對弈）

---

## 未來擴展方向

### 計劃功能

1. **進階 AI**:
   - 多引擎支援
   - 開局庫
   - 殘局庫

2. **線上功能增強**:
   - 好友系統
   - 聊天功能
   - 觀戰模式
   - 排行榜

3. **訓練模式**:
   - 戰術訓練
   - 殘局練習
   - 開局練習

4. **分析功能**:
   - 移動分析
   - 錯誤標記
   - 引擎評估顯示

5. **自訂功能**:
   - 棋盤主題
   - 棋子動畫
   - 音效主題包

---

## 總結

這個 Qt_Chess 專案是一個功能完整的西洋棋應用程式，包含：

✅ **完整的遊戲邏輯** - 所有西洋棋規則正確實現  
✅ **多種遊戲模式** - 雙人、電腦、線上對戰  
✅ **AI 對手** - Stockfish 引擎整合  
✅ **線上對戰** - WebSocket 即時同步  
✅ **豐富的 UI** - 現代化介面和動畫  
✅ **可自訂** - 音效、圖示、顏色設定  
✅ **完整功能** - 時間控制、棋譜、回放、更新檢查

**程式碼品質**:
- 清晰的模組劃分
- 良好的註釋（中文）
- 合理的類別設計
- Qt 最佳實踐

**適合用途**:
- 學習 Qt 開發
- 學習遊戲邏輯實現
- 學習網路程式設計
- 西洋棋娛樂和練習

這是一個優秀的教育性專案，展示了如何使用 Qt 框架開發一個功能豐富的桌面應用程式。
