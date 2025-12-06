# Visual Guide: Game Mode UI Distinction

## Overview
This guide shows the visual changes made to distinguish between offline and online game modes.

## Before and After Comparison

### Before (Original Layout)
```
┌────────────────────────────────────────┐
│ ⚔ 對弈模式:                            │
│                                        │
│ ┌────────┐ ┌────────┐ ┌────────┐     │
│ │👥 雙人 │ │🤖 電腦 │ │🌐 線上 │     │
│ │(Green) │ │(Cyan)  │ │(Cyan)  │     │
│ └────────┘ └────────┘ └────────┘     │
└────────────────────────────────────────┘

Issues:
- All three buttons in one row
- No clear distinction between offline and online
- Online button uses same color as computer mode
- Users may not realize which modes require network
```

### After (Improved Layout)
```
┌────────────────────────────────────────┐
│ ⚔ 對弈模式:                            │
│                                        │
│ 💻 本機對弈 (Green label)              │
│ ┌────────┐ ┌────────┐                 │
│ │👥 雙人 │ │🤖 電腦 │                 │
│ │(Green) │ │(Cyan)  │                 │
│ └────────┘ └────────┘                 │
│                                        │
│ 🌐 連線對弈 (Pink label)               │
│ ┌────────┐                             │
│ │🌐 線上 │                             │
│ │(Pink)  │                             │
│ └────────┘                             │
└────────────────────────────────────────┘

Improvements:
✓ Clear visual grouping with category labels
✓ Offline modes grouped together
✓ Online mode has distinct pink color theme
✓ Immediately obvious which modes need network
✓ Better visual hierarchy and organization
```

## Color Coding

### 💻 本機對弈 (Local Play - Offline)
- **Label Color**: Green (THEME_ACCENT_SUCCESS)
- **👥 雙人 (Two-player)**: Green button theme
- **🤖 電腦 (Computer)**: Cyan button theme
- **Meaning**: These modes work without network connectivity

### 🌐 連線對弈 (Online Play - Online)
- **Label Color**: Pink (THEME_ACCENT_SECONDARY)
- **🌐 線上 (Online)**: Pink button theme
- **Meaning**: This mode requires network connectivity

## Button States

### Normal State
```
┌────────────────────┐
│  🤖 電腦           │
│                    │
│  (Gradient bg)     │
└────────────────────┘
```

### Checked State
```
┌────────────────────┐
│  🤖 電腦           │
│                    │
│  (Bright gradient) │
└────────────────────┘
```

### Hover State
```
┌────────────────────┐
│  🤖 電腦           │
│                    │
│  (Glow effect)     │
└────────────────────┘
```

## Label Styling

### 💻 本機對弈 (Local Play Label)
- Font size: 11px
- Color: Green (霓虹綠色)
- Padding top: 5px
- Indicates offline modes

### 🌐 連線對弈 (Online Play Label)
- Font size: 11px
- Color: Pink (霓虹粉色)
- Padding top: 8px (slightly more space)
- Indicates online mode

## Layout Hierarchy

```
Time Control Panel
│
├── ⚔ 對弈模式: (Main Title)
│   │
│   ├── 💻 本機對弈 (Offline Section)
│   │   │
│   │   └── [👥 雙人] [🤖 電腦]
│   │
│   └── 🌐 連線對弈 (Online Section)
│       │
│       └── [🌐 線上]
│
└── (Other controls below)
```

## User Experience Flow

### Choosing Offline Mode
1. User sees "💻 本機對弈" label (green)
2. Understands these modes are local/offline
3. Selects either 👥 雙人 or 🤖 電腦
4. Can play immediately without network

### Choosing Online Mode
1. User sees "🌐 連線對弈" label (pink)
2. Understands this mode requires connection
3. Selects 🌐 線上
4. System prompts for connection setup

## Design Principles Applied

1. **Clarity**: Clear labels and grouping
2. **Consistency**: Colors match functionality
3. **Hierarchy**: Visual structure guides users
4. **Accessibility**: Emojis + text + color coding
5. **Minimalism**: Clean, uncluttered layout

## Technical Notes

- All buttons maintain original size (70x45 minimum)
- Layout uses QHBoxLayout for horizontal arrangement
- Labels use QLabel with custom styling
- Online button has stretch to maintain left alignment
- Consistent with modern tech theme throughout app
