# Testing Custom Chess Piece Icons

This document provides guidance on how to test the new custom chess piece icon feature.

## Prerequisites

- Qt5 development environment installed
- Image files for chess pieces (PNG, JPG, SVG, or BMP format)

## Building the Application

```bash
qmake Qt_Chess.pro
make
./Qt_Chess
```

## Testing Steps

### 1. Basic Icon Setting Test

1. Launch the application
2. Go to **設定** (Settings) menu → **棋子圖標設定** (Piece Icon Settings)
3. The dialog should display:
   - A checkbox: "使用自訂圖標" (Use Custom Icons)
   - Two groups: "白方棋子" (White Pieces) and "黑方棋子" (Black Pieces)
   - Each piece should have: Browse, Preview, and Reset buttons

### 2. Icon Selection Test

1. Click the **瀏覽** (Browse) button next to any piece (e.g., White King)
2. Select an image file (PNG recommended)
3. The file path should appear in the text field
4. Repeat for other pieces as desired

### 3. Icon Preview Test

1. After selecting an icon file, click the **預覽** (Preview) button
2. A preview dialog should display the icon
3. The preview should show a scaled version of the image
4. Click **關閉** (Close) to dismiss the preview

### 4. Enable Custom Icons Test

1. Check the "使用自訂圖標" (Use Custom Icons) checkbox
2. Click **確定** (OK) to apply settings
3. The game board should now display custom icons for pieces that have been configured
4. Pieces without custom icons should still display Unicode symbols

### 5. Game Play Test with Custom Icons

1. Start a new game with custom icons enabled
2. Select a piece - valid moves should be highlighted correctly
3. Drag and drop a piece:
   - The dragged piece should display the custom icon
   - The piece should appear correctly at the destination
4. Make several moves to ensure icons are displayed correctly throughout the game

### 6. Window Resize Test

1. With custom icons enabled, resize the game window
2. Custom icons should scale proportionally with the board squares
3. Icons should maintain their aspect ratio and remain centered

### 7. Reset Individual Icon Test

1. Open the Piece Icon Settings dialog
2. Click the **重設** (Reset) button next to a configured piece
3. The icon path should clear
4. Apply settings and verify the piece reverts to Unicode symbol

### 8. Reset All Icons Test

1. Open the Piece Icon Settings dialog
2. Click the **重設為預設值** (Reset to Defaults) button at the bottom
3. All icon paths should clear
4. The "Use Custom Icons" checkbox should be unchecked
5. Apply settings and verify all pieces use Unicode symbols

### 9. Persistence Test

1. Configure custom icons and enable them
2. Close the application
3. Relaunch the application
4. Custom icons should be automatically loaded and displayed
5. Open settings to verify all paths are still configured

### 10. Invalid File Test

1. Manually edit the settings file or select a non-existent file
2. The application should fall back to Unicode symbols for that piece
3. No crashes should occur

### 11. Toggle Custom Icons Test

1. Configure some custom icons
2. Apply and verify they are displayed
3. Reopen settings and uncheck "使用自訂圖標" (Use Custom Icons)
4. Apply and verify all pieces use Unicode symbols
5. Reopen settings and check the checkbox again
6. Apply and verify custom icons are displayed again

## Expected Behavior

- **Successful icon loading**: Custom icon appears on the board square
- **Failed icon loading**: Falls back to Unicode symbol
- **No custom icon set**: Uses Unicode symbol
- **Drag and drop**: Displays custom icon during drag operation
- **Window resize**: Icons scale with board size
- **Settings persistence**: Icons persist across application restarts

## Recommended Test Images

For best results, use:
- **Format**: PNG with transparent background
- **Size**: 100x100 to 500x500 pixels
- **Aspect Ratio**: Square (1:1)
- **Background**: Transparent or white
- **Style**: Clear, high-contrast designs

You can find free chess piece icons from:
- Wikimedia Commons
- Open Game Art
- Various free icon libraries

## Troubleshooting

### Icons not displaying
- Check if "Use Custom Icons" checkbox is enabled
- Verify the image file exists at the specified path
- Try using PNG format instead of other formats
- Check if the image file is readable

### Icons appear distorted
- Use square aspect ratio images
- Ensure images are high enough resolution (at least 100x100)

### Application crashes
- Verify image file is not corrupted
- Check file permissions
- Ensure Qt can read the image format

## Settings File Location

Custom icon settings are stored in:
- **Linux**: `~/.config/Qt_Chess/ChessGame.conf`
- **Windows**: Registry or INI file in application data folder
- **macOS**: `~/Library/Preferences/com.Qt_Chess.ChessGame.plist`

You can manually inspect or edit these files if needed.
