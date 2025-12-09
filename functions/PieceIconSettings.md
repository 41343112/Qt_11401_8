# PieceIconSettings 棋子圖標設定功能

## 概述
`PieceIconSettingsDialog` 類別提供完整的棋子圖標自訂功能，允許使用者從多種預設圖標集中選擇，或上傳自訂圖片作為棋子圖標。同時支援棋子大小調整（60%-100%），讓使用者可以根據喜好自訂棋盤外觀。

## 檔案位置
- **標頭檔**: `src/pieceiconsettingsdialog.h`
- **實作檔**: `src/pieceiconsettingsdialog.cpp`

## 主要資料結構

### IconSetType - 圖標集類型
```cpp
enum class IconSetType {
    Unicode,      // 預設 Unicode 符號（♔♕♖♗♘♙）
    Preset1,      // 預設圖標集 1
    Preset2,      // 預設圖標集 2
    Preset3,      // 預設圖標集 3
    Custom        // 自訂使用者選擇的圖標
};
```

### PieceIconSettings 結構
```cpp
struct PieceIconSettings {
    // 白色棋子圖標路徑
    QString whiteKingIcon;
    QString whiteQueenIcon;
    QString whiteRookIcon;
    QString whiteBishopIcon;
    QString whiteKnightIcon;
    QString whitePawnIcon;
    
    // 黑色棋子圖標路徑
    QString blackKingIcon;
    QString blackQueenIcon;
    QString blackRookIcon;
    QString blackBishopIcon;
    QString blackKnightIcon;
    QString blackPawnIcon;
    
    bool useCustomIcons;        // 是否使用自訂圖標
    IconSetType iconSetType;    // 當前圖標集類型
    int pieceScale;             // 棋子縮放百分比（60-100）
};
```

## 核心功能

### 1. 圖標集選擇

#### 預設圖標集
```cpp
static PieceIconSettings getPresetSettings(IconSetType setType) {
    PieceIconSettings settings;
    
    switch (setType) {
        case IconSetType::Unicode:
            // 使用 Unicode 符號（空字串表示使用預設）
            settings.whiteKingIcon = "";
            settings.whiteQueenIcon = "";
            // ... 其他棋子都為空
            settings.useCustomIcons = false;
            break;
            
        case IconSetType::Preset1:
            settings.whiteKingIcon = ":/icons/preset1/white_king.png";
            settings.whiteQueenIcon = ":/icons/preset1/white_queen.png";
            settings.whiteRookIcon = ":/icons/preset1/white_rook.png";
            // ... 其他棋子
            settings.useCustomIcons = true;
            break;
            
        case IconSetType::Preset2:
            settings.whiteKingIcon = ":/icons/preset2/white_king.png";
            // ... 類似設定
            break;
            
        case IconSetType::Preset3:
            settings.whiteKingIcon = ":/icons/preset3/white_king.png";
            // ... 類似設定
            break;
            
        case IconSetType::Custom:
            // 保留當前自訂設定
            settings.useCustomIcons = true;
            break;
    }
    
    settings.iconSetType = setType;
    settings.pieceScale = 80;  // 預設 80%
    
    return settings;
}
```

#### 圖標集切換
```cpp
void PieceIconSettingsDialog::onIconSetTypeChanged(int index) {
    IconSetType type = static_cast<IconSetType>(index);
    
    if (type == IconSetType::Custom) {
        // 顯示自訂圖標的瀏覽按鈕
        showCustomIconControls(true);
    } else {
        // 隱藏瀏覽按鈕，套用預設圖標集
        showCustomIconControls(false);
        m_settings = getPresetSettings(type);
        updateAllPreviews();
    }
}
```

### 2. 自訂圖標上傳

#### 瀏覽圖片檔案
```cpp
QString PieceIconSettingsDialog::browseForIconFile() {
    QString filename = QFileDialog::getOpenFileName(
        this,
        "選擇棋子圖標",
        QDir::homePath(),
        "圖片檔案 (*.png *.jpg *.jpeg *.svg *.bmp);;所有檔案 (*.*)"
    );
    return filename;
}
```

**支援的格式**:
- **PNG** (建議，支援透明背景)
- **SVG** (向量圖，可縮放)
- **JPG/JPEG**
- **BMP**

#### 單一棋子圖標設定
```cpp
void PieceIconSettingsDialog::onBrowseWhiteKing() {
    QString filename = browseForIconFile();
    if (!filename.isEmpty()) {
        // 驗證檔案
        if (!validateIconFile(filename)) {
            QMessageBox::warning(this, "錯誤", "無效的圖片檔案");
            return;
        }
        
        // 更新設定
        m_settings.whiteKingIcon = filename;
        m_whiteKingEdit->setText(filename);
        
        // 更新預覽
        updatePreview(m_whiteKingPreview, filename);
    }
}
```

#### 批次設定（全白/全黑）
```cpp
void PieceIconSettingsDialog::onBrowseAllWhitePieces() {
    QMessageBox::information(
        this,
        "批次上傳",
        "請依序選擇：國王、皇后、城堡、主教、騎士、兵的圖標"
    );
    
    QString pieces[] = {"國王", "皇后", "城堡", "主教", "騎士", "兵"};
    QString* icons[] = {
        &m_settings.whiteKingIcon,
        &m_settings.whiteQueenIcon,
        &m_settings.whiteRookIcon,
        &m_settings.whiteBishopIcon,
        &m_settings.whiteKnightIcon,
        &m_settings.whitePawnIcon
    };
    
    for (int i = 0; i < 6; i++) {
        QString filename = QFileDialog::getOpenFileName(
            this,
            QString("選擇白色%1圖標").arg(pieces[i]),
            QDir::homePath(),
            "圖片檔案 (*.png *.jpg *.svg);;所有檔案 (*.*)"
        );
        
        if (filename.isEmpty()) break;  // 使用者取消
        
        *icons[i] = filename;
    }
    
    updateAllPreviews();
}
```

### 3. 圖標預覽

#### 預覽顯示
```cpp
void PieceIconSettingsDialog::updatePreview(QLabel* previewLabel, 
                                            const QString& iconPath) {
    if (iconPath.isEmpty()) {
        // 顯示 Unicode 符號
        previewLabel->setText("♔");  // 或對應的符號
        previewLabel->setStyleSheet("font-size: 48px;");
    } else {
        // 載入並顯示圖片
        QPixmap pixmap(iconPath);
        if (pixmap.isNull()) {
            previewLabel->setText("?");
            return;
        }
        
        // 縮放到預覽大小
        pixmap = pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        previewLabel->setPixmap(pixmap);
    }
}
```

#### 預覽按鈕
```cpp
void PieceIconSettingsDialog::onPreviewWhiteKing() {
    // 在較大的對話框中顯示預覽
    QDialog previewDialog(this);
    previewDialog.setWindowTitle("預覽 - 白色國王");
    
    QVBoxLayout* layout = new QVBoxLayout(&previewDialog);
    QLabel* previewLabel = new QLabel();
    
    QPixmap pixmap(m_settings.whiteKingIcon);
    if (!pixmap.isNull()) {
        // 顯示原始大小
        previewLabel->setPixmap(pixmap);
    }
    
    layout->addWidget(previewLabel);
    previewDialog.exec();
}
```

### 4. 棋子大小調整

#### 大小滑桿
```cpp
void PieceIconSettingsDialog::setupScaleSlider() {
    QLabel* scaleLabel = new QLabel("棋子大小:");
    
    m_scaleSlider = new QSlider(Qt::Horizontal);
    m_scaleSlider->setMinimum(60);   // 最小 60%
    m_scaleSlider->setMaximum(100);  // 最大 100%
    m_scaleSlider->setValue(80);     // 預設 80%
    
    m_scaleValueLabel = new QLabel("80%");
    
    connect(m_scaleSlider, &QSlider::valueChanged, this, [this](int value) {
        m_settings.pieceScale = value;
        m_scaleValueLabel->setText(QString("%1%").arg(value));
        
        // 即時更新預覽（可選）
        updateAllPreviewsWithScale();
    });
}
```

#### 縮放應用
```cpp
QPixmap Qt_Chess::getScaledPieceIcon(const QString& iconPath, int squareSize) {
    QPixmap pixmap(iconPath);
    if (pixmap.isNull()) return QPixmap();
    
    // 根據設定的縮放比例調整大小
    int targetSize = squareSize * m_iconSettings.pieceScale / 100;
    
    return pixmap.scaled(targetSize, targetSize, 
                        Qt::KeepAspectRatio, 
                        Qt::SmoothTransformation);
}
```

### 5. 重設功能

#### 重設單一棋子
```cpp
void PieceIconSettingsDialog::onResetWhiteKing() {
    PieceIconSettings defaults = getDefaultSettings();
    m_settings.whiteKingIcon = defaults.whiteKingIcon;
    m_whiteKingEdit->setText(m_settings.whiteKingIcon);
    updatePreview(m_whiteKingPreview, m_settings.whiteKingIcon);
}
```

#### 重設所有圖標
```cpp
void PieceIconSettingsDialog::onResetToDefaults() {
    int result = QMessageBox::question(
        this,
        "重設為預設值",
        "確定要將所有棋子圖標重設為預設 Unicode 符號嗎？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (result == QMessageBox::Yes) {
        m_settings = getDefaultSettings();
        updateAllUI();
    }
}
```

## UI 佈局

### 圖標集選擇區域
```
┌─────────────────────────────────┐
│ 選擇圖標集:                     │
│ ○ Unicode 符號 (預設)           │
│ ○ 經典圖標集                    │
│ ○ 現代圖標集                    │
│ ○ 木質圖標集                    │
│ ● 自訂圖標                      │
└─────────────────────────────────┘
```

### 棋子大小設定
```
┌─────────────────────────────────┐
│ 棋子大小: [━━━━━━●━━━] 80%     │
└─────────────────────────────────┘
```

### 自訂圖標設定（僅在選擇「自訂圖標」時顯示）
```
┌──────────────────────────────────────────┐
│ 白色棋子:                                │
│ 國王: [路徑______] [瀏覽] [預覽] [重設] │
│       [預覽圖示]                         │
│ 皇后: [路徑______] [瀏覽] [預覽] [重設] │
│       [預覽圖示]                         │
│ ... (其他白色棋子) ...                   │
├──────────────────────────────────────────┤
│ 黑色棋子:                                │
│ 國王: [路徑______] [瀏覽] [預覽] [重設] │
│       [預覽圖示]                         │
│ ... (其他黑色棋子) ...                   │
└──────────────────────────────────────────┘
```

## 圖標資源管理

### 資源檔案結構
```
resources/
├── icons/
│   ├── preset1/
│   │   ├── white_king.png
│   │   ├── white_queen.png
│   │   ├── white_rook.png
│   │   ├── white_bishop.png
│   │   ├── white_knight.png
│   │   ├── white_pawn.png
│   │   ├── black_king.png
│   │   ├── black_queen.png
│   │   ├── black_rook.png
│   │   ├── black_bishop.png
│   │   ├── black_knight.png
│   │   └── black_pawn.png
│   ├── preset2/
│   │   └── ... (相同結構)
│   └── preset3/
│       └── ... (相同結構)
```

### 資源檔案配置 (resources.qrc)
```xml
<RCC>
    <qresource prefix="/icons">
        <!-- Preset 1 -->
        <file>preset1/white_king.png</file>
        <file>preset1/white_queen.png</file>
        <!-- ... 其他圖標 ... -->
        
        <!-- Preset 2 -->
        <file>preset2/white_king.png</file>
        <!-- ... -->
        
        <!-- Preset 3 -->
        <file>preset3/white_king.png</file>
        <!-- ... -->
    </qresource>
</RCC>
```

## 使用範例

### 開啟設定對話框
```cpp
void MainWindow::onPieceIconSettingsClicked() {
    PieceIconSettingsDialog dialog(this);
    
    // 載入當前設定
    dialog.setSettings(m_iconSettings);
    
    if (dialog.exec() == QDialog::Accepted) {
        m_iconSettings = dialog.getSettings();
        
        // 儲存設定
        saveIconSettings(m_iconSettings);
        
        // 重新繪製棋盤
        updateAllPieceIcons();
    }
}
```

### 應用圖標到棋盤
```cpp
void MainWindow::updatePieceIcon(int row, int col) {
    ChessPiece piece = m_board->getPiece(row, col);
    if (piece.getType() == PieceType::None) {
        m_squareButtons[row][col]->setIcon(QIcon());
        return;
    }
    
    QString iconPath = getIconPath(piece);
    
    if (iconPath.isEmpty()) {
        // 使用 Unicode 符號
        m_squareButtons[row][col]->setText(piece.getSymbol());
        m_squareButtons[row][col]->setIcon(QIcon());
    } else {
        // 使用圖標
        int squareSize = m_squareButtons[row][col]->width();
        QPixmap pixmap = getScaledPieceIcon(iconPath, squareSize);
        m_squareButtons[row][col]->setIcon(QIcon(pixmap));
        m_squareButtons[row][col]->setIconSize(pixmap.size());
        m_squareButtons[row][col]->setText("");
    }
}

QString MainWindow::getIconPath(const ChessPiece& piece) {
    if (!m_iconSettings.useCustomIcons) {
        return "";  // 使用 Unicode
    }
    
    PieceColor color = piece.getColor();
    PieceType type = piece.getType();
    
    if (color == PieceColor::White) {
        switch (type) {
            case PieceType::King:   return m_iconSettings.whiteKingIcon;
            case PieceType::Queen:  return m_iconSettings.whiteQueenIcon;
            case PieceType::Rook:   return m_iconSettings.whiteRookIcon;
            case PieceType::Bishop: return m_iconSettings.whiteBishopIcon;
            case PieceType::Knight: return m_iconSettings.whiteKnightIcon;
            case PieceType::Pawn:   return m_iconSettings.whitePawnIcon;
        }
    } else {
        switch (type) {
            case PieceType::King:   return m_iconSettings.blackKingIcon;
            case PieceType::Queen:  return m_iconSettings.blackQueenIcon;
            case PieceType::Rook:   return m_iconSettings.blackRookIcon;
            case PieceType::Bishop: return m_iconSettings.blackBishopIcon;
            case PieceType::Knight: return m_iconSettings.blackKnightIcon;
            case PieceType::Pawn:   return m_iconSettings.blackPawnIcon;
        }
    }
    
    return "";
}
```

### 儲存和載入設定
```cpp
void saveIconSettings(const PieceIconSettings& settings) {
    QSettings config("MyCompany", "Qt_Chess");
    
    config.setValue("icons/whiteKing", settings.whiteKingIcon);
    config.setValue("icons/whiteQueen", settings.whiteQueenIcon);
    // ... 其他棋子 ...
    
    config.setValue("icons/useCustom", settings.useCustomIcons);
    config.setValue("icons/setType", static_cast<int>(settings.iconSetType));
    config.setValue("icons/scale", settings.pieceScale);
}

PieceIconSettings loadIconSettings() {
    QSettings config("MyCompany", "Qt_Chess");
    PieceIconSettings settings = PieceIconSettingsDialog::getDefaultSettings();
    
    settings.whiteKingIcon = config.value("icons/whiteKing", "").toString();
    // ... 載入其他設定 ...
    
    settings.useCustomIcons = config.value("icons/useCustom", false).toBool();
    settings.iconSetType = static_cast<IconSetType>(
        config.value("icons/setType", 0).toInt()
    );
    settings.pieceScale = config.value("icons/scale", 80).toInt();
    
    return settings;
}
```

## 圖標設計建議

### 圖片規格
- **尺寸**: 128x128 或 256x256 像素
- **格式**: PNG（透明背景）或 SVG（向量圖）
- **風格**: 保持一致的風格和色調
- **對比度**: 確保在淺色和深色方格上都清晰可見

### 顏色建議
- **白色棋子**: 使用白色、米色或淺灰色
- **黑色棋子**: 使用黑色、深灰色或深棕色
- **描邊**: 添加對比色描邊以提高可見度

### 設計原則
- 清晰辨識：每種棋子應該容易區分
- 簡潔大方：避免過於複雜的細節
- 比例協調：所有棋子大小比例要一致

## 疑難排解

### 圖標無法顯示
```cpp
bool validateIconFile(const QString& path) {
    QFileInfo info(path);
    
    // 檢查檔案是否存在
    if (!info.exists()) {
        qWarning() << "圖標檔案不存在:" << path;
        return false;
    }
    
    // 檢查是否為有效的圖片
    QImageReader reader(path);
    if (!reader.canRead()) {
        qWarning() << "無法讀取圖標檔案:" << path;
        return false;
    }
    
    return true;
}
```

### 圖標模糊
- 使用更高解析度的圖片（256x256 或更高）
- 使用 SVG 向量圖格式
- 確保使用 `Qt::SmoothTransformation` 縮放

### 載入速度慢
- 預先快取縮放後的圖標
- 使用適當大小的圖片（不要太大）
- 考慮使用 QPixmapCache

## 相關文檔
- 添加圖標集指南: `docs/ADDING_ICON_SETS.md`
- 棋子大小視覺指南: `docs/PIECE_SIZE_VISUAL_GUIDE.md`
- 自訂圖標測試: `docs/TESTING_CUSTOM_ICONS.md`

## 相關類別
- `ChessPiece` - 定義棋子類型
- `Qt_Chess` - 使用圖標設定繪製棋盤

## 依賴項目
- **Qt Widgets**: `QDialog`, `QFileDialog`, `QLabel`, `QSlider`
- **Qt GUI**: `QPixmap`, `QIcon`, `QImageReader`
