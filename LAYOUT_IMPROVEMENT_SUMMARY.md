# Layout Improvement - Centered Board with Time Control Always Visible

## Problem Statement (Chinese)
棋盤維持在視窗中間 棋盤的左邊不要空白 要被時間控制填滿 無論視窗放大縮小都要填滿

## Problem Statement (English)
Keep the chessboard in the center of the window. The left side of the chessboard should not be blank - it should be filled with time controls. The window should be filled regardless of whether the window is enlarged or reduced.

## Solution Overview

The layout has been restructured to use a three-panel design with fixed-width left and right panels, ensuring the board remains centered and the left panel is always filled with time controls.

### Layout Structure

```
┌─────────────────────────────────────────────────────────┐
│ Menu Bar                                                │
├──────────────┬────────────────────────┬─────────────────┤
│              │                        │                 │
│ Left Panel   │   Board Container      │  Right Panel    │
│ (300px)      │   (Flexible)           │  (200px)        │
│ - Always     │   - Board centered     │  - Always       │
│   visible    │   - Scales with space  │    visible      │
│              │                        │  - New Game btn │
│              │                        │    when needed  │
│              │                        │                 │
└──────────────┴────────────────────────┴─────────────────┘
```

## Implementation Details

### 1. Time Control Panel (Left - 300px)

The left panel contains two mutually exclusive group boxes that switch based on game state:

#### "時間設定" (Time Configuration) - Shown before/after game
- White player time limit slider and display
- Black player time limit slider and display
- Increment per move slider and display
- Start button

#### "對局時間" (Game Time) - Shown during gameplay
- Black player timer with title
- White player timer with title
- Live countdown displays
- Highlights active player's timer

### 2. Board Container (Center - Flexible)

- Contains only the chessboard widget
- No time labels beside the board
- Board widget is centered within the container
- Container uses stretch factor 1 to fill remaining horizontal space
- Board automatically scales based on available space while maintaining square proportions

### 3. Right Panel (200px)

- Fixed width for visual balance
- Contains "New Game" button (vertically centered)
- Button shown only when game is in progress or ended
- Hidden during configuration phase

## Key Code Changes

### setupUI() Function
```cpp
// Left panel - fixed width, always visible
m_timeControlPanel->setMaximumWidth(LEFT_PANEL_MAX_WIDTH);
m_timeControlPanel->setMinimumWidth(LEFT_PANEL_MAX_WIDTH);
contentLayout->addWidget(m_timeControlPanel, 0);  // Stretch factor 0

// Board container - flexible, centered
contentLayout->addWidget(m_boardContainer, 1);  // Stretch factor 1

// Right panel - fixed width, always visible
rightPanel->setMaximumWidth(RIGHT_PANEL_MAX_WIDTH);
rightPanel->setMinimumWidth(RIGHT_PANEL_MAX_WIDTH);
contentLayout->addWidget(rightPanel, 0);  // Stretch factor 0
```

### setupTimeControlUI() Function
- Added "對局時間" group box for game time display
- Moved time display labels into left panel (previously beside board)
- Both group boxes contained within time control panel
- Object names set for easy retrieval: "gameTimeGroup", "timeControlGroup"

### State Transitions

**Initial State → Game Started:**
```cpp
void Qt_Chess::onStartButtonClicked() {
    // Hide configuration, show game time
    timeControlGroup->hide();
    gameTimeGroup->show();
    m_newGameButton->show();
}
```

**Game Ended → New Game:**
```cpp
void Qt_Chess::onNewGameClicked() {
    // Show configuration, hide game time
    timeControlGroup->show();
    gameTimeGroup->hide();
    m_newGameButton->hide();
}
```

### updateSquareSizes() Function
- Always accounts for both fixed-width panels (previously checked visibility)
- Removed time label width calculations (now in fixed-width panel)
- Simplified reserved width calculation:
  ```cpp
  reservedWidth += LEFT_PANEL_MAX_WIDTH + PANEL_SPACING;
  reservedWidth += RIGHT_PANEL_MAX_WIDTH + PANEL_SPACING;
  reservedWidth += BASE_MARGINS;
  ```

## Testing Results

Tested at multiple resolutions to verify proper scaling:

### 1024x768 Resolution
- ✅ Board centered
- ✅ Left panel fills 300px with time controls
- ✅ Right panel provides 200px balance
- ✅ Board scales appropriately

### 1280x720 Resolution
- ✅ Board centered with more space
- ✅ Panels maintain fixed widths
- ✅ Board scales up proportionally
- ✅ Layout remains balanced

### 900x600 Resolution (Minimum)
- ✅ Board centered at minimum size
- ✅ Panels maintain fixed widths
- ✅ All controls accessible
- ✅ No clipping or overflow

## Benefits

1. **Consistent Layout**: Board always centered, panels always visible
2. **No Blank Space**: Left side always filled with relevant controls
3. **Intuitive Design**: Time controls where players expect them
4. **Scales Properly**: Works at any window size
5. **Better UX**: Clear separation between configuration and gameplay states
6. **Minimal Changes**: Surgical modifications to existing code

## Visual States

### State 1: Configuration (Before Game)
```
┌────────────────────────────────────────────┐
│ ┌──────────────┐ ┌────────────┐           │
│ │ 時間設定     │ │            │           │
│ │              │ │            │           │
│ │ 白方時間:    │ │            │           │
│ │ [不限時]     │ │            │           │
│ │ ────────────  │ │   Chess    │           │
│ │              │ │   Board    │           │
│ │ 黑方時間:    │ │            │           │
│ │ [不限時]     │ │            │           │
│ │ ────────────  │ │            │           │
│ │              │ └────────────┘           │
│ │ 每著加秒:    │                          │
│ │ [0秒]        │                          │
│ │ ────────────  │                          │
│ │              │                          │
│ │  [  開始  ]  │                          │
│ └──────────────┘                          │
└────────────────────────────────────────────┘
```

### State 2: During Game
```
┌────────────────────────────────────────────┐
│ ┌──────────────┐ ┌────────────┐ ┌────────┐│
│ │ 對局時間     │ │            │ │        ││
│ │              │ │            │ │        ││
│ │ 黑方:        │ │            │ │        ││
│ │ [  05:00  ]  │ │            │ │[新遊戲]││
│ │              │ │   Chess    │ │        ││
│ │              │ │   Board    │ │        ││
│ │ 白方:        │ │            │ │        ││
│ │ [  05:00  ]  │ │            │ │        ││
│ │              │ │            │ │        ││
│ │              │ └────────────┘ │        ││
│ │              │                │        ││
│ │  [ 進行中 ]  │                └────────┘│
│ └──────────────┘                          │
└────────────────────────────────────────────┘
```

## Files Modified

- `qt_chess.cpp`: Main implementation changes
  - `setupUI()`: Restructured layout with fixed-width panels
  - `setupTimeControlUI()`: Added game time display group
  - `onStartButtonClicked()`: Toggle between configuration and game display
  - `onNewGameClicked()`: Reset to configuration display
  - `showTimeControlAfterTimeout()`: Restore configuration after timeout
  - `updateSquareSizes()`: Simplified panel width calculations

- `qt_chess.h`: No changes required (existing member variables used)

## Backward Compatibility

- ✅ All existing features work unchanged
- ✅ Settings persistence maintained
- ✅ Time control functionality unchanged
- ✅ Drag-and-drop still works
- ✅ All menu functions work
- ✅ Board flip feature unaffected

## Future Considerations

- Panel widths could be made configurable via settings if desired
- Animation could be added for smooth transitions between states
- Additional information could be displayed in the right panel during configuration

## Conclusion

The implementation successfully addresses all requirements:
1. ✅ Chessboard remains centered in the window
2. ✅ Left side always filled with time control panel
3. ✅ Layout adapts to window resizing while maintaining structure
4. ✅ Clean, intuitive user interface
5. ✅ Minimal code changes with surgical precision
