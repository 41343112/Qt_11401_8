# Left Panel Scaling Implementation

## Problem Statement (問題描述)
左側面板隨視窗大小縮放
(Left side panel scales with window size)

## Issue Analysis

### Before Changes
The left side panel (`m_moveListPanel`) had a fixed maximum width constraint:
```cpp
m_moveListPanel->setMaximumWidth(LEFT_PANEL_MAX_WIDTH); // 600px max
contentLayout->addWidget(m_moveListPanel); // No stretch factor
```

This caused the left panel to:
- Never expand beyond 600 pixels
- Not respond to window resizing
- Create an unbalanced layout when the window was enlarged

### After Changes
The left panel now scales dynamically:
```cpp
// No maximum width constraint
contentLayout->addWidget(m_moveListPanel, 1); // Stretch factor of 1
```

This allows the left panel to:
- Expand and contract with window resizing
- Maintain proper proportions with other UI elements
- Provide a more responsive user interface

## Implementation Details

### File: qt_chess.cpp

#### Change 1: Remove Maximum Width Constraint
**Location**: setupUI method (around line 172)

**Before**:
```cpp
m_moveListPanel = new QWidget(this);
m_moveListPanel->setMaximumWidth(LEFT_PANEL_MAX_WIDTH);
QVBoxLayout* moveListLayout = new QVBoxLayout(m_moveListPanel);
```

**After**:
```cpp
m_moveListPanel = new QWidget(this);
QVBoxLayout* moveListLayout = new QVBoxLayout(m_moveListPanel);
```

**Impact**: Removes the 600px width limitation, allowing the panel to grow.

#### Change 2: Add Stretch Factor to Layout
**Location**: setupUI method (around line 252)

**Before**:
```cpp
contentLayout->addWidget(m_moveListPanel);
```

**After**:
```cpp
contentLayout->addWidget(m_moveListPanel, 1);  // 添加伸展因子以允許縮放
```

**Impact**: The stretch factor of 1 tells the Qt layout manager to allocate proportional space to the left panel when the window is resized.

#### Change 3: Add Helper Function for Panel Width Calculation
**Location**: Anonymous namespace (lines 73-84)

```cpp
// 獲取面板的實際寬度，如果尚未渲染則使用後備值的輔助函數
static int getPanelWidth(QWidget* panel) {
    if (!panel) return 0;
    
    int width = panel->width();
    if (width <= 0) {
        width = panel->sizeHint().width();
        if (width <= 0) {
            width = MIN_PANEL_FALLBACK_WIDTH;
        }
    }
    return width;
}
```

**Impact**: Provides a robust three-tier fallback strategy for getting panel widths.

#### Change 4: Update Square Size Calculation
**Location**: updateSquareSizes method (around lines 1208-1214)

**Before**:
```cpp
if (m_moveListPanel) {
    int leftPanelWidth = m_moveListPanel->sizeHint().width();
    if (leftPanelWidth <= 0) leftPanelWidth = LEFT_PANEL_MAX_WIDTH;
    reservedWidth += qMin(leftPanelWidth, LEFT_PANEL_MAX_WIDTH);
}

if (m_timeControlPanel && m_timeControlPanel->isVisible()) {
    int rightPanelWidth = m_timeControlPanel->sizeHint().width();
    if (rightPanelWidth <= 0) rightPanelWidth = RIGHT_PANEL_MAX_WIDTH;
    reservedWidth += qMin(rightPanelWidth, RIGHT_PANEL_MAX_WIDTH);
}
```

**After**:
```cpp
// 考慮左側面板的實際寬度（棋譜面板）- 總是可見
reservedWidth += getPanelWidth(m_moveListPanel);

// 如果可見則考慮右側面板的實際寬度（時間控制面板）
if (m_timeControlPanel && m_timeControlPanel->isVisible()) {
    reservedWidth += getPanelWidth(m_timeControlPanel);
}
```

**Impact**: The chess board size calculation now correctly accounts for the actual panel widths using the `getPanelWidth()` helper function, which provides robust fallback logic.

## Helper Function Design

The `getPanelWidth()` helper function implements a three-tier fallback strategy:

1. **Primary**: `panel->width()` - Returns the actual rendered width
2. **Secondary**: `panel->sizeHint().width()` - Returns the widget's preferred size if not yet rendered
3. **Tertiary**: `MIN_PANEL_FALLBACK_WIDTH` (200px) - A safe minimum value as last resort

This design ensures:
- Accurate calculations during normal operation
- Graceful handling of initialization edge cases
- Prevention of division-by-zero or invalid size calculations
- No code duplication between left and right panel width calculations

## Layout Structure

The application uses a horizontal layout with three main sections:

```
┌─────────────────────────────────────────────────────────────┐
│  Horizontal Layout (contentLayout)                          │
│  ┌─────────────┬──────────┬─────────────┬──────────┬──────┐│
│  │ Left Panel  │ Stretch  │   Board     │ Stretch  │Right ││
│  │ (Move List) │    (0)   │ Container   │   (0)    │Panel ││
│  │             │          │             │          │      ││
│  │ Stretch: 1  │          │ Stretch: 0  │          │Str: 1││
│  └─────────────┴──────────┴─────────────┴──────────┴──────┘│
└─────────────────────────────────────────────────────────────┘
```

### Stretch Factor Explanation
- **Left Panel (1)**: Can expand/contract proportionally
- **Board Container (0)**: Fixed size, centered by surrounding stretches
- **Right Panel (1)**: Can expand/contract proportionally
- **Stretch spaces (0)**: Flexible spacers that center the board

## Expected Behavior

### Window Resizing
1. **Expanding Window Width**:
   - Left panel expands proportionally
   - Chess board may grow up to its maximum size (MAX_SQUARE_SIZE)
   - Right panel expands proportionally
   - Overall balance is maintained

2. **Shrinking Window Width**:
   - Left panel shrinks proportionally
   - Chess board shrinks down to its minimum size (MIN_SQUARE_SIZE)
   - Right panel shrinks proportionally
   - Minimum window size (814x420) prevents excessive shrinking

3. **Height Changes**:
   - Chess board adjusts to maintain square pieces
   - Panels maintain their vertical layout

## Testing Recommendations

To verify the implementation:

1. **Launch the application**:
   ```bash
   ./Qt_Chess
   ```

2. **Test expanding the window**:
   - Drag the window border to increase width
   - Verify the left panel expands
   - Verify the chess board remains centered
   - Verify proportions are maintained

3. **Test shrinking the window**:
   - Drag the window border to decrease width
   - Verify the left panel shrinks
   - Verify minimum size constraints are respected

4. **Test with game in progress**:
   - Start a game and make several moves
   - Resize the window at various stages
   - Verify the move list panel scales appropriately
   - Verify no UI elements are cut off or overlapping

## Technical Considerations

### Qt Layout System
- Qt's QHBoxLayout manages widget spacing based on stretch factors
- Stretch factor 0: Widget uses its size hint and doesn't grow
- Stretch factor > 0: Widget gets proportional space from available extra room
- The ratio of stretch factors determines space distribution

### Size Constraints
- Minimum window size: 814x420 pixels (set in constructor)
- Minimum square size: 40 pixels (MIN_SQUARE_SIZE)
- Maximum square size: 170 pixels (MAX_SQUARE_SIZE)
- No maximum window size (system-dependent)

### Performance
- The `resizeEvent` handler is called during window resizing
- `updateSquareSizes()` recalculates board and piece sizes
- All calculations are O(1) and very fast
- No performance impact expected

## Compatibility

- **Qt Version**: Qt 5.15+ (compatible with project)
- **Platform**: Cross-platform (Linux, Windows, macOS)
- **Dependencies**: No new dependencies introduced

## Related Constants

From qt_chess.cpp:
```cpp
const int LEFT_PANEL_MAX_WIDTH = 600;  // No longer used for m_moveListPanel
const int RIGHT_PANEL_MAX_WIDTH = 600; // Still used for m_timeControlPanel
const int MIN_SQUARE_SIZE = 40;
const int MAX_SQUARE_SIZE = 170;
```

Note: `LEFT_PANEL_MAX_WIDTH` constant is still defined but no longer applied to the left panel. It may be removed in future cleanup.

## Summary

This implementation successfully enables the left side panel to scale with window size by:
1. Removing restrictive width constraints
2. Adding appropriate stretch factors in the layout
3. Updating size calculations to use actual panel widths

The changes are minimal, focused, and maintain compatibility with the existing codebase.
