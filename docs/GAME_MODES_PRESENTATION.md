# Qt Chess 遊戲模式詳細介紹
## PowerPoint 簡報內容

> **轉換為 PowerPoint 說明**：
> 
> 方法 1 - 使用 Pandoc：
> ```bash
> pandoc GAME_MODES_PRESENTATION.md -o presentation.pptx
> ```
> 
> 方法 2 - 使用 Marp（推薦）：
> 1. 安裝 Marp: https://marp.app/
> 2. 打開此文件並導出為 PPTX
>
> 方法 3 - 線上轉換：
> 使用 https://www.markdowntopdf.com/ 或類似工具

---

## 投影片 1：封面
### Qt Chess 特殊遊戲模式
**四大創新玩法深度剖析**

- 🌫️ 霧戰模式 (Fog of War)
- 🌍 地吸引力模式 (Gravity Mode)  
- 🔮 傳送陣模式 (Teleportation)
- 💣 踩地雷模式 (Landmine)

專案路徑：`src/qt_chess.cpp` & `src/chessboard.cpp`

---

## 投影片 2：目錄
### 簡報大綱

1. 霧戰模式 - 視野限制系統
2. 地吸引力模式 - 物理重力模擬
3. 傳送陣模式 - 空間傳送機制
4. 踩地雷模式 - 地雷爆炸系統
5. 技術總結與對比

---

## 投影片 3：霧戰模式 - 概述
### 🌫️ Fog of War Mode

**功能說明**
- 限制玩家視野，只能看到己方棋子及其可移動範圍
- 對手棋子和位置被黑色迷霧遮蔽
- 動態計算可見區域

**遊戲影響**
- 增加策略深度
- 需要記憶對手位置
- 類似真實戰爭的資訊不對稱

---

## 投影片 4：霧戰模式 - 核心變數
### 資料結構

```cpp
// qt_chess.h (第 266-268 行)
bool m_fogOfWarEnabled;  // 模式開關
std::vector<std::vector<bool>> m_visibleSquares;  // 8x8 可見性矩陣
```

**變數說明**
- `m_fogOfWarEnabled`: 布林值，控制霧戰模式是否啟用
- `m_visibleSquares`: 二維向量，記錄每個方格的可見狀態
  - `true` = 可見
  - `false` = 被迷霧遮蔽

---

## 投影片 5：霧戰模式 - 視野計算演算法
### 核心函數：calculateVisibleSquares()

```cpp
// qt_chess.cpp (第 8187-8232 行)
void Qt_Chess::calculateVisibleSquares(PieceColor playerColor) {
    // 步驟 1：重置所有方格為不可見
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            m_visibleSquares[row][col] = false;
        }
    }
    
    // 步驟 2：遍歷所有己方棋子
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_chessBoard.getPiece(row, col);
            
            if (piece.getColor() == playerColor && 
                piece.getType() != PieceType::None) {
                // 棋子所在位置可見
                m_visibleSquares[row][col] = true;
                
                // 步驟 3：計算所有合法移動位置
                for (int targetRow = 0; targetRow < 8; ++targetRow) {
                    for (int targetCol = 0; targetCol < 8; ++targetCol) {
                        QPoint to(targetCol, targetRow);
                        if (m_chessBoard.isValidMove(from, to)) {
                            m_visibleSquares[targetRow][targetCol] = true;
                        }
                    }
                }
            }
        }
    }
}
```

---

## 投影片 6：霧戰模式 - 視覺效果實作
### 渲染邏輯

```cpp
// qt_chess.cpp (第 2006-2009 行)
if (m_fogOfWarEnabled && m_isOnlineGame && 
    !isSquareVisible(logicalRow, logicalCol)) {
    // 不可見方格顯示純黑色
    color = QColor(0, 0, 0);  // RGB: (0, 0, 0)
}

// qt_chess.cpp (第 2076-2078 行)
if (m_fogOfWarEnabled && m_isOnlineGame && 
    !isSquareVisible(logicalRow, logicalCol)) {
    return;  // 不顯示任何棋子
}
```

**視覺特效**
- 不可見方格：純黑色背景
- 棋子隱藏：完全不顯示
- 即時更新：每次移動後重新計算

---

## 投影片 7：地吸引力模式 - 概述
### 🌍 Gravity Mode

**功能說明**
- 棋盤順時針旋轉 90 度
- 所有棋子受重力影響向右側「下落」
- 棋子堆疊直到碰到邊界或其他棋子

**遊戲影響**
- 改變傳統國際象棋的空間概念
- 每次移動後棋子重新排列
- 需要預測重力效果

---

## 投影片 8：地吸引力模式 - 核心變數
### 資料結構

```cpp
// qt_chess.h (第 271 行)
bool m_gravityModeEnabled;  // 模式開關
```

**簡潔設計**
- 單一布林變數控制模式
- 不需額外資料結構
- 直接操作棋盤狀態

---

## 投影片 9：地吸引力模式 - 重力演算法
### 核心函數：applyGravity()

```cpp
// qt_chess.cpp (第 8263-8306 行)
void Qt_Chess::applyGravity() {
    if (!m_gravityModeEnabled || m_isReplayMode) return;
    
    bool pieceMoved = false;
    
    // 重複執行直到沒有棋子移動
    do {
        pieceMoved = false;
        
        // 從右往左檢查每一列（最右列不需檢查）
        for (int col = 6; col >= 0; --col) {
            for (int row = 0; row < 8; ++row) {
                ChessPiece& piece = m_chessBoard.getPiece(row, col);
                
                if (piece.getType() != PieceType::None) {
                    int targetCol = col + 1;
                    
                    // 向右滑動直到碰到障礙物
                    while (targetCol < 8 && 
                           m_chessBoard.getPiece(row, targetCol).getType() 
                           == PieceType::None) {
                        targetCol++;
                    }
                    
                    targetCol--;  // 退回到最後一個空位
                    
                    // 移動棋子
                    if (targetCol > col) {
                        ChessPiece movedPiece = piece;
                        m_chessBoard.setPiece(row, targetCol, movedPiece);
                        m_chessBoard.setPiece(row, col, 
                            ChessPiece(PieceType::None, PieceColor::None));
                        pieceMoved = true;
                    }
                }
            }
        }
    } while (pieceMoved);
}
```

---

## 投影片 10：地吸引力模式 - UI 旋轉
### 棋盤顯示旋轉函數

```cpp
// qt_chess.cpp (第 8308-8361 行)
void Qt_Chess::rotateBoardDisplay(bool rotate) {
    if (rotate) {
        // 順時針旋轉 90 度
        // 新行 = 舊列，新列 = 7 - 舊行
        for (int oldRow = 0; oldRow < 8; ++oldRow) {
            for (int oldCol = 0; oldCol < 8; ++oldCol) {
                int newRow = oldCol;
                int newCol = 7 - oldRow;
                gridLayout->addWidget(
                    tempSquares[oldRow][oldCol], newRow, newCol);
            }
        }
    } else {
        // 恢復正常佈局
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                gridLayout->addWidget(m_squares[row][col], row, col);
            }
        }
    }
}
```

**旋轉邏輯**
- 房主（Host）：正常角度
- 房客（Guest）：270 度旋轉

---

## 投影片 11：傳送陣模式 - 概述
### 🔮 Teleportation Mode

**功能說明**
- 棋盤上隨機生成兩個傳送門
- 棋子踩到傳送門自動傳送到另一個門
- 每次使用後重新生成新位置

**遊戲影響**
- 增加不可預測性
- 可快速穿越棋盤
- 可能吃掉傳送目標位置的對手棋子

---

## 投影片 12：傳送陣模式 - 核心變數
### 資料結構

```cpp
// qt_chess.h (第 273-277 行)
bool m_teleportModeEnabled;      // 模式開關
QPoint m_teleportPortal1;        // 傳送門 1 座標
QPoint m_teleportPortal2;        // 傳送門 2 座標
QPixmap m_teleportIconCache;     // 傳送門圖示快取
```

**變數說明**
- `QPoint`: Qt 的點類別，存儲 (x, y) 座標
- `QPixmap`: 圖片快取，避免重複載入 `send.png`
- 傳送門位置動態變化

---

## 投影片 13：傳送陣模式 - 生成演算法
### 核心函數：resetTeleportPortals()

```cpp
// qt_chess.cpp (第 8371-8411 行)
void Qt_Chess::resetTeleportPortals() {
    if (!m_teleportModeEnabled) {
        m_teleportPortal1 = QPoint(-1, -1);
        m_teleportPortal2 = QPoint(-1, -1);
        return;
    }
    
    // 收集所有空格子
    QVector<QPoint> emptySquares;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_chessBoard.getPiece(row, col);
            if (piece.getType() == PieceType::None) {
                emptySquares.append(QPoint(col, row));
            }
        }
    }
    
    if (emptySquares.size() < 2) {
        return;  // 空格不足
    }
    
    // Fisher-Yates 洗牌：隨機選擇兩個位置
    int index1 = QRandomGenerator::global()->bounded(emptySquares.size());
    int index2;
    do {
        index2 = QRandomGenerator::global()->bounded(emptySquares.size());
    } while (index2 == index1);
    
    m_teleportPortal1 = emptySquares[index1];
    m_teleportPortal2 = emptySquares[index2];
}
```

---

## 投影片 14：傳送陣模式 - 傳送邏輯
### 傳送執行函數

```cpp
// qt_chess.cpp (第 8470-8516 行)
QPoint Qt_Chess::handleTeleportationAndGetFinalPosition(
    const QPoint& from, const QPoint& to) {
    
    QPoint finalPosition = to;  // 預設：不傳送
    
    // 檢查是否踩到傳送門
    if (!isTeleportPortal(to.y(), to.x())) {
        return finalPosition;
    }
    
    // 確定目標傳送門
    QPoint targetPortal = (to == m_teleportPortal1) ? 
                          m_teleportPortal2 : m_teleportPortal1;
    
    // 檢查目標門是否被佔據
    ChessPiece piece = m_chessBoard.getPiece(to.y(), to.x());
    const ChessPiece& targetPiece = 
        m_chessBoard.getPiece(targetPortal.y(), targetPortal.x());
    
    if (targetPiece.getType() != PieceType::None) {
        if (targetPiece.getColor() == piece.getColor()) {
            return finalPosition;  // 己方棋子，傳送失敗
        }
        // 對方棋子，傳送並吃掉
    }
    
    // 執行傳送
    m_chessBoard.setPiece(targetPortal.y(), targetPortal.x(), piece);
    m_chessBoard.setPiece(to.y(), to.x(), 
        ChessPiece(PieceType::None, PieceColor::None));
    
    finalPosition = targetPortal;
    
    // 重新生成傳送門
    resetTeleportPortals();
    
    return finalPosition;
}
```

---

## 投影片 15：傳送陣模式 - 視覺效果
### 傳送門圖示顯示

```cpp
// qt_chess.cpp (第 2081-2093 行)
if (m_teleportModeEnabled && 
    isTeleportPortal(logicalRow, logicalCol) && 
    piece.getType() == PieceType::None) {
    
    // 載入傳送門圖示
    if (m_teleportIconCache.isNull()) {
        m_teleportIconCache = QPixmap(":/resources/images/send.png");
    }
    
    QIcon portalIcon(m_teleportIconCache);
    square->setIcon(portalIcon);
    square->setIconSize(iconSize);
}
```

**視覺元素**
- 圖示：`send.png` 疊加在空格上
- 快取機制：避免重複載入圖片
- 只在空格顯示（有棋子時隱藏）

---

## 投影片 16：踩地雷模式 - 概述
### 💣 Landmine / Bomb Mode

**功能說明**
- 棋盤中隱藏 4-5 個地雷
- 棋子移動到地雷位置觸發爆炸
- 棋子被摧毀，國王踩雷直接輸掉遊戲

**遊戲影響**
- 高風險高回報
- 需要記憶地雷位置
- 增加隨機性和刺激感

---

## 投影片 17：踩地雷模式 - 核心變數
### 資料結構

```cpp
// chessboard.h (第 96-98 行)
bool m_bombModeEnabled;              // 模式開關
std::vector<QPoint> m_minePositions; // 地雷座標列表
bool m_lastMoveTriggeredMine;        // 上一步是否觸發地雷
```

**變數說明**
- `m_minePositions`: 動態陣列存儲所有地雷位置
- `m_lastMoveTriggeredMine`: 用於觸發爆炸動畫
- 地雷觸發後從列表中移除（一次性）

---

## 投影片 18：踩地雷模式 - 地雷生成演算法
### 靜態工具函數：generateRandomMinePositions()

```cpp
// chessboard.cpp (第 714-742 行)
std::vector<QPoint> ChessBoard::generateRandomMinePositions() {
    std::vector<QPoint> minePositions;
    
    // 地雷區域：第 3-6 行（索引 2-5），全部 8 列
    std::vector<QPoint> availablePositions;
    for (int row = 2; row <= 5; ++row) {
        for (int col = 0; col < 8; ++col) {
            availablePositions.push_back(QPoint(col, row));
        }
    }
    
    // 隨機生成 4 或 5 個地雷
    QRandomGenerator *rng = QRandomGenerator::global();
    int numMines = 4 + (rng->bounded(2));  // 4 或 5
    
    // Fisher-Yates 洗牌演算法
    for (int i = availablePositions.size() - 1; i > 0; --i) {
        int j = rng->bounded(i + 1);
        std::swap(availablePositions[i], availablePositions[j]);
    }
    
    // 選取前 numMines 個位置
    for (int i = 0; i < numMines && 
         i < static_cast<int>(availablePositions.size()); ++i) {
        minePositions.push_back(availablePositions[i]);
    }
    
    return minePositions;
}
```

**演算法說明**
- 地雷區域：32 個候選位置（4 行 × 8 列）
- Fisher-Yates 洗牌：O(n) 時間複雜度
- 隨機性：每局遊戲地雷位置不同

---

## 投影片 19：踩地雷模式 - 爆炸檢測
### 移動中的地雷檢測邏輯

```cpp
// chessboard.cpp (第 256-285 行)
// 在 movePiece() 函數中
if (m_bombModeEnabled && isMineAt(to)) {
    // 踩到地雷：棋子被摧毀
    ChessPiece explodedPiece = m_board[to.y()][to.x()];
    m_board[to.y()][to.x()] = 
        ChessPiece(PieceType::None, PieceColor::None);
    
    // 加入被吃掉的棋子列表（用於UI顯示）
    if (explodedPiece.getColor() == PieceColor::White) {
        m_capturedWhite.push_back(explodedPiece);
    } else {
        m_capturedBlack.push_back(explodedPiece);
    }
    
    m_lastMoveTriggeredMine = true;
    
    // 移除該地雷（一次性使用）
    auto it = std::find(m_minePositions.begin(), 
                        m_minePositions.end(), to);
    if (it != m_minePositions.end()) {
        m_minePositions.erase(it);
    }
    
    // 國王踩雷 = 遊戲結束
    if (explodedPiece.getType() == PieceType::King) {
        if (explodedPiece.getColor() == PieceColor::White) {
            m_gameResult = GameResult::BlackWins;
        } else {
            m_gameResult = GameResult::WhiteWins;
        }
    }
}
```

---

## 投影片 20：踩地雷模式 - 爆炸動畫
### UI 動畫效果實作

```cpp
// qt_chess.cpp (第 2117-2175 行)
void Qt_Chess::handleMineExplosion(const QPoint& logicalPosition, 
                                   bool isOpponentMove) {
    // 1. 播放爆炸音效
    if (m_soundSettings.allSoundsEnabled) {
        m_explosionSound.play();  // bomb.mp3
    }
    
    // 2. 顯示爆炸圖片
    QPushButton* explodedSquare = m_squares[displayRow][displayCol];
    m_explodingSquares.insert(explodedSquare);
    
    explodedSquare->setText("");
    QPixmap boomPixmap(":/resources/images/boom.png");
    QIcon boomIcon(boomPixmap);
    explodedSquare->setIcon(boomIcon);
    explodedSquare->setIconSize(explodedSquare->size());
    
    // 3. 設置爆炸樣式
    explodedSquare->setStyleSheet(
        "QPushButton { "
        "  background-color: rgba(255, 100, 0, 0.8); "
        "  border: 3px solid #FF0000; "
        "}"
    );
    
    // 4. 1.5 秒後恢復正常
    QTimer::singleShot(1500, this, [this, explodedSquare, ...]() {
        m_explodingSquares.remove(explodedSquare);
        explodedSquare->setIcon(QIcon());
        updateSquareColor(displayRow, displayCol);
    });
    
    // 5. 檢查遊戲結束
    if (isKingExplosion) {
        QTimer::singleShot(1600, this, [this]() {
            updateStatus();
        });
    }
}
```

---

## 投影片 21：踩地雷模式 - 線上同步機制
### 多人遊戲的地雷同步

**同步策略**
- 房主（Host）生成地雷位置
- 透過網路訊息發送給所有玩家
- 所有客戶端使用相同的地雷位置

```cpp
// qt_chess.cpp (第 2913-2919 行)
// 房主端
if (m_selectedGameModes.contains(GAME_MODE_BOMB) && 
    m_selectedGameModes[GAME_MODE_BOMB]) {
    minePositions = ChessBoard::generateRandomMinePositions();
    qDebug() << "Host generated" << minePositions.size() 
             << "mine positions";
}

// qt_chess.cpp (第 6365-6377 行)
// 客戶端接收
if (m_selectedGameModes[GAME_MODE_BOMB]) {
    if (!minePositions.empty()) {
        m_chessBoard.enableBombMode(true);
        m_chessBoard.setMinePositions(minePositions);
    } else {
        // 錯誤處理：未收到地雷位置
        QMessageBox::warning(this, "地雷模式錯誤", 
            "未能從伺服器接收地雷位置資料");
    }
}
```

---

## 投影片 22：技術對比表
### 四種模式的技術特性比較

| 特性 | 霧戰 | 地吸引力 | 傳送陣 | 踩地雷 |
|------|------|----------|--------|--------|
| **主要類別** | Qt_Chess | Qt_Chess | Qt_Chess | ChessBoard |
| **資料結構** | 2D bool 陣列 | 單一 bool | 2 個 QPoint | QPoint 向量 |
| **演算法複雜度** | O(n²) | O(n²) | O(n) | O(n) |
| **視覺效果** | 黑色遮罩 | 棋盤旋轉 | 圖示疊加 | 爆炸動畫 |
| **線上同步** | 獨立計算 | 同步重力 | 獨立生成 | 房主同步 |
| **音效** | ❌ | ❌ | ❌ | ✅ |
| **遊戲結束** | ❌ | ❌ | ❌ | ✅（國王） |

---

## 投影片 23：程式碼架構圖
### 模組關係與呼叫流程

```
┌─────────────────────────────────────────────┐
│           Qt_Chess (主控制器)                │
│  • 遊戲流程控制                              │
│  • UI 事件處理                               │
│  • 線上同步                                  │
└────────┬────────────────────────────────────┘
         │
         ├─────────────────┬─────────────────┐
         │                 │                 │
    ┌────▼────┐      ┌────▼────┐      ┌────▼────┐
    │霧戰模式  │      │地吸引力  │      │傳送陣    │
    │         │      │         │      │         │
    │8x8 陣列 │      │重力演算法│      │2 傳送門 │
    └─────────┘      └─────────┘      └─────────┘
                            │
                       ┌────▼─────────────────┐
                       │  ChessBoard (棋盤)    │
                       │  • 棋子管理           │
                       │  • 移動驗證           │
                       │  • 地雷系統           │
                       └──────────────────────┘
```

---

## 投影片 24：關鍵程式碼檔案
### 原始碼位置索引

**主要實作檔案**
1. `src/qt_chess.h` (第 54-59, 266-289 行)
   - 遊戲模式常數定義
   - 成員變數宣告

2. `src/qt_chess.cpp`
   - 霧戰模式：第 8184-8257 行
   - 地吸引力：第 8260-8361 行
   - 傳送陣：第 8364-8535 行
   - 地雷動畫：第 2117-2175 行

3. `src/chessboard.h` (第 76-84, 96-98 行)
   - 地雷模式介面定義

4. `src/chessboard.cpp`
   - 地雷生成：第 714-742 行
   - 地雷檢測：第 256-285 行

---

## 投影片 25：遊戲模式常數
### 字串常數定義

```cpp
// qt_chess.h (第 54-58 行)
constexpr const char* GAME_MODE_FOG_OF_WAR = "霧戰";
constexpr const char* GAME_MODE_GRAVITY = "地吸引力";
constexpr const char* GAME_MODE_TELEPORT = "傳送陣";
constexpr const char* GAME_MODE_DICE = "骰子";
constexpr const char* GAME_MODE_BOMB = "踩地雷";
```

**用途**
- 網路訊息協定
- UI 選單識別
- 設定檔儲存

---

## 投影片 26：效能考量
### 各模式的效能分析

**霧戰模式**
- 時間複雜度：O(64 × 64) = O(4096) 每次更新
- 優化：只在移動後重新計算
- 記憶體：512 bytes (8×8 bool)

**地吸引力模式**
- 最壞情況：O(n²) 當所有棋子需移動
- 優化：迴圈直到無移動（早期終止）
- 記憶體：無額外開銷

**傳送陣模式**
- 時間複雜度：O(n) 掃描空格
- 優化：圖示快取
- 記憶體：~4KB 圖片快取

**踩地雷模式**
- 時間複雜度：O(1) 檢測，O(n) 生成
- 優化：Fisher-Yates O(n)
- 記憶體：~40 bytes (5 個 QPoint)

---

## 投影片 27：設計模式應用
### 軟體工程實踐

**策略模式 (Strategy Pattern)**
- 每種遊戲模式可獨立開關
- 透過 bool flag 控制行為

**觀察者模式 (Observer Pattern)**
- UI 更新基於棋盤狀態變化
- 信號槽機制 (Qt Signals/Slots)

**單例模式 (Singleton)**
- `QRandomGenerator::global()` 全域隨機數生成器

**快取模式 (Caching)**
- 傳送門圖示快取 (`m_teleportIconCache`)
- 棋子圖示快取 (`m_pieceIconCache`)

---

## 投影片 28：測試建議
### 品質保證策略

**單元測試**
```cpp
// 霧戰模式測試
QVERIFY(isSquareVisible(0, 0) == true);  // 己方棋子
QVERIFY(isSquareVisible(7, 7) == false); // 對手區域

// 地雷生成測試
auto mines = ChessBoard::generateRandomMinePositions();
QVERIFY(mines.size() >= 4 && mines.size() <= 5);
QVERIFY(mines[0].y() >= 2 && mines[0].y() <= 5);
```

**整合測試**
- 多模式組合測試
- 線上同步測試
- 邊界條件測試

---

## 投影片 29：未來改進方向
### 潛在優化與擴展

**功能擴展**
1. 霧戰模式：可調整視野範圍
2. 地吸引力：支援四個方向的重力
3. 傳送陣：多對傳送門網路
4. 踩地雷：可移動的地雷

**效能優化**
1. 霧戰：增量更新而非全部重算
2. 地吸引力：GPU 加速物理模擬
3. 傳送陣：空間索引加速查找
4. 踩地雷：哈希表加速檢測

**使用者體驗**
- 動畫過渡效果
- 音效增強
- 提示系統

---

## 投影片 30：總結
### 四大遊戲模式核心要點

**霧戰模式** 🌫️
- 資訊不對稱遊戲
- 動態視野計算
- 策略深度增加

**地吸引力模式** 🌍
- 物理模擬創新
- 棋盤空間重構
- 視覺旋轉效果

**傳送陣模式** 🔮
- 隨機傳送機制
- 空間跳躍戰術
- 每局不同體驗

**踩地雷模式** 💣
- 高風險玩法
- 爆炸視聽效果
- 記憶力挑戰

**技術亮點**
- 模組化設計
- 高效演算法
- 完整線上同步

---

## 投影片 31：參考資源
### 相關文件與連結

**專案文件**
- `docs/GAME_MODES_FEATURE.md` - 遊戲模式功能說明
- `docs/BOMB_CHESS_FEATURE.md` - 地雷模式詳細文檔
- `docs/UPDATED_GAME_MODES.md` - 模式選擇對話框

**原始碼**
- `src/qt_chess.h` - 頭文件
- `src/qt_chess.cpp` - 主要實作
- `src/chessboard.cpp` - 棋盤邏輯

**圖形資源**
- `resources/images/send.png` - 傳送門圖示
- `resources/images/boom.png` - 爆炸圖片
- `resources/sounds/bomb.mp3` - 爆炸音效

---

## 投影片 32：Q&A
### 問題與討論

**常見問題**

Q: 多個模式可以同時啟用嗎？
A: 可以，所有模式都可自由組合

Q: 線上模式如何保證同步？
A: 霧戰/傳送陣各自計算，地雷由房主生成並廣播

Q: 效能瓶頸在哪裡？
A: 霧戰模式的視野計算，已優化為只在移動後執行

**感謝觀看！**

---
