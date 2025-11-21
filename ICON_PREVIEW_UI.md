# Icon Preview UI - Visual Selection Interface

This document describes the new visual icon selection interface implemented in the Piece Icon Settings Dialog.

## Overview

The icon selection interface has been enhanced from a simple text dropdown to a visual grid layout that shows preview images of each icon set, making it much easier for players to choose their preferred style.

## New Interface Layout

```
┌────────────────────────────────────────────────────────────────────────┐
│  圖標集選擇 - 點擊預覽圖選擇 (Icon Set Selection - Click Preview to   │
│  Select)                                                                │
├────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌─────────────────────────────┐  ┌─────────────────────────────┐    │
│  │  ○ Unicode 符號 (預設)      │  │  ○ 預設圖標集 1            │    │
│  ├─────────────────────────────┤  ├─────────────────────────────┤    │
│  │     ♔ ♕ ♖ ♗ ♘ ♙            │  │  [♔] [♕] [♖] [♚] [♛] [♜]   │    │
│  │                              │  │  (actual PNG images)         │    │
│  └─────────────────────────────┘  └─────────────────────────────┘    │
│                                                                          │
│  ┌─────────────────────────────┐  ┌─────────────────────────────┐    │
│  │  ○ 預設圖標集 2             │  │  ○ 預設圖標集 3            │    │
│  ├─────────────────────────────┤  ├─────────────────────────────┤    │
│  │  [♔] [♕] [♖] [♚] [♛] [♜]   │  │  [♔] [♕] [♖] [♚] [♛] [♜]   │    │
│  │  (actual PNG images)         │  │  (actual PNG images)         │    │
│  └─────────────────────────────┘  └─────────────────────────────┘    │
│                                                                          │
│  ┌───────────────────────────────────────────────────────────────┐    │
│  │  ○ 自訂圖標 (Custom Icons)                                    │    │
│  ├───────────────────────────────────────────────────────────────┤    │
│  │  使用您自己的圖標檔案                                          │    │
│  │  點擊下方「瀏覽」按鈕選擇                                      │    │
│  └───────────────────────────────────────────────────────────────┘    │
│                                                                          │
└────────────────────────────────────────────────────────────────────────┘
```

## Features

### 1. Visual Grid Layout
- Icon sets are displayed in a 2x2 grid (plus custom option spanning full width)
- Each option is presented in a framed box with clear visual separation
- Large, easy-to-click selection areas

### 2. Radio Button Selection
- Each icon set has a radio button for selection
- Only one icon set can be selected at a time
- Current selection is visually highlighted

### 3. Preview Images

#### Unicode Option
- Displays large Unicode chess symbols: ♔ ♕ ♖ ♗ ♘ ♙
- Font size: 36pt for clear visibility
- Shows exactly what the default symbols look like

#### Preset Icon Sets (1, 2, 3)
- Shows 6 representative pieces:
  - White King, Queen, Rook
  - Black King, Queen, Rook
- Images are scaled to 48x48 pixels with smooth transformation
- Actual PNG icons are loaded from resources
- Maintains aspect ratio for proper display

#### Custom Icons Option
- Shows descriptive text in Chinese explaining the feature
- Text: "使用您自己的圖標檔案" (Use your own icon files)
- Text: "點擊下方「瀏覽」按鈕選擇" (Click 'Browse' button below to select)

### 4. Behavior

When a user clicks on any preview widget:
1. The radio button for that option is selected
2. The icon set type is updated in settings
3. If preset is selected, all piece icons are automatically loaded
4. If custom is selected, browse buttons are enabled for manual selection
5. The hidden combo box is updated for backward compatibility

## Implementation Details

### Key Methods

- **`createIconSetPreviewWidget(IconSetType, QString)`**: Creates a preview widget with:
  - Framed container with border
  - Radio button with bold label
  - Preview layout with icons or symbols
  - Returns complete widget ready to be added to grid

- **`onIconSetButtonClicked(int)`**: Handles selection:
  - Updates settings with selected icon set type
  - Syncs with hidden combo box
  - Applies preset icons or enables custom editing
  - Updates control states

### Visual Styling

- Frames use `QFrame::Box | QFrame::Raised` style with 2px line width
- Radio buttons use bold 11pt font for labels
- Preview images are 48x48 with smooth transformation
- Unicode symbols are 36pt for visibility
- Custom option text is 10pt in gray (#666)

## Benefits

1. **Immediate Visual Feedback**: Users see exactly what each icon set looks like
2. **Easier Selection**: No need to select and preview separately
3. **Better UX**: Click on the preview itself to select
4. **Clear Presentation**: Grid layout makes all options visible at once
5. **Professional Appearance**: Framed boxes with proper spacing

## Backward Compatibility

- Hidden combo box maintained for compatibility with existing code
- All existing icon set functionality preserved
- Settings structure unchanged
- Preset icon paths remain the same

## Resource Management

- Set1 icons are now included in resources.qrc
- Icons are embedded in application binary
- Resource paths use Qt resource system (:/resources/icons/...)
- Filenames match actual files on disk (including "biship" typo)

## Future Enhancements

Possible improvements for future versions:
- Add hover effects to preview widgets
- Show all 12 pieces in preview (if space permits)
- Add tooltips with icon set descriptions
- Animate selection transitions
- Support drag-and-drop for custom icons
