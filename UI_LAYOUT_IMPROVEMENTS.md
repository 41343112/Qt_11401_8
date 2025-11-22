# UI Layout Improvements Documentation

## Overview
This document describes the UI layout improvements implemented to enhance the chess game interface.

## Requirements (Original in Chinese)
1. **時間顯示不要碰到棋盤** - Time displays should not touch the chessboard
2. **時間控制裡的新遊戲移除** - Remove "New Game" from time control panel
3. **當遊戲開始隱藏時間控制，顯示新遊戲按鍵** - Hide time control when game starts, show New Game button
4. **棋盤置中** - Center the chessboard

## Implementation Details

### 1. Spacing Between Time Displays and Board
**Before:** Time labels were directly adjacent to the chessboard edges
**After:** Added 10px spacing above and below the board

```cpp
// Added spacing after black time label (above board)
boardContainerLayout->addWidget(m_blackTimeLabel, 0, Qt::AlignCenter);
boardContainerLayout->addSpacing(10);  // New spacing

// Board widget
boardContainerLayout->addWidget(m_boardWidget, 0, Qt::AlignCenter);

// Added spacing before white time label (below board)
boardContainerLayout->addSpacing(10);  // New spacing
boardContainerLayout->addWidget(m_whiteTimeLabel, 0, Qt::AlignCenter);
```

### 2. New Game Button Relocation
**Before:** "New Game" button was in the time control panel (left sidebar)
**After:** "New Game" button appears below the board, initially hidden

```cpp
// New game button (below white time label) - initially hidden
m_newGameButton = new QPushButton("新遊戲", m_boardContainer);
m_newGameButton->setMinimumHeight(40);
m_newGameButton->setMaximumWidth(200);
m_newGameButton->hide();  // Initially hidden
boardContainerLayout->addWidget(m_newGameButton, 0, Qt::AlignCenter);
```

### 3. Dynamic Panel Visibility
**Behavior:**
- **Initial State:** Time control panel visible, New Game button hidden
- **After clicking Start:** Time control panel hidden, time displays shown, New Game button visible
- **After game ends:** New Game button remains visible
- **After clicking New Game:** Time control panel visible, New Game button hidden

```cpp
// In onStartButtonClicked()
void Qt_Chess::onStartButtonClicked() {
    if (m_timeControlEnabled && !m_timerStarted) {
        m_timerStarted = true;
        startTimer();
        
        // Hide time control panel
        if (m_timeControlPanel) {
            m_timeControlPanel->hide();
        }
        
        // Show time displays
        if (m_whiteTimeLabel && m_blackTimeLabel) {
            m_whiteTimeLabel->show();
            m_blackTimeLabel->show();
        }
        
        // Show new game button
        if (m_newGameButton) {
            m_newGameButton->show();
        }
        // ...
    }
}

// In onNewGameClicked()
void Qt_Chess::onNewGameClicked() {
    // ...
    
    // Show time control panel
    if (m_timeControlPanel) {
        m_timeControlPanel->show();
    }
    
    // Hide new game button
    if (m_newGameButton) {
        m_newGameButton->hide();
    }
    
    // ...
}
```

### 4. Board Centering
**Implementation:**
- Board container aligned to center
- Maintained 2:1 space ratio (time control panel : board container)
- Center alignment ensures board is visually centered when time control is visible
- Board expands to fill space when time control is hidden

```cpp
contentLayout->addWidget(m_boardContainer, 2);  // Give board more space (2:1 ratio)
contentLayout->setAlignment(m_boardContainer, Qt::AlignCenter);  // Center the board container
```

## Code Quality Improvements

### Helper Method for Timeout Handling
Extracted duplicated code into a reusable helper method:

```cpp
void Qt_Chess::showTimeControlAfterTimeout() {
    // Show time control panel so user can adjust settings
    if (m_timeControlPanel) {
        m_timeControlPanel->show();
    }
    
    // Show new game button so user can quickly start a new game
    if (m_newGameButton) {
        m_newGameButton->show();
    }
    
    // Re-enable start button based on whether time control is active
    if (m_startButton) {
        m_startButton->setText("開始");
        m_startButton->setEnabled(m_timeControlEnabled);
    }
    
    // Hide time displays since game is over
    if (m_whiteTimeLabel) m_whiteTimeLabel->hide();
    if (m_blackTimeLabel) m_blackTimeLabel->hide();
}
```

This method is called when:
- White player runs out of time
- Black player runs out of time

## Visual Layout States

### State 1: Initial / Configuration
```
┌─────────────────────────────────────────┐
│  [Time Control Panel]  │  [Chessboard] │
│  - White Time Slider   │               │
│  - Black Time Slider   │  ┌─────────┐  │
│  - Increment Slider    │  │         │  │
│  - Start Button        │  │  Board  │  │
│                        │  │         │  │
│                        │  └─────────┘  │
└─────────────────────────────────────────┘
```

### State 2: During Gameplay
```
┌─────────────────────────────────────────┐
│                                         │
│              [Black Time]               │
│                 10px spacing            │
│             ┌─────────────┐             │
│             │             │             │
│             │  Chessboard │             │
│             │             │             │
│             └─────────────┘             │
│                 10px spacing            │
│              [White Time]               │
│              [New Game Button]          │
│                                         │
└─────────────────────────────────────────┘
```

### State 3: After Game Ends or Timeout
```
┌─────────────────────────────────────────┐
│  [Time Control Panel]  │  [Chessboard] │
│  - White Time Slider   │               │
│  - Black Time Slider   │  ┌─────────┐  │
│  - Increment Slider    │  │         │  │
│  - Start Button        │  │  Board  │  │
│                        │  │         │  │
│                        │  └─────────┘  │
│                        │ [New Game]    │
└─────────────────────────────────────────┘
```

## Benefits

1. **Cleaner Interface During Gameplay**: Only shows relevant controls (board, time displays, restart button)
2. **Better Visual Separation**: 10px spacing prevents visual clutter
3. **Intuitive Controls**: New Game button appears when needed, not during configuration
4. **Centered Board**: Board is properly centered for better visual balance
5. **Improved Code Quality**: Reduced duplication, consistent behavior, clear naming

## Testing Recommendations

To test these changes:
1. Start the application - verify time control panel is visible, New Game button is hidden
2. Set time controls and click Start - verify panel hides, time displays show, New Game button appears
3. Make some moves - verify New Game button remains visible
4. Click New Game - verify panel shows again, button hides
5. Let time run out - verify panel and button both appear
6. End game via checkmate - verify New Game button appears
7. Resize window - verify board remains centered

## Files Modified
- `qt_chess.h` - Added `showTimeControlAfterTimeout()` method declaration
- `qt_chess.cpp` - Implemented all UI changes and helper method

## Backward Compatibility
All changes are backward compatible. No breaking changes to existing functionality.
