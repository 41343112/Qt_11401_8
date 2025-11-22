# Visual Testing Guide for AspectRatioWidget Implementation

## Before and After Comparison

### Before (Original Layout)
```
┌─────────────────────────────────────────┐
│         Qt Chess Application            │
├─────────────────────────────────────────┤
│                                         │
│    ┌───────────────────────────┐       │
│    │                           │       │
│    │                           │       │
│    │                           │       │
│    │      Chessboard           │       │
│    │     (May be stretched)    │       │
│    │                           │       │
│    │                           │       │
│    └───────────────────────────┘       │
│                                         │
├─────────────────────────────────────────┤
│         [ 新遊戲 / New Game ]            │
└─────────────────────────────────────────┘

Issues:
❌ Board can be stretched horizontally or vertically
❌ Not always square
❌ No time control display
❌ Wasted space on wide screens
```

### After (New Layout with AspectRatioWidget)
```
┌───────────────────────────────────────────────────────┐
│            Qt Chess Application                       │
├────────┬──────────────────────────────────────────────┤
│        │                                              │
│ 白方   │                                              │
│ 時間   │          ┌─────────────┐                     │
│ --:--  │          │             │                     │
│        │          │             │                     │
│ 黑方   │          │ Chessboard  │                     │
│ 時間   │          │  (Square &  │                     │
│ --:--  │          │  Centered)  │                     │
│        │          │             │                     │
│        │          │             │                     │
│        │          └─────────────┘                     │
│        │                                              │
│[新遊戲]│                                              │
└────────┴──────────────────────────────────────────────┘

Improvements:
✅ Board always maintains 1:1 aspect ratio (perfect square)
✅ Board centered both horizontally and vertically
✅ Left panel sized by content (80-200px)
✅ Time controls visible
✅ Efficient use of space
```

## Testing Scenarios

### Scenario 1: Wide Window (1200 x 600)

**Window Configuration:**
- Width: 1200px
- Height: 600px
- Aspect ratio: 2:1 (very wide)

**Expected Layout:**
```
┌────────┬────────────────────────────────────────────────────────────┐ 600px
│ Left   │                                                            │
│ Panel  │              ┌───────────────┐                             │
│ ~120px │              │               │                             │
│        │              │               │                             │
│ Time   │              │   600x600     │                             │
│ Labels │              │  Chessboard   │                             │
│        │              │               │                             │
│ [新遊戲]│              │               │                             │
│        │              └───────────────┘                             │
│        │                                                            │
└────────┴────────────────────────────────────────────────────────────┘
         └──────────────── ~1080px ───────────────────┘

Key Points:
- Board size: 600x600 (limited by window height)
- Board centered in remaining ~1080px horizontal space
- Extra space appears on left and right of board
- Left panel: ~120px (based on content)
```

### Scenario 2: Tall Window (600 x 1000)

**Window Configuration:**
- Width: 600px
- Height: 1000px
- Aspect ratio: 3:5 (very tall)

**Expected Layout:**
```
┌────────┬──────────────────┐ 600px wide
│ Left   │                  │
│ Panel  │                  │
│ ~120px │  ┌────────────┐  │
│        │  │            │  │
│ Time   │  │            │  │
│ Labels │  │   480x480  │  │
│        │  │ Chessboard │  │
│ [新遊戲]│  │            │  │
│        │  │            │  │
│        │  └────────────┘  │
│        │                  │
│        │                  │
│        │                  │
│        │                  │
│        │                  │
└────────┴──────────────────┘
         └─── ~480px ──┘
         1000px tall

Key Points:
- Board size: 480x480 (limited by remaining width after left panel)
- Board centered vertically in available space
- Extra space appears above and below board
- Left panel: ~120px (based on content)
```

### Scenario 3: Square Window (800 x 800)

**Window Configuration:**
- Width: 800px
- Height: 800px
- Aspect ratio: 1:1 (perfect square)

**Expected Layout:**
```
┌────────┬────────────────────────────────┐ 800px
│ Left   │                                │
│ Panel  │     ┌──────────────────┐       │
│ ~120px │     │                  │       │
│        │     │                  │       │
│ Time   │     │                  │       │
│ Labels │     │    680x680       │       │
│        │     │   Chessboard     │       │
│ [新遊戲]│     │                  │       │
│        │     │                  │       │
│        │     └──────────────────┘       │
│        │                                │
└────────┴────────────────────────────────┘
         └────── ~680px ─────┘

Key Points:
- Board size: ~680x680 (uses most of remaining ~680px space)
- Board centered both horizontally and vertically
- Well-balanced appearance
- Left panel: ~120px (based on content)
```

### Scenario 4: Minimum Size (320 x 380)

**Window Configuration:**
- Width: 320px (minimum)
- Height: 380px (minimum)
- Aspect ratio: ~1:1.2

**Expected Layout:**
```
┌──┬────────────┐ 320px
│L │            │
│e │ ┌────────┐ │
│f │ │240x240 │ │
│t │ │ Board  │ │
│  │ └────────┘ │
│[…]│            │
└──┴────────────┘
80  └─ 240px ─┘
   380px tall

Key Points:
- Board size: 240x240 (minimum usable size)
- Left panel: 80px (minimum width)
- All UI elements still functional
- Pieces still visible and clickable
```

## Visual Verification Checklist

When testing, verify the following for each window size:

### Board Shape and Position
- [ ] Board is perfectly square (width == height)
- [ ] Board is horizontally centered in its available space
- [ ] Board is vertically centered in its available space
- [ ] Board scales smoothly during resize

### Left Panel
- [ ] Panel width is between 80-200px
- [ ] Panel doesn't stretch unnecessarily on wide windows
- [ ] Time labels are visible and properly formatted
- [ ] New Game button is visible and accessible

### Board Content
- [ ] All 64 squares are visible
- [ ] Chess pieces are properly sized within squares
- [ ] Piece icons/symbols are clearly readable
- [ ] Square colors (light/dark) are correct
- [ ] Board borders don't clip

### Interaction
- [ ] Clicking squares works correctly
- [ ] Drag and drop works smoothly
- [ ] Valid move highlights appear correctly
- [ ] Check/checkmate highlights work
- [ ] New Game button functions properly

## Common Visual Issues and Solutions

### Issue: Board is Rectangular Instead of Square
**Symptom:** Board appears stretched horizontally or vertically
**Check:**
- Verify AspectRatioWidget is properly wrapping the board
- Check that heightForWidth() returns the width value
- Ensure resizeEvent() uses qMin(width, height) for side calculation

### Issue: Board is Not Centered
**Symptom:** Board appears in corner or offset from center
**Check:**
- Verify centering calculation in resizeEvent():
  ```cpp
  int x = r.x() + (r.width() - side) / 2;
  int y = r.y() + (r.height() - side) / 2;
  ```
- Ensure contentsRect() is used, not geometry()

### Issue: Left Panel Too Wide/Narrow
**Symptom:** Panel takes too much or too little space
**Check:**
- Verify size policy is QSizePolicy::Preferred
- Check minimum width (should be 80px)
- Check maximum width (should be 200px)

### Issue: Pieces Overlap Square Borders
**Symptom:** Pieces appear larger than squares
**Check:**
- Verify updateSquareSizes() is being called on resize
- Check pieceScale setting (should be 60-100%)
- Ensure square size calculation is correct

## Performance Testing

### Resize Performance
**Test:** Rapidly resize window by dragging corner
**Expected:** Smooth resizing without lag or flickering
**Acceptance:** <16ms per frame (60 FPS)

### Initial Load
**Test:** Launch application and measure time to full display
**Expected:** Window appears with correct layout in <500ms
**Acceptance:** All elements visible and correctly positioned

### Large Window
**Test:** Maximize window on high-resolution display (e.g., 4K)
**Expected:** Board scales appropriately, stays centered
**Acceptance:** No visual artifacts, smooth appearance

## Regression Testing

Ensure existing functionality still works:

### Game Logic
- [ ] Piece movement rules (pawns, knights, bishops, rooks, queens, kings)
- [ ] Castling (kingside and queenside)
- [ ] En passant capture
- [ ] Pawn promotion
- [ ] Check detection
- [ ] Checkmate detection
- [ ] Stalemate detection

### UI Features
- [ ] Menu items (Settings, Sound, Piece Icons, Board Colors)
- [ ] Board flip feature
- [ ] Custom piece icons
- [ ] Board color themes
- [ ] Sound effects
- [ ] Drag and drop
- [ ] Click to move

### Settings Persistence
- [ ] Board colors saved across sessions
- [ ] Piece icon settings saved
- [ ] Board flip state saved
- [ ] Sound settings saved

## Screenshot Locations

When manually testing, capture screenshots at these key points:

1. **Initial launch** - Default window size
2. **Wide window** - 1200x600 or similar
3. **Tall window** - 600x1000 or similar
4. **Square window** - 800x800 or similar
5. **Minimum window** - 320x380
6. **During gameplay** - With pieces moved and highlights active
7. **Menu open** - Showing settings menu
8. **Settings dialog** - Board colors or piece icons

Save screenshots with descriptive names:
- `01_initial_launch.png`
- `02_wide_window_1200x600.png`
- `03_tall_window_600x1000.png`
- etc.

## Automated Testing Notes

While manual testing is required due to Qt's graphical nature, consider these aspects for future automation:

1. **Unit Tests:**
   - AspectRatioWidget::heightForWidth() returns input value
   - Centering calculations are mathematically correct
   - Size hints return expected values

2. **Integration Tests:**
   - Board widget is properly wrapped in AspectRatioWidget
   - Layout hierarchy is correct
   - Size policies are properly set

3. **Visual Regression Tests:**
   - Screenshot comparison at standard window sizes
   - Pixel-perfect layout verification
   - Color scheme consistency

## Conclusion

This visual testing guide ensures that the AspectRatioWidget implementation meets all requirements:
- ✅ Board stays square at all window sizes
- ✅ Board is centered both horizontally and vertically
- ✅ Left panel adjusts to content width
- ✅ No regressions in existing functionality
- ✅ Smooth and responsive user experience
