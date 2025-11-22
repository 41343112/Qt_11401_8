# Time Control UI Improvements Documentation

## Overview
This document describes the UI improvements made to the time control feature based on the requirements:
- 控制時間使用滑桿介面與棋盤同高 (Control time slider interface same height as chessboard)
- 新增開始按鍵 (Add start button)
- 顯示時間顯示在棋盤的右下與左上 (Display time at bottom-right and top-left of chessboard)

## New UI Layout

```
+------------------------------------------------------------------------------+
|  Menu: 設定 (Settings)                                                        |
+------------------------------------------------------------------------------+
|                                                                              |
|  +-----------------------------------------+  +-------------------------+   |
|  | +-----+                                 |  | 時間控制 (Time Control) |   |
|  | |10:00|  ← Black Time (Top-Left)       |  |                         |   |
|  | +-----+                                 |  | 總時間: (Total Time)    |   |
|  |                                         |  | [無限制/30秒/1分鐘... ▼]|   |
|  |                                         |  |                         |   |
|  |           Chess Board                   |  | 每著加秒: (Increment)   |   |
|  |           (8x8 squares)                 |  |      [5秒]              |   |
|  |                                         |  |       ▲                 |   |
|  |                                         |  |       |                 |   |
|  |                                         |  |       █  ← Vertical     |   |
|  |                                         |  |       |     Slider      |   |
|  |                                         |  |       |     (matches    |   |
|  |                       White Time ↓      |  |       |      board      |   |
|  |                               +-----+   |  |       |      height)    |   |
|  |                               |10:00|   |  |       ▼                 |   |
|  |                               +-----+   |  |                         |   |
|  +-----------------------------------------+  | +---------------------+ |   |
|                                              | |   開始 (Start)       | |   |
|  +-----------------------------------------+  | +---------------------+ |   |
|  |         新遊戲 (New Game)                |  +-------------------------+   |
|  +-----------------------------------------+                                |
|                                                                              |
+------------------------------------------------------------------------------+
```

## Key Changes

### 1. Time Display Position
- **Black Time**: Top-left corner of chessboard (10px, 10px)
- **White Time**: Bottom-right corner of chessboard (calculated dynamically)
- **Style**: Semi-transparent background (rgba(51, 51, 51, 200))
- **Size**: 100px × 40px (compact and unobtrusive)
- **Active Player**: Green background (rgba(76, 175, 80, 200))
- **Inactive Player**: Gray background (rgba(51, 51, 51, 200))

### 2. Increment Slider
- **Orientation**: Changed from horizontal to vertical
- **Height**: Matches chessboard height (stretch factor 1 in layout)
- **Position**: In the time control panel on the right
- **Range**: 0-60 seconds
- **Tick marks**: Every 5 seconds on the left side

### 3. Start Button (開始)
- **Position**: Bottom of time control panel
- **Size**: Minimum height 40px
- **Font**: 12pt bold
- **States**:
  - **Disabled** (gray): When time control is set to "無限制" (unlimited)
  - **Enabled** (clickable): When a time limit is selected
  - **"開始"**: Ready to start
  - **"進行中"** (disabled): Timer running

### 4. Timer Behavior
- Timer does **NOT** start automatically when first move is made
- User must click the "開始" (Start) button to begin countdown
- Timer continues running after moves (no automatic restart needed)
- Timer stops when:
  - New game is started
  - Time control is changed to unlimited
  - Player runs out of time

## Component Layout in Right Panel

```
時間控制 (Time Control Group Box)
├── 總時間: (Label)
├── [Combo Box] ← Time limit selection
├── 每著加秒: (Label)
├── [0秒] ← Current increment value
├── [Vertical Slider] ← Stretches to fill height
└── [開始 Button] ← Start timer manually
```

## Visual Characteristics

### Time Display Overlays
- **Transparency**: Semi-transparent to see board underneath
- **Positioning**: Absolute positioning, overlaid on board container
- **Auto-repositioning**: Recalculated on window resize
- **Z-index**: Raised to top layer
- **Font**: 14pt bold, white color

### Vertical Slider
- **Visual feedback**: Current value displayed above slider
- **Tick marks**: On the left side
- **Height**: Dynamically matches board height
- **Responsive**: Adjusts with window resize

## User Workflow

1. **Setup Game**:
   - Select time limit from dropdown (e.g., "5分鐘")
   - Adjust increment slider if desired (e.g., 5 seconds)
   - Click "新遊戲" (New Game) to reset board

2. **Start Timer**:
   - "開始" button becomes enabled after selecting a time limit
   - Click "開始" to start the countdown
   - Button text changes to "進行中" and becomes disabled

3. **During Game**:
   - Time displays update every 100ms
   - Active player's timer shows green background
   - Increment automatically added after each move
   - Time displays remain visible on board

4. **Game End**:
   - Timer stops when a player runs out of time
   - Dialog shows which player won
   - Click "新遊戲" to start a new game

## Technical Implementation Details

### New Class Members
```cpp
QPushButton* m_startButton;          // Start button widget
bool m_timerStarted;                 // Track if timer manually started
QWidget* m_boardContainer;           // Container for board + overlays
```

### New Methods
```cpp
void onStartButtonClicked();         // Handle start button click
void positionTimeDisplaysOnBoard();  // Position time labels on board
```

### Modified Methods
- `setupUI()`: Creates board container for overlays
- `setupTimeControlUI()`: Vertical slider, start button, overlay labels
- `onNewGameClicked()`: Resets timer state and button
- `onTimeLimitChanged()`: Manages start button state
- `startTimer()`: Only starts if `m_timerStarted` is true
- `resizeEvent()`: Repositions time displays on resize
- Move methods: Removed automatic timer start

## Benefits

1. **Less Intrusive**: Time displays are overlaid on board, don't take extra space
2. **Better Visual Flow**: Vertical slider naturally aligns with board height
3. **More Control**: Manual start gives players time to prepare
4. **Intuitive**: Time positions match player positions on board
5. **Professional**: Semi-transparent overlays look modern and clean

## Compatibility
- Works with existing board flip feature
- Compatible with custom piece icons
- Maintains all existing time control settings persistence
- No breaking changes to existing functionality
