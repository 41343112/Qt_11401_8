# Piece Size Adjustment Feature - Visual Guide

## Feature Overview
用戶可以在「棋子圖標設定」對話框中調整棋子大小，範圍從 60% 到 100%。
(Users can adjust piece size in the "Piece Icon Settings" dialog, ranging from 60% to 100%.)

## UI Layout

```
┌─────────────────────────────────────────────────────────┐
│  棋子圖標設定 (Piece Icon Settings)                     │
├─────────────────────────────────────────────────────────┤
│                                                           │
│  ┌─ 圖標集選擇 - 點擊預覽圖選擇 ────────────────────┐  │
│  │  ( ) Unicode 符號 (預設)  ( ) 預設圖標集 1       │  │
│  │  ( ) 預設圖標集 2         ( ) 預設圖標集 3       │  │
│  │  ( ) 自訂圖標                                     │  │
│  └───────────────────────────────────────────────────┘  │
│                                                           │
│  ┌─ 棋子大小調整 (NEW FEATURE) ─────────────────────┐  │
│  │  棋子縮放比例: [===●======] 80%                   │  │
│  │                60%  ←→  100%                      │  │
│  └───────────────────────────────────────────────────┘  │
│                                                           │
│  [重設為預設值]                    [確定]  [取消]        │
└─────────────────────────────────────────────────────────┘
```

## Slider Control Details

### Components
- **Label**: "棋子縮放比例:" (Piece Scale Ratio:)
- **Slider**: Horizontal slider with tick marks every 10%
- **Value Display**: Shows current percentage (e.g., "80%")

### Behavior
- **Minimum Value**: 60% (smaller pieces, more compact)
- **Maximum Value**: 100% (larger pieces, fill square more)
- **Default Value**: 80% (original appearance)
- **Step Size**: Tick marks every 10% for visual reference
- **Real-time Update**: Percentage label updates as slider moves

## Visual Examples

### 60% Scale (Minimum)
```
┌───┬───┬───┐
│ ♜ │   │ ♝ │  Pieces are smaller
├───┼───┼───┤  More space around pieces
│   │ ♟ │   │  Better for seeing board patterns
└───┴───┴───┘
```

### 80% Scale (Default)
```
┌───┬───┬───┐
│ ♜ │   │ ♝ │  Original appearance
├───┼───┼───┤  Balanced size
│   │ ♟ │   │  Standard spacing
└───┴───┴───┘
```

### 100% Scale (Maximum)
```
┌───┬───┬───┐
│ ♜ │   │ ♝ │  Pieces are larger
├───┼───┼───┤  Fill squares more completely
│   │ ♟ │   │  Easier to see for accessibility
└───┴───┴───┘
```

## User Flow

1. **Open Settings**
   ```
   Menu Bar → 設定 (Settings) → 棋子圖標設定 (Piece Icon Settings)
   ```

2. **Locate Slider**
   ```
   Look for "棋子大小調整" section (below icon set selection)
   ```

3. **Adjust Size**
   ```
   Drag slider left (smaller) or right (larger)
   Watch percentage value update in real-time
   ```

4. **Apply Changes**
   ```
   Click "確定" (OK) to apply
   Chess pieces on board immediately resize
   ```

5. **Result**
   ```
   All pieces on board display at new scale
   Drag-and-drop also uses new scale
   Setting persists across application restarts
   ```

## Use Cases

### Use Case 1: Accessibility
**Scenario**: User has difficulty seeing small pieces
**Solution**: Move slider to 90-100% for larger, more visible pieces
**Benefit**: Improved visibility and gameplay experience

### Use Case 2: Compact Board
**Scenario**: User wants more space around pieces for strategic planning
**Solution**: Move slider to 60-70% for smaller pieces
**Benefit**: Better overview of board position

### Use Case 3: Personal Preference
**Scenario**: User prefers different aesthetic
**Solution**: Adjust to any preferred size between 60-100%
**Benefit**: Customized appearance matching user taste

## Technical Notes

### Icon Types Supported
- ✅ Unicode chess symbols (♔, ♕, ♖, ♗, ♘, ♙)
- ✅ Preset icon set 1
- ✅ Preset icon set 2
- ✅ Preset icon set 3
- ✅ Custom user-uploaded icons

### Scale Application
- Applies to all 12 piece types uniformly
- Maintains aspect ratio (no distortion)
- Uses smooth scaling algorithm for quality
- Updates immediately when window resizes

### Performance
- Negligible performance impact
- Icons are cached after loading
- Scaling happens only when needed
- No impact on game logic or move validation

## Keyboard and Mouse Interaction

### Mouse
- Click and drag slider thumb
- Click on slider track to jump to position
- Scroll wheel over slider (if supported)

### Keyboard (when slider has focus)
- Left/Right arrow keys: Adjust by 1%
- Page Up/Down: Adjust by 10%
- Home: Jump to minimum (60%)
- End: Jump to maximum (100%)

## Integration with Other Features

### Icon Sets
Scale applies to whichever icon set is selected:
- Unicode symbols scale via font size
- Preset icons scale via pixmap scaling
- Custom icons scale via pixmap scaling

### Window Resize
When window is resized:
- Square sizes recalculate
- Piece icons scale proportionally
- User's scale factor is maintained

### Settings Persistence
Scale value is saved to QSettings:
- Automatically saved when OK is clicked
- Loaded when application starts
- Survives application restarts
- Stored with other piece icon settings

## Summary

The piece size adjustment feature provides users with flexible control over chess piece display size, enhancing both accessibility and aesthetic customization. The intuitive slider interface makes it easy to find the perfect size, and the persistent settings ensure a consistent experience across sessions.

**Key Benefits:**
- 🎯 Easy to use slider interface
- 👁️ Improves accessibility for users with visual needs
- 🎨 Customizable aesthetics
- 💾 Settings persist across sessions
- ⚡ Immediate visual feedback
- 🔧 Works with all icon types
