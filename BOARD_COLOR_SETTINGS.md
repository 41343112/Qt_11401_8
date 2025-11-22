# Board Color Settings Feature

## Overview
Added a custom chessboard color settings interface that allows users to customize the appearance of the chessboard.

## Features

### 1. Color Scheme Selection
Users can choose from 7 preset options:
- **經典棕褐色 (Classic)**: Traditional brown/beige chess board colors (#F0D9B5 and #B58863)
- **藍灰配色 (Blue/Gray)**: Modern blue and gray color scheme (#DEE3E6 and #8CA2AD)
- **綠白配色 (Green/White)**: Fresh green and white color scheme (#FFFFDD and #86A666)
- **紫粉配色 (Purple/Pink)**: Elegant purple and pink color scheme (#E8C4E8 and #9B6B9B)
- **木紋深色 (Wood/Dark)**: Rich wooden brown with dark contrast (#D4A574 and #6B4423)
- **海洋藍配色 (Ocean/Blue)**: Deep ocean blue with light blue contrast (#A8D8EA and #2E5B6D)
- **淺色系 (Light Theme)**: Very light pastel colors for a soft, minimal look (#FEFEFE and #E0E0E0)
- **自訂顏色 (Custom)**: User-defined colors using color picker

### 2. 2×2 Preview Grid
- A real-time 2×2 preview grid shows how the board will look
- Updates instantly when colors are changed
- Each preset also has its own mini 2×2 preview thumbnail
- **NEW: Preview thumbnails are clickable** - click to instantly apply that color scheme

### 3. Custom Color Picker
When "Custom" mode is selected or custom preview is clicked:
- **NEW: Click the "自訂" (Custom) preview thumbnail to open color pickers directly**
- Two sequential color picker dialogs allow users to select light and dark square colors
- Original method: Two color buttons allow users to select colors individually
- Color picker dialog opens when clicking the buttons
- Preview updates in real-time

### 4. Settings Persistence
- All color settings are saved automatically using QSettings
- Settings persist between application sessions
- Stored under "Qt_Chess/BoardColorSettings"

## User Interface

### Accessing the Settings
1. Open the application
2. Click "設定" (Settings) menu in the menu bar
3. Select "棋盤顏色設定" (Board Color Settings)

### Dialog Layout
1. **Preset Schemes Section**: Visual previews with 7 preset color schemes
2. **Preset Previews**: Seven clickable 2×2 mini previews displayed horizontally
   - Classic, Blue-Gray, Green-White, Purple-Pink, Wood-Dark, Ocean-Blue, Light Theme
   - Hover effect shows blue border indicating they are interactive
   - Click to instantly apply that color scheme
   - Note: On smaller screens, the dialog may need to be resized to view all presets
3. **Custom Colors Section**: Two color buttons for light and dark squares (still available)
4. **Main Preview**: Large 2×2 preview grid (80×80 pixels per square)
5. **Buttons**: Reset to Defaults, OK, Cancel

## Implementation Details

### Files Added
- `boardcolorsettingsdialog.h`: Header file defining the dialog class
- `boardcolorsettingsdialog.cpp`: Implementation of the color settings dialog

### Files Modified
- `Qt_Chess.pro`: Added new source and header files
- `qt_chess.h`: Added board color settings member variable and methods
- `qt_chess.cpp`: 
  - Added include for boardcolorsettingsdialog.h
  - Added menu action for board color settings
  - Modified `updateSquareColor()` to use configurable colors
  - Added `loadBoardColorSettings()` and `applyBoardColorSettings()` methods
  - Integrated color settings into main window initialization

### Color Scheme Defaults
```cpp
Classic:    Light=#F0D9B5, Dark=#B58863
BlueGray:   Light=#DEE3E6, Dark=#8CA2AD
GreenWhite: Light=#FFFFDD, Dark=#86A666
PurplePink: Light=#E8C4E8, Dark=#9B6B9B
WoodDark:   Light=#D4A574, Dark=#6B4423
OceanBlue:  Light=#A8D8EA, Dark=#2E5B6D
LightTheme: Light=#FEFEFE, Dark=#E0E0E0
```

## Technical Architecture

### BoardColorSettingsDialog Class
```cpp
enum class ColorScheme {
    Classic,      // Classic brown/beige (default)
    BlueGray,     // Blue and gray
    GreenWhite,   // Green and white
    PurplePink,   // Purple and pink
    WoodDark,     // Wood and dark
    OceanBlue,    // Ocean blue
    LightTheme,   // Light pastel colors
    Custom1,      // Custom user-selected colors slot 1
    Custom2,      // Custom user-selected colors slot 2
    Custom3,      // Custom user-selected colors slot 3
    Custom4,      // Custom user-selected colors slot 4
    Custom5,      // Custom user-selected colors slot 5
    Custom6,      // Custom user-selected colors slot 6
    Custom7       // Custom user-selected colors slot 7
};

struct BoardColorSettings {
    QColor lightSquareColor;
    QColor darkSquareColor;
    ColorScheme scheme;
    QString customName;  // Name for custom color schemes
};
```

### Key Methods
- `getDefaultSettings()`: Returns default (Classic) color scheme
- `getPresetSettings(ColorScheme)`: Returns preset colors for a given scheme
- `updatePreview()`: Updates the 2×2 preview grid
- `createPresetPreview()`: **NEW: Creates clickable QPushButton previews** with hover effects
- `applyPresetColorScheme()`: Applies a selected preset
- `onPresetPreviewClicked()`: **NEW: Handles clicks on preset preview thumbnails**

## Integration Points

### Main Window Integration
1. Settings loaded on application startup via `loadBoardColorSettings()`
2. Menu action added to Settings menu
3. Dialog opened via `onBoardColorSettingsClicked()`
4. Changes applied immediately via `applyBoardColorSettings()`
5. All board squares updated when settings change

### Settings Storage
```cpp
QSettings settings("Qt_Chess", "BoardColorSettings");
settings.setValue("colorScheme", schemeInt);
settings.setValue("lightSquareColor", colorName);
settings.setValue("darkSquareColor", colorName);
```

## User Experience

### Default Behavior
- Application starts with Classic color scheme
- All squares use configured colors
- Highlights and special states (check, valid moves) still work correctly

### Color Changes
- Changes are applied immediately when OK is clicked
- Preview shows exact appearance before applying
- All 64 board squares update simultaneously
- Existing game state is preserved

### Reset Functionality
- "重設為預設值" (Reset to Defaults) button restores Classic scheme
- Individual colors can be changed without affecting the overall scheme
- Switching to a preset automatically updates both colors

## Compatibility
- Works with existing sound settings
- Works with existing piece icon settings
- Maintains check, checkmate, and valid move highlights
- Compatible with all chess game features (castling, en passant, promotion)

## Future Enhancements (Optional)
- Add more preset color schemes
- Allow saving custom schemes with names
- Import/export color schemes
- Add opacity/transparency controls
- Add border color customization
