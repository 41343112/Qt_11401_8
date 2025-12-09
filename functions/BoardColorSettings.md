# BoardColorSettings 棋盤顏色設定功能

## 概述
`BoardColorSettingsDialog` 類別提供完整的棋盤顏色自訂功能，包含多種預設配色方案、2×2 預覽網格、自訂顏色選擇器等。使用者可以輕鬆切換不同風格的棋盤配色，或建立自己獨特的配色方案。

## 檔案位置
- **標頭檔**: `src/boardcolorsettingsdialog.h`
- **實作檔**: `src/boardcolorsettingsdialog.cpp`

## 主要資料結構

### ColorScheme - 配色方案
```cpp
enum class ColorScheme {
    Classic,      // 經典棕色/米色（預設）
    BlueGray,     // 藍色和灰色
    GreenWhite,   // 綠色和白色
    PurplePink,   // 紫色和粉色
    WoodDark,     // 木色和深色
    OceanBlue,    // 海藍色
    LightTheme,   // 淺色柔和色
    Custom1,      // 自訂插槽 1
    Custom2,      // 自訂插槽 2
    Custom3,      // 自訂插槽 3
    Custom4,      // 自訂插槽 4
    Custom5,      // 自訂插槽 5
    Custom6,      // 自訂插槽 6
    Custom7       // 自訂插槽 7
};
```

### BoardColorSettings 結構
```cpp
struct BoardColorSettings {
    QColor lightSquareColor;  // 淺色方格顏色
    QColor darkSquareColor;   // 深色方格顏色
    ColorScheme scheme;       // 配色方案類型
    QString customName;       // 自訂配色方案名稱
};
```

## 預設配色方案

### 預設顏色定義
```cpp
static BoardColorSettings getPresetSettings(ColorScheme scheme) {
    BoardColorSettings settings;
    settings.scheme = scheme;
    
    switch (scheme) {
        case ColorScheme::Classic:
            // 經典棕褐色 - 傳統棋盤外觀
            settings.lightSquareColor = QColor("#F0D9B5");  // 淺棕色
            settings.darkSquareColor = QColor("#B58863");   // 深棕色
            settings.customName = "經典棕褐色";
            break;
            
        case ColorScheme::BlueGray:
            // 藍灰配色 - 現代冷色調
            settings.lightSquareColor = QColor("#E8EAF6");  // 淺藍灰
            settings.darkSquareColor = QColor("#7986CB");   // 深藍
            settings.customName = "藍灰配色";
            break;
            
        case ColorScheme::GreenWhite:
            // 綠白配色 - 清新自然
            settings.lightSquareColor = QColor("#FFFFCC");  // 淺黃
            settings.darkSquareColor = QColor("#86A666");   // 綠色
            settings.customName = "綠白配色";
            break;
            
        case ColorScheme::PurplePink:
            // 紫粉配色 - 優雅柔和
            settings.lightSquareColor = QColor("#F8BBD0");  // 粉紅色
            settings.darkSquareColor = QColor("#9C27B0");   // 紫色
            settings.customName = "紫粉配色";
            break;
            
        case ColorScheme::WoodDark:
            // 木紋深色 - 豐富木質感
            settings.lightSquareColor = QColor("#D7A86E");  // 木色
            settings.darkSquareColor = QColor("#8B4513");   // 深棕色
            settings.customName = "木紋深色";
            break;
            
        case ColorScheme::OceanBlue:
            // 海洋藍 - 深海風格
            settings.lightSquareColor = QColor("#B3E5FC");  // 淺藍
            settings.darkSquareColor = QColor("#0277BD");   // 深藍
            settings.customName = "海洋藍配色";
            break;
            
        case ColorScheme::LightTheme:
            // 淺色系 - 柔和簡約
            settings.lightSquareColor = QColor("#FFF9E6");  // 象牙白
            settings.darkSquareColor = QColor("#E0E0E0");   // 淺灰
            settings.customName = "淺色系";
            break;
            
        default:
            // 自訂配色，使用經典作為基礎
            return getPresetSettings(ColorScheme::Classic);
    }
    
    return settings;
}
```

### 配色方案特點

#### 經典棕褐色
- 溫暖的木質色調
- 最傳統的棋盤外觀
- 眼睛舒適，適合長時間對弈

#### 藍灰配色
- 現代科技感
- 冷色調減少眼睛疲勞
- 專業競技風格

#### 綠白配色
- 清新自然
- 高對比度，清晰易辨
- 類似公園棋盤

#### 紫粉配色
- 優雅浪漫
- 柔和色調
- 獨特個性化外觀

#### 木紋深色
- 深沉穩重
- 富有質感
- 適合喜歡深色主題的使用者

#### 海洋藍
- 清爽涼快
- 藍色系放鬆心情
- 現代簡約風格

#### 淺色系
- 極簡風格
- 低對比，柔和舒適
- 適合明亮環境

## UI 元件

### 顏色選擇按鈕
```cpp
void BoardColorSettingsDialog::setupColorButtons() {
    m_lightColorButton = new QPushButton("淺色方格");
    m_lightColorButton->setMinimumSize(100, 40);
    
    m_darkColorButton = new QPushButton("深色方格");
    m_darkColorButton->setMinimumSize(100, 40);
    
    connect(m_lightColorButton, &QPushButton::clicked, 
            this, &BoardColorSettingsDialog::onLightColorClicked);
    connect(m_darkColorButton, &QPushButton::clicked, 
            this, &BoardColorSettingsDialog::onDarkColorClicked);
    
    updateColorButtons();
}
```

### 更新顏色按鈕外觀
```cpp
void BoardColorSettingsDialog::updateColorButtons() {
    // 設定淺色按鈕背景
    QString lightStyle = QString(
        "background-color: %1; "
        "border: 2px solid #333; "
        "border-radius: 5px; "
        "font-weight: bold;"
    ).arg(m_settings.lightSquareColor.name());
    m_lightColorButton->setStyleSheet(lightStyle);
    
    // 設定深色按鈕背景
    QString darkStyle = QString(
        "background-color: %1; "
        "border: 2px solid #333; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "color: white;"
    ).arg(m_settings.darkSquareColor.name());
    m_darkColorButton->setStyleSheet(darkStyle);
}
```

### 2×2 預覽網格
```cpp
void BoardColorSettingsDialog::setupPreviewGrid() {
    m_previewWidget = new QWidget();
    QGridLayout* gridLayout = new QGridLayout(m_previewWidget);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    
    // 建立 2x2 方格預覽
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 2; col++) {
            m_previewSquares[row][col] = new QLabel();
            m_previewSquares[row][col]->setMinimumSize(80, 80);
            m_previewSquares[row][col]->setMaximumSize(80, 80);
            m_previewSquares[row][col]->setFrameStyle(QFrame::Box);
            gridLayout->addWidget(m_previewSquares[row][col], row, col);
        }
    }
    
    updatePreview();
}
```

### 更新預覽
```cpp
void BoardColorSettingsDialog::updatePreview() {
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 2; col++) {
            // 棋盤格子顏色交替
            bool isLight = (row + col) % 2 == 0;
            QColor color = isLight ? m_settings.lightSquareColor 
                                   : m_settings.darkSquareColor;
            
            QString style = QString("background-color: %1;").arg(color.name());
            m_previewSquares[row][col]->setStyleSheet(style);
        }
    }
}
```

### 預設方案預覽縮圖
```cpp
QPushButton* BoardColorSettingsDialog::createPresetPreview(
    ColorScheme scheme, const QString& label) {
    
    QPushButton* button = new QPushButton();
    button->setMinimumSize(PREVIEW_BUTTON_WIDTH, PREVIEW_BUTTON_HEIGHT);
    button->setMaximumSize(PREVIEW_BUTTON_WIDTH, PREVIEW_BUTTON_HEIGHT);
    button->setToolTip(label);
    
    // 建立 2x2 預覽圖
    QPixmap pixmap(PREVIEW_BUTTON_WIDTH, PREVIEW_BUTTON_HEIGHT);
    QPainter painter(&pixmap);
    
    BoardColorSettings settings = getPresetSettings(scheme);
    
    int squareSize = PREVIEW_SQUARE_SIZE;
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 2; col++) {
            bool isLight = (row + col) % 2 == 0;
            QColor color = isLight ? settings.lightSquareColor 
                                   : settings.darkSquareColor;
            painter.fillRect(col * squareSize, row * squareSize, 
                           squareSize, squareSize, color);
        }
    }
    
    // 添加標籤
    painter.setPen(Qt::black);
    painter.drawText(0, squareSize * 2 + 15, label);
    
    button->setIcon(QIcon(pixmap));
    button->setIconSize(pixmap.size());
    
    connect(button, &QPushButton::clicked, this, [this, scheme]() {
        applyPresetColorScheme(scheme);
    });
    
    return button;
}
```

## 顏色選擇功能

### 開啟顏色選擇器
```cpp
void BoardColorSettingsDialog::onLightColorClicked() {
    QColor color = QColorDialog::getColor(
        m_settings.lightSquareColor,
        this,
        "選擇淺色方格顏色",
        QColorDialog::ShowAlphaChannel
    );
    
    if (color.isValid()) {
        m_settings.lightSquareColor = color;
        m_settings.scheme = ColorScheme::Custom1;  // 標記為自訂
        updateColorButtons();
        updatePreview();
    }
}

void BoardColorSettingsDialog::onDarkColorClicked() {
    QColor color = QColorDialog::getColor(
        m_settings.darkSquareColor,
        this,
        "選擇深色方格顏色",
        QColorDialog::ShowAlphaChannel
    );
    
    if (color.isValid()) {
        m_settings.darkSquareColor = color;
        m_settings.scheme = ColorScheme::Custom1;
        updateColorButtons();
        updatePreview();
    }
}
```

### 套用預設配色
```cpp
void BoardColorSettingsDialog::applyPresetColorScheme(ColorScheme scheme) {
    m_settings = getPresetSettings(scheme);
    updateColorButtons();
    updatePreview();
}
```

## 自訂配色插槽

### 儲存自訂配色
```cpp
void BoardColorSettingsDialog::saveToCustomSlot(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= 7) return;
    
    m_customSlots[slotIndex] = m_settings;
    m_customSlots[slotIndex].scheme = static_cast<ColorScheme>(
        static_cast<int>(ColorScheme::Custom1) + slotIndex
    );
    
    // 詢問配色名稱
    bool ok;
    QString name = QInputDialog::getText(
        this,
        "儲存自訂配色",
        "請輸入配色名稱:",
        QLineEdit::Normal,
        m_settings.customName,
        &ok
    );
    
    if (ok && !name.isEmpty()) {
        m_customSlots[slotIndex].customName = name;
    }
    
    // 更新自訂配色預覽按鈕
    updateCustomSlotPreview(slotIndex);
}
```

### 載入自訂配色
```cpp
void BoardColorSettingsDialog::loadFromCustomSlot(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= 7) return;
    
    m_settings = m_customSlots[slotIndex];
    updateColorButtons();
    updatePreview();
}
```

## 應用到棋盤

### 更新棋盤顏色
```cpp
void Qt_Chess::applyBoardColors(const BoardColorSettings& settings) {
    m_boardColorSettings = settings;
    
    // 更新所有方格的顏色
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            updateSquareColor(row, col);
        }
    }
}

void Qt_Chess::updateSquareColor(int row, int col) {
    bool isLight = (row + col) % 2 == 0;
    QColor color = isLight ? m_boardColorSettings.lightSquareColor 
                           : m_boardColorSettings.darkSquareColor;
    
    QString style = QString(
        "background-color: %1; "
        "border: none;"
    ).arg(color.name());
    
    m_squareButtons[row][col]->setStyleSheet(style);
}
```

### 處理選中狀態
```cpp
void Qt_Chess::highlightSquare(int row, int col, HighlightType type) {
    bool isLight = (row + col) % 2 == 0;
    QColor baseColor = isLight ? m_boardColorSettings.lightSquareColor 
                                : m_boardColorSettings.darkSquareColor;
    
    QColor highlightColor;
    switch (type) {
        case HighlightType::Selected:
            // 選中的棋子 - 橙色
            highlightColor = QColor("#FFA500");
            break;
        case HighlightType::ValidMove:
            // 有效移動 - 淺綠色
            highlightColor = baseColor.lighter(120);
            break;
        case HighlightType::ValidCapture:
            // 可吃子 - 紅色
            highlightColor = QColor("#FF6B6B");
            break;
    }
    
    QString style = QString("background-color: %1;").arg(highlightColor.name());
    m_squareButtons[row][col]->setStyleSheet(style);
}
```

## 持久化設定

### 儲存設定
```cpp
void saveBoardColorSettings(const BoardColorSettings& settings) {
    QSettings config("MyCompany", "Qt_Chess");
    
    config.setValue("board/lightColor", settings.lightSquareColor.name());
    config.setValue("board/darkColor", settings.darkSquareColor.name());
    config.setValue("board/scheme", static_cast<int>(settings.scheme));
    config.setValue("board/customName", settings.customName);
    
    // 儲存自訂插槽
    for (int i = 0; i < 7; i++) {
        QString prefix = QString("board/custom%1/").arg(i);
        config.setValue(prefix + "light", m_customSlots[i].lightSquareColor.name());
        config.setValue(prefix + "dark", m_customSlots[i].darkSquareColor.name());
        config.setValue(prefix + "name", m_customSlots[i].customName);
    }
}
```

### 載入設定
```cpp
BoardColorSettings loadBoardColorSettings() {
    QSettings config("MyCompany", "Qt_Chess");
    BoardColorSettings defaults = BoardColorSettingsDialog::getDefaultSettings();
    
    BoardColorSettings settings;
    settings.lightSquareColor = QColor(
        config.value("board/lightColor", defaults.lightSquareColor.name()).toString()
    );
    settings.darkSquareColor = QColor(
        config.value("board/darkColor", defaults.darkSquareColor.name()).toString()
    );
    settings.scheme = static_cast<ColorScheme>(
        config.value("board/scheme", static_cast<int>(ColorScheme::Classic)).toInt()
    );
    settings.customName = config.value("board/customName", "").toString();
    
    // 載入自訂插槽
    for (int i = 0; i < 7; i++) {
        QString prefix = QString("board/custom%1/").arg(i);
        m_customSlots[i].lightSquareColor = QColor(
            config.value(prefix + "light", "#F0D9B5").toString()
        );
        m_customSlots[i].darkSquareColor = QColor(
            config.value(prefix + "dark", "#B58863").toString()
        );
        m_customSlots[i].customName = config.value(prefix + "name", "").toString();
    }
    
    return settings;
}
```

## UI 佈局

### 完整對話框佈局
```
┌──────────────────────────────────────┐
│ 棋盤顏色設定                         │
├──────────────────────────────────────┤
│ 預覽:                                │
│ ┌─────┬─────┐                        │
│ │淺色 │深色 │                        │
│ ├─────┼─────┤                        │
│ │深色 │淺色 │                        │
│ └─────┴─────┘                        │
├──────────────────────────────────────┤
│ 顏色選擇:                            │
│ [淺色方格] [深色方格]                │
├──────────────────────────────────────┤
│ 預設配色方案:                        │
│ [經典] [藍灰] [綠白] [紫粉]          │
│ [木紋] [海藍] [淺色]                 │
├──────────────────────────────────────┤
│ 自訂配色插槽:                        │
│ [插槽1] [插槽2] [插槽3] [插槽4]      │
│ [插槽5] [插槽6] [插槽7]              │
├──────────────────────────────────────┤
│      [重設為預設值]                  │
│      [確定] [取消]                   │
└──────────────────────────────────────┘
```

## 顏色設計建議

### 對比度
- 確保淺色和深色方格有足夠對比
- 但不要過於刺眼（避免純白配純黑）
- 建議對比度比例: 1:1.5 到 1:3

### 飽和度
- 適度的飽和度，不要太鮮豔
- 長時間觀看不會疲勞
- 經典棋盤通常使用低飽和度的暖色調

### 亮度
- 淺色方格亮度建議: 85-95%
- 深色方格亮度建議: 45-65%
- 避免過亮或過暗

### 色溫
- 暖色調（黃、橙、棕）: 傳統、溫暖
- 冷色調（藍、綠、灰）: 現代、專業
- 根據使用場景和個人喜好選擇

## 進階功能

### 匯出/匯入配色方案
```cpp
void exportColorScheme(const BoardColorSettings& settings, const QString& filename) {
    QJsonObject json;
    json["lightColor"] = settings.lightSquareColor.name();
    json["darkColor"] = settings.darkSquareColor.name();
    json["name"] = settings.customName;
    
    QJsonDocument doc(json);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}

BoardColorSettings importColorScheme(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return BoardColorSettingsDialog::getDefaultSettings();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject json = doc.object();
    
    BoardColorSettings settings;
    settings.lightSquareColor = QColor(json["lightColor"].toString());
    settings.darkSquareColor = QColor(json["darkColor"].toString());
    settings.customName = json["name"].toString();
    settings.scheme = ColorScheme::Custom1;
    
    return settings;
}
```

### 顏色和諧度檢查
```cpp
bool isColorHarmony(const QColor& light, const QColor& dark) {
    // 檢查對比度是否足夠
    double lightLuma = (0.299 * light.red() + 0.587 * light.green() + 0.114 * light.blue()) / 255.0;
    double darkLuma = (0.299 * dark.red() + 0.587 * dark.green() + 0.114 * dark.blue()) / 255.0;
    
    double contrast = (lightLuma + 0.05) / (darkLuma + 0.05);
    
    // 建議對比度至少為 1.5
    return contrast >= 1.5;
}
```

## 相關文檔
- 快速開始指南: `docs/BOARD_COLOR_QUICK_START.md`
- 配色方案設定: `docs/BOARD_COLOR_SETTINGS.md`
- 新配色方案: `docs/NEW_COLOR_SCHEMES.md`

## 相關類別
- `Qt_Chess` - 使用顏色設定繪製棋盤

## 依賴項目
- **Qt Widgets**: `QDialog`, `QColorDialog`, `QPushButton`, `QLabel`
- **Qt GUI**: `QColor`, `QPixmap`, `QPainter`
