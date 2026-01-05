# 程式碼位置和說明指南

本文件說明西洋棋遊戲中「設定」和「棋子判斷」功能的程式碼位置及其意義。

## 目錄

1. [設定功能程式碼](#設定功能程式碼)
   - [音效設定](#音效設定)
   - [棋子圖標設定](#棋子圖標設定)
   - [棋盤顏色設定](#棋盤顏色設定)
2. [棋子判斷程式碼](#棋子判斷程式碼)
   - [棋子類別定義](#棋子類別定義)
   - [移動驗證邏輯](#移動驗證邏輯)
   - [棋盤遊戲規則](#棋盤遊戲規則)

---

## 設定功能程式碼

設定功能讓玩家可以自訂遊戲的外觀和音效。主要包含三個設定對話框：

### 音效設定

**檔案位置：**
- 標頭檔：`src/soundsettingsdialog.h`
- 實作檔：`src/soundsettingsdialog.cpp`

**主要功能：**

這個類別 `SoundSettingsDialog` 提供一個對話框，讓使用者可以自訂遊戲中的各種音效。

**程式碼結構說明：**

```cpp
class SoundSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    // 音效設定資料結構
    struct SoundSettings {
        QString moveSound;           // 移動音效檔案路徑
        QString captureSound;        // 吃子音效檔案路徑
        QString castlingSound;       // 王車易位音效檔案路徑
        QString checkSound;          // 將軍音效檔案路徑
        QString checkmateSound;      // 將死音效檔案路徑
        double moveVolume;           // 移動音效音量 (0.0-1.0)
        double captureVolume;        // 吃子音效音量
        double castlingVolume;       // 王車易位音效音量
        double checkVolume;          // 將軍音效音量
        double checkmateVolume;      // 將死音效音量
        bool moveSoundEnabled;       // 是否啟用移動音效
        bool captureSoundEnabled;    // 是否啟用吃子音效
        bool castlingSoundEnabled;   // 是否啟用王車易位音效
        bool checkSoundEnabled;      // 是否啟用將軍音效
        bool checkmateSoundEnabled;  // 是否啟用將死音效
        bool allSoundsEnabled;       // 是否啟用所有音效
    };
    
    SoundSettings getSettings() const;        // 取得目前設定
    void setSettings(const SoundSettings& settings);  // 套用設定
    static SoundSettings getDefaultSettings();        // 取得預設設定
};
```

**重要方法說明：**

1. **`getSettings()`**：取得使用者在對話框中設定的所有音效參數
2. **`setSettings()`**：將儲存的設定載入到對話框中
3. **`getDefaultSettings()`**：提供預設的音效設定值
4. **瀏覽按鈕槽函數（如 `onBrowseMove()`）**：讓使用者選擇自訂音效檔案
5. **預覽按鈕槽函數（如 `onPreviewMove()`）**：播放音效讓使用者試聽
6. **重設按鈕槽函數（如 `onResetMove()`）**：將個別音效重設為預設值

**使用方式：**

設定會透過 Qt 的 `QSettings` 類別儲存到本地設定檔中，在程式關閉後會保留設定。

---

### 棋子圖標設定

**檔案位置：**
- 標頭檔：`src/pieceiconsettingsdialog.h`
- 實作檔：`src/pieceiconsettingsdialog.cpp`

**主要功能：**

這個類別 `PieceIconSettingsDialog` 提供一個對話框，讓使用者可以自訂棋子的圖示。

**程式碼結構說明：**

```cpp
class PieceIconSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    // 圖標集類型列舉
    enum class IconSetType {
        Unicode,      // 預設 Unicode 符號 (♔, ♕, ♖, 等)
        Preset1,      // 預設圖示集 1
        Preset2,      // 預設圖示集 2
        Preset3,      // 預設圖示集 3
        Custom        // 自訂使用者選擇的圖示
    };

    // 棋子圖標設定資料結構
    struct PieceIconSettings {
        QString whiteKingIcon;     // 白國王圖示路徑
        QString whiteQueenIcon;    // 白皇后圖示路徑
        QString whiteRookIcon;     // 白城堡圖示路徑
        QString whiteBishopIcon;   // 白主教圖示路徑
        QString whiteKnightIcon;   // 白騎士圖示路徑
        QString whitePawnIcon;     // 白兵圖示路徑
        QString blackKingIcon;     // 黑國王圖示路徑
        QString blackQueenIcon;    // 黑皇后圖示路徑
        QString blackRookIcon;     // 黑城堡圖示路徑
        QString blackBishopIcon;   // 黑主教圖示路徑
        QString blackKnightIcon;   // 黑騎士圖示路徑
        QString blackPawnIcon;     // 黑兵圖示路徑
        bool useCustomIcons;       // 是否使用自訂圖示
        IconSetType iconSetType;   // 圖標集類型
        int pieceScale;            // 棋子縮放百分比 (60-100)
    };
    
    PieceIconSettings getSettings() const;
    void setSettings(const PieceIconSettings& settings);
    static PieceIconSettings getDefaultSettings();
    static PieceIconSettings getPresetSettings(IconSetType setType);
};
```

**重要功能：**

1. **圖標集選擇**：可以從 Unicode 符號、預設圖示集或完全自訂中選擇
2. **個別棋子自訂**：為 12 種棋子（6 種白棋 + 6 種黑棋）各自設定圖示
3. **縮放功能**：使用滑桿調整棋子大小（60%-100%）
4. **預覽功能**：套用前可以預覽圖示外觀
5. **儲存/載入**：可以儲存自訂圖標集並載入

---

### 棋盤顏色設定

**檔案位置：**
- 標頭檔：`src/boardcolorsettingsdialog.h`
- 實作檔：`src/boardcolorsettingsdialog.cpp`

**主要功能：**

這個類別 `BoardColorSettingsDialog` 提供一個對話框，讓使用者可以自訂棋盤的配色。

**程式碼結構說明：**

```cpp
class BoardColorSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    // 配色方案列舉
    enum class ColorScheme {
        Classic,      // 經典棕色/米色（預設）
        BlueGray,     // 藍色和灰色
        GreenWhite,   // 綠色和白色
        PurplePink,   // 紫色和粉色
        WoodDark,     // 木色和深色
        OceanBlue,    // 海藍色
        LightTheme,   // 淺色柔和色
        Custom1-7     // 自訂使用者選擇的顏色插槽 1-7
    };

    // 棋盤顏色設定資料結構
    struct BoardColorSettings {
        QColor lightSquareColor;  // 淺色方格顏色
        QColor darkSquareColor;   // 深色方格顏色
        ColorScheme scheme;       // 配色方案
        QString customName;       // 自訂配色方案的名稱
    };
    
    BoardColorSettings getSettings() const;
    void setSettings(const BoardColorSettings& settings);
    static BoardColorSettings getDefaultSettings();
    static BoardColorSettings getPresetSettings(ColorScheme scheme);
};
```

**重要功能：**

1. **預設配色方案**：提供 7 種預設配色（經典、藍灰、綠白、紫粉、木紋、海洋、淺色）
2. **自訂顏色**：使用顏色選擇器選擇任意顏色
3. **2×2 預覽網格**：即時顯示棋盤配色效果
4. **縮圖預覽**：顯示所有預設配色方案的視覺預覽
5. **自訂插槽**：可以儲存最多 7 個自訂配色方案

---

## 棋子判斷程式碼

棋子判斷功能負責檢查棋子的移動是否合法，以及執行西洋棋的遊戲規則。

### 棋子類別定義

**檔案位置：**
- 標頭檔：`src/chesspiece.h`
- 實作檔：`src/chesspiece.cpp`

**主要功能：**

這個類別 `ChessPiece` 定義了棋子的類型、顏色和移動規則。

**程式碼結構說明：**

```cpp
// 棋子類型列舉
enum class PieceType {
    None,     // 無棋子（空格）
    Pawn,     // 兵
    Rook,     // 城堡
    Knight,   // 騎士
    Bishop,   // 主教
    Queen,    // 皇后
    King      // 國王
};

// 棋子顏色列舉
enum class PieceColor {
    None,     // 無顏色（空格）
    White,    // 白色
    Black     // 黑色
};

class ChessPiece {
public:
    ChessPiece(PieceType type = PieceType::None, PieceColor color = PieceColor::None);
    
    PieceType getType() const { return m_type; }      // 取得棋子類型
    PieceColor getColor() const { return m_color; }   // 取得棋子顏色
    bool hasMoved() const { return m_hasMoved; }      // 是否已移動過
    void setMoved(bool moved) { m_hasMoved = moved; } // 設定已移動標記
    
    QString getSymbol() const;  // 取得 Unicode 符號（♔, ♕, 等）
    
    // 檢查移動到目標位置是否有效（基本棋子移動規則）
    bool isValidMove(const QPoint& from, const QPoint& to, 
                     const std::vector<std::vector<ChessPiece>>& board,
                     const QPoint& enPassantTarget = QPoint(-1, -1)) const;
    
private:
    PieceType m_type;    // 棋子類型
    PieceColor m_color;  // 棋子顏色
    bool m_hasMoved;     // 是否已移動過（用於王車易位和兵的雙格移動）
    
    // 各種棋子的移動驗證函數
    bool isValidPawnMove(const QPoint& from, const QPoint& to, 
                         const std::vector<std::vector<ChessPiece>>& board,
                         const QPoint& enPassantTarget) const;
    bool isValidRookMove(const QPoint& from, const QPoint& to, 
                         const std::vector<std::vector<ChessPiece>>& board) const;
    bool isValidKnightMove(const QPoint& from, const QPoint& to) const;
    bool isValidBishopMove(const QPoint& from, const QPoint& to, 
                           const std::vector<std::vector<ChessPiece>>& board) const;
    bool isValidQueenMove(const QPoint& from, const QPoint& to, 
                          const std::vector<std::vector<ChessPiece>>& board) const;
    bool isValidKingMove(const QPoint& from, const QPoint& to) const;
    
    bool isPathClear(const QPoint& from, const QPoint& to, 
                     const std::vector<std::vector<ChessPiece>>& board) const;
};
```

---

### 移動驗證邏輯

**主要驗證方法：`isValidMove()`**

這個方法是棋子移動驗證的核心，位於 `src/chesspiece.cpp` 中。

**程式碼說明：**

```cpp
bool ChessPiece::isValidMove(const QPoint& from, const QPoint& to, 
                              const std::vector<std::vector<ChessPiece>>& board,
                              const QPoint& enPassantTarget) const {
    // 1. 檢查起點和終點是否相同
    if (from == to) return false;
    
    // 2. 檢查終點是否在棋盤邊界內
    if (to.x() < 0 || to.x() >= 8 || to.y() < 0 || to.y() >= 8) return false;
    
    // 3. 檢查是否吃掉自己的棋子（不允許）
    const ChessPiece& target = board[to.y()][to.x()];
    if (target.getColor() == m_color && target.getType() != PieceType::None) return false;
    
    // 4. 根據棋子類型調用對應的驗證函數
    switch (m_type) {
        case PieceType::Pawn:   return isValidPawnMove(from, to, board, enPassantTarget);
        case PieceType::Rook:   return isValidRookMove(from, to, board);
        case PieceType::Knight: return isValidKnightMove(from, to);
        case PieceType::Bishop: return isValidBishopMove(from, to, board);
        case PieceType::Queen:  return isValidQueenMove(from, to, board);
        case PieceType::King:   return isValidKingMove(from, to);
        default: return false;
    }
}
```

**各棋子移動規則詳解：**

#### 1. 兵（Pawn）的移動規則

位於 `isValidPawnMove()` 方法中：

```cpp
bool ChessPiece::isValidPawnMove(const QPoint& from, const QPoint& to, 
                                  const std::vector<std::vector<ChessPiece>>& board,
                                  const QPoint& enPassantTarget) const {
    // 白色兵向上移動（-1），黑色兵向下移動（+1）
    int direction = (m_color == PieceColor::White) ? -1 : 1;
    int startRow = (m_color == PieceColor::White) ? 6 : 1;
    
    int dx = to.x() - from.x();  // 水平位移
    int dy = to.y() - from.y();  // 垂直位移
    
    // 向前移動（不能吃子）
    if (dx == 0) {
        const ChessPiece& target = board[to.y()][to.x()];
        if (target.getType() != PieceType::None) return false;  // 前方有棋子
        
        // 向前一格
        if (dy == direction) return true;
        
        // 從起始位置向前兩格
        if (!m_hasMoved && from.y() == startRow && dy == 2 * direction) {
            const ChessPiece& middle = board[from.y() + direction][from.x()];
            return middle.getType() == PieceType::None;  // 中間沒有棋子
        }
    }
    // 斜向吃子
    else if (abs(dx) == 1 && dy == direction) {
        const ChessPiece& target = board[to.y()][to.x()];
        // 普通斜向吃子
        if (target.getType() != PieceType::None && target.getColor() != m_color) {
            return true;
        }
        // 吃過路兵（En Passant）
        if (enPassantTarget.x() >= 0 && to == enPassantTarget) {
            return true;
        }
    }
    
    return false;
}
```

**說明：**
- 兵只能向前移動
- 第一次移動可以走 1 或 2 格
- 只能斜向吃子
- 支援吃過路兵特殊規則

#### 2. 城堡（Rook）的移動規則

```cpp
bool ChessPiece::isValidRookMove(const QPoint& from, const QPoint& to, 
                                  const std::vector<std::vector<ChessPiece>>& board) const {
    // 必須是水平或垂直移動
    if (from.x() != to.x() && from.y() != to.y()) return false;
    
    // 檢查路徑是否暢通
    return isPathClear(from, to, board);
}
```

**說明：**
- 只能水平或垂直移動
- 路徑上不能有其他棋子

#### 3. 騎士（Knight）的移動規則

```cpp
bool ChessPiece::isValidKnightMove(const QPoint& from, const QPoint& to) const {
    int dx = abs(to.x() - from.x());
    int dy = abs(to.y() - from.y());
    
    // L 形移動：2+1 或 1+2
    return (dx == 2 && dy == 1) || (dx == 1 && dy == 2);
}
```

**說明：**
- 騎士走「日」字或「L」形
- 可以跳過其他棋子

#### 4. 主教（Bishop）的移動規則

```cpp
bool ChessPiece::isValidBishopMove(const QPoint& from, const QPoint& to, 
                                    const std::vector<std::vector<ChessPiece>>& board) const {
    int dx = abs(to.x() - from.x());
    int dy = abs(to.y() - from.y());
    
    // 必須是斜向移動（dx == dy）
    if (dx != dy) return false;
    
    // 檢查路徑是否暢通
    return isPathClear(from, to, board);
}
```

**說明：**
- 只能斜向移動
- 路徑上不能有其他棋子

#### 5. 皇后（Queen）的移動規則

```cpp
bool ChessPiece::isValidQueenMove(const QPoint& from, const QPoint& to, 
                                   const std::vector<std::vector<ChessPiece>>& board) const {
    // 皇后 = 城堡 + 主教
    return isValidRookMove(from, to, board) || isValidBishopMove(from, to, board);
}
```

**說明：**
- 結合城堡和主教的移動方式
- 可以水平、垂直或斜向移動

#### 6. 國王（King）的移動規則

```cpp
bool ChessPiece::isValidKingMove(const QPoint& from, const QPoint& to) const {
    int dx = abs(to.x() - from.x());
    int dy = abs(to.y() - from.y());
    
    // 只能移動一格
    return dx <= 1 && dy <= 1;
}
```

**說明：**
- 可以向任何方向移動一格
- 王車易位（Castling）由 `ChessBoard` 類別處理

---

### 棋盤遊戲規則

**檔案位置：**
- 標頭檔：`src/chessboard.h`
- 實作檔：`src/chessboard.cpp`

**主要功能：**

這個類別 `ChessBoard` 管理整個棋盤狀態和遊戲規則。

**程式碼結構說明：**

```cpp
class ChessBoard {
public:
    ChessBoard();
    
    void initializeBoard();  // 初始化棋盤到起始狀態
    
    // 棋子存取
    const ChessPiece& getPiece(int row, int col) const;
    ChessPiece& getPiece(int row, int col);
    void setPiece(int row, int col, const ChessPiece& piece);
    
    // 移動和驗證
    bool movePiece(const QPoint& from, const QPoint& to);  // 執行移動
    bool isValidMove(const QPoint& from, const QPoint& to) const;  // 檢查移動是否合法
    
    // 遊戲狀態查詢
    PieceColor getCurrentPlayer() const { return m_currentPlayer; }
    void setCurrentPlayer(PieceColor player) { m_currentPlayer = player; }
    bool isInCheck(PieceColor color) const;        // 是否被將軍
    bool isCheckmate(PieceColor color) const;      // 是否被將死
    bool isStalemate(PieceColor color) const;      // 是否僵局
    bool isInsufficientMaterial() const;           // 是否材料不足無法將死
    
    QPoint findKing(PieceColor color) const;       // 尋找國王位置
    QPoint getEnPassantTarget() const { return m_enPassantTarget; }
    
    // 升變
    bool needsPromotion(const QPoint& to) const;   // 是否需要兵升變
    void promotePawn(const QPoint& pos, PieceType newType);  // 執行升變
    
private:
    std::vector<std::vector<ChessPiece>> m_board;  // 8x8 棋盤
    PieceColor m_currentPlayer;                    // 目前輪到的玩家
    QPoint m_enPassantTarget;                      // 吃過路兵目標位置
    std::vector<MoveRecord> m_moveHistory;         // 棋步歷史記錄
    GameResult m_gameResult;                       // 遊戲結果
    
    void switchPlayer();  // 切換玩家
    bool wouldBeInCheck(const QPoint& from, const QPoint& to, PieceColor color) const;
    bool hasAnyValidMoves(PieceColor color) const;
    bool canPieceMove(const QPoint& pos) const;
    bool canCastle(const QPoint& from, const QPoint& to) const;  // 檢查王車易位
};
```

**重要方法詳解：**

#### 1. `isValidMove()` - 完整的移動驗證

這個方法不僅檢查棋子的基本移動規則，還檢查進階規則：

```cpp
bool ChessBoard::isValidMove(const QPoint& from, const QPoint& to) const {
    // 1. 基本檢查
    if (from == to) return false;
    if (from.x() < 0 || from.x() >= 8 || from.y() < 0 || from.y() >= 8) return false;
    if (to.x() < 0 || to.x() >= 8 || to.y() < 0 || to.y() >= 8) return false;
    
    const ChessPiece& piece = getPiece(from.y(), from.x());
    
    // 2. 檢查是否是空格或不是目前玩家的棋子
    if (piece.getType() == PieceType::None) return false;
    if (piece.getColor() != m_currentPlayer) return false;
    
    // 3. 檢查棋子的基本移動規則
    if (!piece.isValidMove(from, to, m_board, m_enPassantTarget)) {
        // 4. 特殊情況：王車易位
        if (piece.getType() == PieceType::King && canCastle(from, to)) {
            return true;
        }
        return false;
    }
    
    // 5. 檢查移動後是否會導致自己被將軍（不允許）
    if (wouldBeInCheck(from, to, m_currentPlayer)) {
        return false;
    }
    
    return true;
}
```

#### 2. `isInCheck()` - 將軍檢測

```cpp
bool ChessBoard::isInCheck(PieceColor color) const {
    QPoint kingPos = findKing(color);
    if (kingPos.x() < 0) return false;  // 找不到國王
    
    PieceColor opponentColor = (color == PieceColor::White) ? PieceColor::Black : PieceColor::White;
    
    // 檢查是否有任何對手的棋子可以吃掉國王
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const ChessPiece& piece = m_board[row][col];
            if (piece.getColor() == opponentColor && piece.getType() != PieceType::None) {
                QPoint from(col, row);
                if (piece.isValidMove(from, kingPos, m_board, m_enPassantTarget)) {
                    return true;  // 國王被將軍
                }
            }
        }
    }
    
    return false;
}
```

**說明：**
- 遍歷所有對手的棋子
- 檢查是否有任何棋子可以吃掉國王
- 用於驗證移動後是否合法

#### 3. `isCheckmate()` - 將死檢測

```cpp
bool ChessBoard::isCheckmate(PieceColor color) const {
    // 必須先被將軍
    if (!isInCheck(color)) return false;
    
    // 檢查是否有任何合法移動可以解除將軍
    return !hasAnyValidMoves(color);
}
```

**說明：**
- 被將軍且沒有任何合法移動 = 將死
- 遊戲結束，對手獲勝

#### 4. `isStalemate()` - 僵局檢測

```cpp
bool ChessBoard::isStalemate(PieceColor color) const {
    // 必須沒有被將軍
    if (isInCheck(color)) return false;
    
    // 但是沒有任何合法移動
    return !hasAnyValidMoves(color);
}
```

**說明：**
- 沒有被將軍但也沒有合法移動 = 僵局
- 遊戲結束，平手

#### 5. `canCastle()` - 王車易位檢查

王車易位是一個特殊的移動，需要滿足多個條件：

```cpp
bool ChessBoard::canCastle(const QPoint& from, const QPoint& to) const {
    const ChessPiece& king = getPiece(from.y(), from.x());
    
    // 1. 必須是國王
    if (king.getType() != PieceType::King) return false;
    
    // 2. 國王和城堡都不能移動過
    if (king.hasMoved()) return false;
    
    // 3. 國王不能在被將軍狀態
    if (isInCheck(king.getColor())) return false;
    
    // 4. 檢查是王翼易位還是后翼易位
    int dx = to.x() - from.x();
    if (abs(dx) != 2) return false;  // 國王必須移動 2 格
    
    // 5. 找到對應的城堡
    int rookCol = (dx > 0) ? 7 : 0;  // 王翼或后翼
    const ChessPiece& rook = getPiece(from.y(), rookCol);
    
    // 6. 城堡必須存在且未移動過
    if (rook.getType() != PieceType::Rook || rook.hasMoved()) return false;
    
    // 7. 國王和城堡之間不能有棋子
    int step = (dx > 0) ? 1 : -1;
    for (int x = from.x() + step; x != rookCol; x += step) {
        if (getPiece(from.y(), x).getType() != PieceType::None) return false;
    }
    
    // 8. 國王移動經過的位置不能被將軍
    for (int x = from.x(); x != to.x() + step; x += step) {
        // 模擬國王移動到該位置
        if (wouldBeInCheck(from, QPoint(x, from.y()), king.getColor())) {
            return false;
        }
    }
    
    return true;
}
```

**說明：**
- 王車易位是最複雜的特殊移動
- 需要同時檢查多個條件
- 國王移動 2 格，城堡跳到國王另一側

---

## 總結

### 設定功能
- **音效設定**：`src/soundsettingsdialog.h/cpp` - 管理遊戲音效的自訂和播放
- **棋子圖標設定**：`src/pieceiconsettingsdialog.h/cpp` - 管理棋子圖示的外觀
- **棋盤顏色設定**：`src/boardcolorsettingsdialog.h/cpp` - 管理棋盤的配色方案

### 棋子判斷功能
- **棋子類別**：`src/chesspiece.h/cpp` - 定義棋子類型和基本移動規則
- **棋盤類別**：`src/chessboard.h/cpp` - 管理遊戲狀態和進階規則（將軍、將死、王車易位等）

這些類別共同實現了一個功能完整的西洋棋遊戲，包含所有標準規則和豐富的自訂選項。
