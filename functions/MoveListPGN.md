# MoveListPGN 棋譜記錄與PGN匯出功能

## 概述
棋譜記錄功能自動追蹤每一步移動，使用標準代數記譜法（Algebraic Notation）顯示在棋譜列表中。PGN（Portable Game Notation）匯出功能可將完整對局儲存為標準格式，方便分享和分析。

## 主要檔案
功能整合在主遊戲類別和棋盤類別中：
- **移動記錄**: `src/chessboard.cpp` - `MoveRecord` 結構
- **棋譜顯示**: `src/qt_chess.cpp` - 棋譜列表 UI
- **PGN 匯出**: `src/qt_chess.cpp` - PGN 生成邏輯

## 移動記錄

### MoveRecord 結構
```cpp
struct MoveRecord {
    QPoint from;                    // 起始位置
    QPoint to;                      // 目標位置
    PieceType pieceType;            // 移動的棋子類型
    PieceColor pieceColor;          // 棋子顏色
    bool isCapture;                 // 是否吃子
    bool isCastling;                // 是否王車易位
    bool isEnPassant;               // 是否吃過路兵
    bool isPromotion;               // 是否兵升變
    PieceType promotionType;        // 升變後的棋子類型
    bool isCheck;                   // 是否將軍
    bool isCheckmate;               // 是否將死
    QString algebraicNotation;      // 代數記譜法表示
};
```

### 記錄移動
```cpp
void ChessBoard::recordMove(const QPoint& from, const QPoint& to) {
    MoveRecord record;
    record.from = from;
    record.to = to;
    
    ChessPiece piece = getPiece(from.y(), from.x());
    record.pieceType = piece.getType();
    record.pieceColor = piece.getColor();
    
    // 檢查各種特殊走法
    record.isCastling = detectCastling(from, to);
    record.isEnPassant = (to == m_enPassantTarget && piece.getType() == PieceType::Pawn);
    record.isCapture = (getPiece(to.y(), to.x()).getType() != PieceType::None) || record.isEnPassant;
    record.isPromotion = false;  // 稍後在升變時更新
    record.promotionType = PieceType::None;
    
    // 生成代數記譜法
    record.algebraicNotation = generateAlgebraicNotation(record);
    
    // 檢查是否將軍/將死（在移動後）
    PieceColor opponent = (record.pieceColor == PieceColor::White) 
                          ? PieceColor::Black 
                          : PieceColor::White;
    record.isCheck = isInCheck(opponent);
    record.isCheckmate = isCheckmate(opponent);
    
    // 更新記譜法（加上將軍/將死符號）
    if (record.isCheckmate) {
        record.algebraicNotation += "#";
    } else if (record.isCheck) {
        record.algebraicNotation += "+";
    }
    
    // 加入歷史記錄
    m_moveHistory.push_back(record);
}
```

## 代數記譜法 (Algebraic Notation)

### 格式規則

#### 基本移動
- 兵: 只寫目標格 (如 `e4`)
- 其他棋子: 棋子符號 + 目標格 (如 `Nf3`, `Bb5`)

#### 棋子符號
- K = King (國王)
- Q = Queen (皇后)
- R = Rook (城堡)
- B = Bishop (主教)
- N = Knight (騎士)
- (無符號) = Pawn (兵)

#### 吃子
- 用 `x` 表示吃子 (如 `Nxe5`)
- 兵吃子要加起始列 (如 `exd5`)

#### 王車易位
- 王翼易位: `O-O`
- 后翼易位: `O-O-O`

#### 兵升變
- 格式: 目標格 `=` 升變棋子 (如 `e8=Q`)

#### 將軍與將死
- 將軍: 加 `+` (如 `Qh5+`)
- 將死: 加 `#` (如 `Qxf7#`)

#### 消歧義
當多個同類棋子可以移動到相同位置時，需要加入額外資訊：
- 加入起始列: `Nbd7` (b列的騎士到d7)
- 加入起始行: `R1e1` (第1行的城堡到e1)
- 同時加入: `Qh4e1` (完整起始位置)

### 生成代數記譜法
```cpp
QString ChessBoard::generateAlgebraicNotation(const MoveRecord& move) {
    QString notation;
    
    // 王車易位特殊處理
    if (move.isCastling) {
        int dx = move.to.x() - move.from.x();
        return (dx > 0) ? "O-O" : "O-O-O";
    }
    
    // 棋子符號（兵不加符號）
    if (move.pieceType != PieceType::Pawn) {
        notation += getPieceSymbol(move.pieceType);
    }
    
    // 消歧義檢查
    if (move.pieceType != PieceType::Pawn) {
        QString disambig = getDisambiguation(move);
        notation += disambig;
    }
    
    // 吃子符號
    if (move.isCapture) {
        // 兵吃子要加起始列
        if (move.pieceType == PieceType::Pawn) {
            notation += QChar('a' + move.from.x());
        }
        notation += "x";
    }
    
    // 目標位置
    notation += positionToAlgebraic(move.to);
    
    // 兵升變
    if (move.isPromotion) {
        notation += "=";
        notation += getPieceSymbol(move.promotionType);
    }
    
    // 吃過路兵標記（可選）
    if (move.isEnPassant) {
        notation += " e.p.";
    }
    
    return notation;
}

// 輔助函數
QString getPieceSymbol(PieceType type) {
    switch (type) {
        case PieceType::King:   return "K";
        case PieceType::Queen:  return "Q";
        case PieceType::Rook:   return "R";
        case PieceType::Bishop: return "B";
        case PieceType::Knight: return "N";
        default: return "";
    }
}

QString positionToAlgebraic(const QPoint& pos) {
    QChar file = QChar('a' + pos.x());
    QChar rank = QChar('1' + (7 - pos.y()));
    return QString("%1%2").arg(file).arg(rank);
}

QString getDisambiguation(const MoveRecord& move) {
    // 檢查是否有其他同類棋子也能移動到目標位置
    std::vector<QPoint> candidates;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            QPoint pos(col, row);
            if (pos == move.from) continue;
            
            ChessPiece piece = getPiece(row, col);
            if (piece.getType() == move.pieceType && 
                piece.getColor() == move.pieceColor &&
                isValidMove(pos, move.to)) {
                candidates.push_back(pos);
            }
        }
    }
    
    if (candidates.empty()) {
        return "";  // 不需要消歧義
    }
    
    // 檢查是否需要列消歧義
    bool needFile = false;
    for (const auto& cand : candidates) {
        if (cand.x() != move.from.x()) {
            needFile = true;
            break;
        }
    }
    
    if (needFile) {
        return QString(QChar('a' + move.from.x()));
    }
    
    // 需要行消歧義
    return QString(QChar('1' + (7 - move.from.y())));
}
```

## 棋譜列表顯示

### UI 元件
```cpp
class Qt_Chess {
private:
    QListWidget* m_moveListWidget;
    QLabel* m_moveListLabel;
    
public:
    void setupMoveList() {
        m_moveListLabel = new QLabel("棋譜:");
        
        m_moveListWidget = new QListWidget();
        m_moveListWidget->setMinimumWidth(150);
        m_moveListWidget->setMaximumWidth(200);
        
        // 雙擊進入回放模式
        connect(m_moveListWidget, &QListWidget::itemDoubleClicked,
                this, &Qt_Chess::onMoveListItemDoubleClicked);
    }
};
```

### 更新棋譜列表
```cpp
void Qt_Chess::updateMoveList() {
    m_moveListWidget->clear();
    
    const auto& history = m_board->getMoveHistory();
    
    for (size_t i = 0; i < history.size(); i++) {
        const MoveRecord& move = history[i];
        
        QString moveText;
        if (i % 2 == 0) {
            // 白方移動，加上回合數
            int moveNumber = (i / 2) + 1;
            moveText = QString("%1. %2").arg(moveNumber).arg(move.algebraicNotation);
        } else {
            // 黑方移動
            moveText = QString("   %1").arg(move.algebraicNotation);
        }
        
        m_moveListWidget->addItem(moveText);
    }
    
    // 自動滾動到最新移動
    m_moveListWidget->scrollToBottom();
}
```

### 格式化顯示
```
1. e4     e5
2. Nf3    Nc6
3. Bb5    a6
4. Ba4    Nf6
5. O-O    Be7
6. Re1    b5
7. Bb3    d6
8. c3     O-O
```

## PGN 格式

### PGN 結構
PGN 檔案包含兩部分：
1. **標籤區域**: 遊戲資訊（元資料）
2. **移動區域**: 實際移動序列

### 標準標籤
```
[Event "?"]
[Site "?"]
[Date "2024.01.15"]
[Round "?"]
[White "玩家1"]
[Black "玩家2"]
[Result "1-0"]
```

#### 可選標籤
```
[TimeControl "300+5"]
[ECO "C50"]
[Opening "Italian Game"]
[Termination "Normal"]
```

### 生成 PGN
```cpp
QString Qt_Chess::generatePGN() {
    QString pgn;
    
    // 1. 標籤區域
    pgn += "[Event \"Qt Chess Game\"]\n";
    pgn += "[Site \"Local Computer\"]\n";
    pgn += QString("[Date \"%1\"]\n").arg(QDate::currentDate().toString("yyyy.MM.dd"));
    pgn += "[Round \"?\"]\n";
    
    // 玩家名稱
    QString whiteName = getWhitePlayerName();  // 從設定或使用者輸入
    QString blackName = getBlackPlayerName();
    pgn += QString("[White \"%1\"]\n").arg(whiteName);
    pgn += QString("[Black \"%1\"]\n").arg(blackName);
    
    // 遊戲結果
    QString result = getGameResult();
    pgn += QString("[Result \"%1\"]\n").arg(result);
    
    // 時間控制（如果有）
    if (m_totalTimeSeconds > 0) {
        pgn += QString("[TimeControl \"%1+%2\"]\n")
                   .arg(m_totalTimeSeconds)
                   .arg(m_incrementSeconds);
    }
    
    // 空行分隔
    pgn += "\n";
    
    // 2. 移動區域
    const auto& history = m_board->getMoveHistory();
    
    for (size_t i = 0; i < history.size(); i++) {
        const MoveRecord& move = history[i];
        
        if (i % 2 == 0) {
            // 白方移動，加上回合數
            int moveNumber = (i / 2) + 1;
            pgn += QString("%1. %2 ").arg(moveNumber).arg(move.algebraicNotation);
        } else {
            // 黑方移動
            pgn += QString("%1 ").arg(move.algebraicNotation);
            
            // 每兩回合換行（可選）
            if ((i + 1) % 6 == 0) {
                pgn += "\n";
            }
        }
    }
    
    // 添加結果
    pgn += result;
    
    return pgn;
}

QString Qt_Chess::getGameResult() {
    switch (m_board->getGameResult()) {
        case GameResult::WhiteWins:
        case GameResult::BlackResigns:
            return "1-0";
        case GameResult::BlackWins:
        case GameResult::WhiteResigns:
            return "0-1";
        case GameResult::Draw:
            return "1/2-1/2";
        default:
            return "*";  // 進行中或未知
    }
}
```

### PGN 範例
```
[Event "Qt Chess Game"]
[Site "Local Computer"]
[Date "2024.01.15"]
[Round "?"]
[White "Alice"]
[Black "Bob"]
[Result "1-0"]
[TimeControl "300+5"]

1. e4 e5 2. Nf3 Nc6 3. Bb5 a6 4. Ba4 Nf6 5. O-O Be7 6. Re1 b5 
7. Bb3 d6 8. c3 O-O 9. h3 Na5 10. Bc2 c5 11. d4 Qc7 12. Nbd2 Nc6 
13. dxe5 dxe5 14. Nf1 Be6 15. Ng3 Rad8 16. Qe2 c4 17. Bg5 Bc5 
18. Bxf6 gxf6 19. Nh5 Kh8 20. Nxf6 Rg8 21. Qh5 Rg7 22. Nxh7 1-0
```

## PGN 匯出功能

### 匯出到檔案
```cpp
void Qt_Chess::exportPGN() {
    // 生成 PGN 內容
    QString pgn = generatePGN();
    
    // 開啟儲存對話框
    QString filename = QFileDialog::getSaveFileName(
        this,
        "匯出 PGN",
        QDir::homePath() + "/chess_game_" + 
            QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".pgn",
        "PGN 檔案 (*.pgn);;所有檔案 (*.*)"
    );
    
    if (filename.isEmpty()) {
        return;  // 使用者取消
    }
    
    // 寫入檔案
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");  // 支援中文玩家名稱
        out << pgn;
        file.close();
        
        QMessageBox::information(
            this,
            "匯出成功",
            QString("PGN 檔案已儲存至:\n%1").arg(filename)
        );
    } else {
        QMessageBox::critical(
            this,
            "匯出失敗",
            "無法寫入檔案"
        );
    }
}
```

### 複製到剪貼簿
```cpp
void Qt_Chess::copyMoveListToClipboard() {
    QString moveList;
    
    const auto& history = m_board->getMoveHistory();
    
    for (size_t i = 0; i < history.size(); i++) {
        const MoveRecord& move = history[i];
        
        if (i % 2 == 0) {
            int moveNumber = (i / 2) + 1;
            moveList += QString("%1. %2 ").arg(moveNumber).arg(move.algebraicNotation);
        } else {
            moveList += QString("%1\n").arg(move.algebraicNotation);
        }
    }
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(moveList);
    
    statusBar()->showMessage("棋譜已複製到剪貼簿", 2000);
}
```

## PGN 匯入（載入）

### 解析 PGN
```cpp
bool Qt_Chess::loadPGN(const QString& pgnContent) {
    // 解析標籤
    QMap<QString, QString> tags = parsePGNTags(pgnContent);
    
    // 解析移動
    QString moveText = extractPGNMoves(pgnContent);
    QStringList moves = parsePGNMoves(moveText);
    
    // 重置棋盤
    m_board->initializeBoard();
    
    // 重播移動
    for (const QString& move : moves) {
        if (!playMoveFromAlgebraic(move)) {
            QMessageBox::warning(
                this,
                "載入失敗",
                QString("無法執行移動: %1").arg(move)
            );
            return false;
        }
    }
    
    // 更新 UI
    updateBoardDisplay();
    updateMoveList();
    
    return true;
}

QMap<QString, QString> parsePGNTags(const QString& pgn) {
    QMap<QString, QString> tags;
    QRegularExpression tagRe("\\[([A-Za-z]+)\\s+\"([^\"]+)\"\\]");
    
    QRegularExpressionMatchIterator it = tagRe.globalMatch(pgn);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString key = match.captured(1);
        QString value = match.captured(2);
        tags[key] = value;
    }
    
    return tags;
}

QStringList parsePGNMoves(const QString& moveText) {
    QStringList moves;
    
    // 移除回合編號、結果標記等
    QString cleaned = moveText;
    cleaned.remove(QRegularExpression("\\d+\\."));  // 移除回合數
    cleaned.remove(QRegularExpression("[\\*1-0]"));  // 移除結果
    
    // 分割移動
    QStringList tokens = cleaned.split(QRegularExpression("\\s+"), 
                                       QString::SkipEmptyParts);
    
    for (const QString& token : tokens) {
        if (!token.isEmpty() && token != "1/2-1/2") {
            moves.append(token);
        }
    }
    
    return moves;
}
```

## 註解與變化

### 添加註解（擴展功能）
```cpp
struct AnnotatedMove : public MoveRecord {
    QString comment;         // 移動註解
    QString evaluation;      // 評估符號 (!, !!, ?, ??, !?, ?!)
    int centipawnScore;      // 引擎評估分數（可選）
    std::vector<AnnotatedMove> variations;  // 變化分支
};
```

### 註解符號
- `!` - 好棋
- `!!` - 妙棋
- `?` - 可疑走法
- `??` - 失誤
- `!?` - 有趣的走法
- `?!` - 值得懷疑的走法

### 帶註解的 PGN
```
1. e4 e5 2. Nf3 Nc6 3. Bb5 {Ruy Lopez} a6 4. Ba4 Nf6 
5. O-O! {最佳走法} Be7 6. Re1 b5 7. Bb3 d6 8. c3 O-O 
```

## UI 整合

### 按鈕配置
```cpp
void Qt_Chess::setupMoveListButtons() {
    QPushButton* exportButton = new QPushButton("匯出 PGN");
    connect(exportButton, &QPushButton::clicked, this, &Qt_Chess::exportPGN);
    
    QPushButton* copyButton = new QPushButton("複製棋譜");
    connect(copyButton, &QPushButton::clicked, 
            this, &Qt_Chess::copyMoveListToClipboard);
    
    QPushButton* importButton = new QPushButton("載入 PGN");
    connect(importButton, &QPushButton::clicked, this, &Qt_Chess::importPGN);
}
```

## 相關文檔
- 棋譜與 PGN: `docs/MOVE_LIST_PGN_FEATURE.md`
- 回放功能: `docs/REPLAY_FEATURE.md`

## 相關類別
- `ChessBoard` - 儲存移動歷史
- `Qt_Chess` - 顯示棋譜和處理 PGN

## 測試要點
1. 所有移動正確記錄
2. 代數記譜法格式正確
3. 特殊走法（易位、吃過路兵、升變）正確標記
4. PGN 格式符合標準
5. 匯入的 PGN 可以正確重播
6. 認輸正確記錄在結果中

## 參考資源
- [PGN 格式標準](https://en.wikipedia.org/wiki/Portable_Game_Notation)
- [代數記譜法](https://en.wikipedia.org/wiki/Algebraic_notation_(chess))
- [FIDE 記譜規則](https://www.fide.com/FIDE/handbook/LawsOfChess.pdf)
