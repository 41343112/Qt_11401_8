# UI Visual Mockup - Separate Time Control Feature

## Before Start (Initial State)

```
┌──────────────────────────────────────────────────────────────────────┐
│ 國際象棋 - 雙人對弈                                    [_] [□] [×]    │
├──────────────────────────────────────────────────────────────────────┤
│ 設定                                                                  │
├──────────────────────────────────────────────────────────────────────┤
│                                                                       │
│  ┌─────────────────────────────┐  ┌────────────────────────────┐   │
│  │                             │  │ ┌────────────────────────┐ │   │
│  │  ♜  ♞  ♝  ♛  ♚  ♝  ♞  ♜   │  │ │    時間控制            │ │   │
│  │  ♟  ♟  ♟  ♟  ♟  ♟  ♟  ♟   │  │ └────────────────────────┘ │   │
│  │  ·  ·  ·  ·  ·  ·  ·  ·   │  │                            │   │
│  │  ·  ·  ·  ·  ·  ·  ·  ·   │  │ 白方時間:                  │   │
│  │  ·  ·  ·  ·  ·  ·  ·  ·   │  │    不限時                  │   │
│  │  ·  ·  ·  ·  ·  ·  ·  ·   │  │ ├──────o──────────────┤   │   │
│  │  ♙  ♙  ♙  ♙  ♙  ♙  ♙  ♙   │  │ 0   10   20   30(分)      │   │
│  │  ♖  ♘  ♗  ♕  ♔  ♗  ♘  ♖   │  │                            │   │
│  │                             │  │ 黑方時間:                  │   │
│  └─────────────────────────────┘  │    不限時                  │   │
│                                    │ ├──────o──────────────┤   │   │
│                                    │ 0   10   20   30(分)      │   │
│                                    │                            │   │
│                                    │ 每著加秒:                  │   │
│                                    │    0秒                     │   │
│                                    │ ├──o───────────────────┤  │   │
│                                    │ 0  20  40  60(秒)         │   │
│                                    │                            │   │
│                                    │ ┌────────────────────┐    │   │
│                                    │ │      開始          │    │   │
│                                    │ └────────────────────┘    │   │
│                                    │                            │   │
│                                    │ ┌────────────────────┐    │   │
│                                    │ │     新遊戲         │    │   │
│                                    │ └────────────────────┘    │   │
│                                    └────────────────────────────┘   │
│                                                                       │
└──────────────────────────────────────────────────────────────────────┘
```

## After Clicking "開始" (Start Button)

```
┌──────────────────────────────────────────────────────────────────────┐
│ 國際象棋 - 雙人對弈                                    [_] [□] [×]    │
├──────────────────────────────────────────────────────────────────────┤
│ 設定                                                                  │
├──────────────────────────────────────────────────────────────────────┤
│                                                                       │
│                                                                       │
│              ┌─────────────────────────────┐                         │
│              │ ┌──────────┐                │                         │
│              │ │黑:10:00  │                │  ← Black time (upper    │
│              │ └──────────┘                │     left overlay)       │
│              │                             │                         │
│              │  ♜  ♞  ♝  ♛  ♚  ♝  ♞  ♜   │                         │
│              │  ♟  ♟  ♟  ♟  ♟  ♟  ♟  ♟   │                         │
│              │  ·  ·  ·  ·  ·  ·  ·  ·   │                         │
│              │  ·  ·  ·  ·  ·  ·  ·  ·   │                         │
│              │  ·  ·  ·  ·  ·  ·  ·  ·   │                         │
│              │  ·  ·  ·  ·  ·  ·  ·  ·   │                         │
│              │  ♙  ♙  ♙  ♙  ♙  ♙  ♙  ♙   │                         │
│              │  ♖  ♘  ♗  ♕  ♔  ♗  ♘  ♖   │                         │
│              │                             │                         │
│              │                ┌──────────┐ │                         │
│              │                │白:10:00  │ │  ← White time (lower   │
│              │                └──────────┘ │     right overlay)     │
│              └─────────────────────────────┘                         │
│                                                                       │
│                                                                       │
└──────────────────────────────────────────────────────────────────────┘

Note: Time control panel is hidden, board is centered
```

## UI State Diagram

```
┌──────────────────┐
│  Initial State   │
│                  │
│ - Panel visible  │
│ - Board left     │
│ - Overlays hidden│
└────────┬─────────┘
         │
         │ Click "開始"
         │ (when time control enabled)
         ▼
┌──────────────────┐
│  Playing State   │
│                  │
│ - Panel hidden   │
│ - Board centered │
│ - Overlays shown │
└────────┬─────────┘
         │
         │ Click "新遊戲"
         │ or timeout occurs
         ▼
┌──────────────────┐
│  Initial State   │
│  (returns)       │
└──────────────────┘
```

## Time Label Details

### Black Time Label (Upper Left)
```
┌──────────────┐
│  黑:10:00    │  ← Semi-transparent background
└──────────────┘     Color: rgba(51, 51, 51, 200)
                     Font: 14pt, bold
                     Padding: 8px
                     Border-radius: 5px
```

### White Time Label (Lower Right)
```
┌──────────────┐
│  白:10:00    │  ← Semi-transparent background
└──────────────┘     Color: rgba(51, 51, 51, 200)
                     Font: 14pt, bold
                     Padding: 8px
                     Border-radius: 5px
```

### Active Player Highlight
```
Active (Green):     Inactive (Gray):
┌──────────────┐   ┌──────────────┐
│  白:10:00    │   │  黑:10:00    │
└──────────────┘   └──────────────┘
rgba(76,175,80,200) rgba(51,51,51,200)
```

## Time Slider Examples

### Example 1: White 5 min, Black 10 min
```
白方時間:  5分鐘
├───────────o──────────────┤
0          5 10   20   30

黑方時間:  10分鐘  
├──────────────────o───────┤
0   5      10      20   30
```

### Example 2: White unlimited, Black 3 min
```
白方時間:  不限時
o──────────────────────────┤
0   5      10      20   30

黑方時間:  3分鐘
├─────o────────────────────┤
0   3 5    10      20   30
```

### Example 3: Both sides 30 seconds
```
白方時間:  30秒
─o─────────────────────────┤
0 1 5      10      20   30
  ^
  30s position

黑方時間:  30秒
─o─────────────────────────┤
0 1 5      10      20   30
```

## Positioning Logic

```
Board Container Coordinates:
┌─────────────────────────────┐
│(x, y)                       │
│  ┌──────────┐               │
│  │BLACK TIME│← (x+10, y+10) │
│  └──────────┘               │
│                             │
│     ┌─────────────┐         │
│     │   BOARD     │         │
│     └─────────────┘         │
│                             │
│            ┌──────────┐     │
│            │WHITE TIME│     │
│            └──────────┘     │
│         (x+w-110, y+h-50)   │
│                      (x+w, y+h)│
└─────────────────────────────┘

Where:
- (x, y) = boardWidget.geometry().topLeft()
- w = boardWidget.width()
- h = boardWidget.height()
- Margin = 10px (TIME_LABEL_MARGIN)
```

## Responsive Behavior

### On Window Resize
1. Board adjusts size automatically
2. Time labels reposition to maintain:
   - Black: 10px from top-left corner
   - White: 10px from bottom-right corner

### On Panel Show/Hide
```
Panel Visible:               Panel Hidden:
┌──────────────────┐        ┌──────────────────┐
│┌─────┐ ┌───────┐│        │    ┌─────┐       │
││Board│ │Control││        │    │Board│       │
││     │ │ Panel ││  →     │    │     │       │
││     │ │       ││        │    │     │       │
│└─────┘ └───────┘│        │    └─────┘       │
└──────────────────┘        └──────────────────┘
  Board left side             Board centered
```

## Color Scheme

### Time Labels
- **Background (inactive)**: rgba(51, 51, 51, 200) - Dark gray with transparency
- **Background (active)**: rgba(76, 175, 80, 200) - Green with transparency  
- **Text**: #FFF - White
- **Border**: None
- **Border-radius**: 5px

### Board (default Classic theme)
- **Light squares**: #F0D9B5 - Cream
- **Dark squares**: #B58863 - Brown

## Implementation Notes

### Key Classes and IDs
- Board container: `m_boardContainer`
- Board widget: `m_boardWidget`
- Time control panel: `m_timeControlPanel`
- White time label: `m_whiteTimeLabel`
- Black time label: `m_blackTimeLabel`

### Layout Hierarchy
```
QMainWindow
└── centralWidget
    └── mainLayout (QVBoxLayout)
        └── contentLayout (QHBoxLayout)
            ├── m_boardContainer (QWidget) [stretch=1]
            │   ├── boardContainerLayout (QVBoxLayout)
            │   │   └── m_boardWidget [centered]
            │   ├── m_blackTimeLabel [overlay]
            │   └── m_whiteTimeLabel [overlay]
            └── m_timeControlPanel (QWidget) [stretch=1]
                └── rightPanelLayout (QVBoxLayout)
                    └── timeControlGroup (QGroupBox)
```

## Testing Checklist

Visual Tests:
- [ ] Panel hides when start button clicked
- [ ] Board centers when panel hidden
- [ ] Black time appears in upper left
- [ ] White time appears in lower right
- [ ] Time labels have semi-transparent background
- [ ] Active player's label is green
- [ ] Inactive player's label is gray
- [ ] Labels reposition on window resize
- [ ] Panel reappears on new game

Functional Tests:
- [ ] White and black sliders work independently
- [ ] Unlimited option works for each side
- [ ] Maximum 30 minutes enforced
- [ ] Timer counts down correctly
- [ ] Increment adds correctly
- [ ] Timeout shows correct dialog
- [ ] Settings persist across restarts
