# Visual UI Mockup - Time Control Improvements

## Before (Old Layout)

```
+--------------------------------------------------+
| Menu                                              |
+--------------------------------------------------+
|                                                  |
|  +--------------------+  +--------------------+  |
|  |                    |  | 時間控制           |  |
|  |                    |  | 總時間:            |  |
|  |   Chess Board      |  | [5分鐘 ▼]         |  |
|  |   (8x8)            |  |                    |  |
|  |                    |  | 每著加秒:          |  |
|  |                    |  | [5秒]              |  |
|  |                    |  | |-----o-------|    |  |
|  |                    |  |                    |  |
|  +--------------------+  | 黑方時間:          |  |
|                          | +----------------+ |  |
|  +--------------------+  | |    05:00       | |  |
|  |  新遊戲            |  | +----------------+ |  |
|  +--------------------+  | 白方時間:          |  |
|                          | +----------------+ |  |
|                          | |    05:00       | |  |
|                          | +----------------+ |  |
|                          +--------------------+  |
+--------------------------------------------------+
```

## After (New Layout)

```
+--------------------------------------------------+
| Menu                                              |
+--------------------------------------------------+
|                                                  |
|  +--------------------+  +--------------------+  |
|  | ┌──────┐           |  | 時間控制           |  |
|  | │05:00 │ ← BLACK   |  | 總時間:            |  |
|  | └──────┘           |  | [5分鐘 ▼]         |  |
|  |                    |  |                    |  |
|  |   Chess Board      |  | 每著加秒:          |  |
|  |   (8x8)            |  |   [5秒]            |  |
|  |                    |  |    ▲               |  |
|  |                    |  |    █               |  |
|  |                    |  |    █  ← Vertical   |  |
|  |       ┌──────┐     |  |    █     Slider    |  |
|  |       │05:00 │ ←   |  |    █               |  |
|  |       └──────┘     |  |    ▼               |  |
|  +────────────WHITE───+  |                    |  |
|                          | ┌────────────────┐ |  |
|  +--------------------+  | │  開始 (Start)  │ |  |
|  |  新遊戲            |  | └────────────────┘ |  |
|  +--------------------+  +--------------------+  |
+--------------------------------------------------+
```

## Detailed View: Time Displays on Board

### Black Time Display (Top-Left)
```
Position: (10px, 10px) from board top-left
┌────────────────────┐
│  ┌──────────────┐  │
│  │ 05:23        │  │ ← Semi-transparent overlay
│  └──────────────┘  │    100px × 40px
│                    │    Background: rgba(51,51,51,200)
│  □ Chess Square    │    Active: rgba(76,175,80,200)
│                    │
└────────────────────┘
```

### White Time Display (Bottom-Right)
```
Position: (boardWidth-110px, boardHeight-50px)
┌────────────────────┐
│                    │
│  □ Chess Square    │
│                    │
│  ┌──────────────┐  │
│  │        03:45 │  │ ← Semi-transparent overlay
│  └──────────────┘  │    100px × 40px
└────────────────────┘    Positioned bottom-right
```

## Vertical Slider Details

```
Time Control Panel (Right Side)
┌─────────────────────┐
│ 時間控制             │
│                     │
│ 總時間:              │
│ [5分鐘 ▼]           │
│                     │
│ 每著加秒:            │
│   [5秒]              │  ← Current value display
│                     │
│    ▲  60 sec        │  ← Top of slider
│    │                │
│    █                │
│    █  ← 50 sec      │
│    █                │
│    █                │
│    █  ← 40 sec      │
│    █                │
│    █  ← 30 sec      │
│    █                │
│    █                │
│  ╶─█─╴ ← 20 sec     │  ← Tick marks every 5 sec
│    █                │
│    █  ← 10 sec      │
│    █                │
│    █                │
│    █  ← 5 sec       │
│    ●  ← Handle      │  ← Current position
│    │                │
│    ▼  0 sec         │  ← Bottom of slider
│                     │
│ ┌─────────────────┐ │
│ │  開始 (Start)   │ │  ← Start button
│ └─────────────────┘ │
└─────────────────────┘
```

## Color Coding

### Time Display States

**Active Player (Green)**
```
┌──────────┐
│  05:23   │  Background: rgba(76, 175, 80, 200) - Green
└──────────┘  Text: White (#FFF)
```

**Inactive Player (Gray)**
```
┌──────────┐
│  05:23   │  Background: rgba(51, 51, 51, 200) - Dark Gray
└──────────┘  Text: White (#FFF)
```

**No Time Control**
```
┌──────────┐
│  --:--   │  Background: rgba(51, 51, 51, 200) - Dark Gray
└──────────┘  Text: White (#FFF)
```

### Start Button States

**Enabled (Ready to Start)**
```
┌────────────────┐
│  開始 (Start)  │  Clickable, normal button style
└────────────────┘
```

**Disabled (No Time Control)**
```
┌────────────────┐
│  開始 (Start)  │  Grayed out, not clickable
└────────────────┘
```

**Running (Timer Active)**
```
┌────────────────┐
│  進行中        │  Grayed out, shows status
└────────────────┘  (Running)
```

## Responsive Behavior

### On Window Resize

1. **Board scales** - Maintains square aspect ratio
2. **Vertical slider** - Height matches board height automatically
3. **Time displays** - Reposition to maintain:
   - Black: 10px from board top-left
   - White: 10px from board bottom-right

### Example at Different Sizes

**Small Window (520×380 minimum)**
```
Board: 280×280
Slider Height: ~280px
Time Labels: 100×40 (fixed)
```

**Medium Window (900×660 default)**
```
Board: 580×580
Slider Height: ~580px
Time Labels: 100×40 (fixed)
```

**Large Window (1200×900)**
```
Board: 820×820
Slider Height: ~820px
Time Labels: 100×40 (fixed)
```

## Animation States

### Timer Countdown
- Updates every 100ms
- Smooth number transitions
- Background color changes when active player switches

### Start Button Click
1. Click "開始"
2. Text changes to "進行中"
3. Button becomes disabled
4. Timer begins countdown
5. Active player's time display turns green

## Accessibility Features

- **High Contrast**: Semi-transparent overlays with white text
- **Clear Feedback**: Button state changes
- **Visual Hierarchy**: Active player highlighted in green
- **Non-Intrusive**: Overlays don't block important board areas
- **Responsive**: All elements scale appropriately

## User Experience Flow

```
[Start Application]
        ↓
[Select Time Limit: e.g., "5分鐘"]
        ↓
[Adjust Increment: e.g., "5秒" using vertical slider]
        ↓
[Click "新遊戲"] → Board resets, times reset
        ↓
[Click "開始"] → Timer starts, button shows "進行中"
        ↓
[Make moves] → Timer switches between players
        ↓         Increment added after each move
        ↓         Active player shows green
        ↓
[Time runs out OR Game ends]
        ↓
[Click "新遊戲"] → Repeat
```
