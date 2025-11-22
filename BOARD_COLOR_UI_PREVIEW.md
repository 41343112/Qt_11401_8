# Board Color Settings UI Preview

## Dialog Window
```
┌─────────────────────────────────────────────────────┐
│  棋盤顏色設定                                    ×   │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌─ 預設配色方案 ────────────────────────────┐    │
│  │                                           │    │
│  │  配色方案: [經典棕褐色 ▼]                 │    │
│  │                                           │    │
│  │  ┌────┐  ┌────┐  ┌────┐                 │    │
│  │  │░▓│  │░▓│  │░▓│                 │    │
│  │  │▓░│  │▓░│  │▓░│                 │    │
│  │  └────┘  └────┘  └────┘                 │    │
│  │   經典     藍灰     綠白                  │    │
│  │                                           │    │
│  └───────────────────────────────────────────┘    │
│                                                     │
│  ┌─ 自訂顏色 ───────────────────────────────┐    │
│  │                                           │    │
│  │  淺色方格: [████████]                     │    │
│  │                                           │    │
│  │  深色方格: [████████]                     │    │
│  │                                           │    │
│  └───────────────────────────────────────────┘    │
│                                                     │
│  ┌─ 預覽 (2×2) ─────────────────────────────┐    │
│  │                                           │    │
│  │         ┌────────┬────────┐              │    │
│  │         │░░░░░░░░│████████│              │    │
│  │         │░░░░░░░░│████████│              │    │
│  │         │░░░░░░░░│████████│              │    │
│  │         │░░░░░░░░│████████│              │    │
│  │         ├────────┼────────┤              │    │
│  │         │████████│░░░░░░░░│              │    │
│  │         │████████│░░░░░░░░│              │    │
│  │         │████████│░░░░░░░░│              │    │
│  │         │████████│░░░░░░░░│              │    │
│  │         └────────┴────────┘              │    │
│  │                                           │    │
│  └───────────────────────────────────────────┘    │
│                                                     │
│  [重設為預設值]              [確定]  [取消]        │
│                                                     │
└─────────────────────────────────────────────────────┘
```

## Preset Color Schemes

### 1. 經典棕褐色 (Classic)
- Light Square: #F0D9B5 (Beige)
- Dark Square: #B58863 (Brown)
- Traditional chess board appearance
```
┌──┬──┐
│░░│██│  ░ = #F0D9B5 (Light beige)
├──┼──┤  █ = #B58863 (Brown)
│██│░░│
└──┴──┘
```

### 2. 藍灰配色 (Blue/Gray)
- Light Square: #DEE3E6 (Light gray-blue)
- Dark Square: #8CA2AD (Dark gray-blue)
- Modern, cool-toned appearance
```
┌──┬──┐
│░░│██│  ░ = #DEE3E6 (Light blue-gray)
├──┼──┤  █ = #8CA2AD (Dark blue-gray)
│██│░░│
└──┴──┘
```

### 3. 綠白配色 (Green/White)
- Light Square: #FFFFDD (Cream/White)
- Dark Square: #86A666 (Forest green)
- Fresh, natural appearance
```
┌──┬──┐
│░░│██│  ░ = #FFFFDD (Light cream)
├──┼──┤  █ = #86A666 (Forest green)
│██│░░│
└──┴──┘
```

## Dialog Features

### Interactive Elements

1. **配色方案下拉菜單 (Color Scheme Dropdown)**
   - 4 options: 經典棕褐色, 藍灰配色, 綠白配色, 自訂顏色
   - Auto-applies preset colors when selected

2. **Preset Preview Thumbnails**
   - Three small 2×2 grids (30×30 pixels per square)
   - Show visual representation of each preset
   - Help users quickly identify desired color scheme

3. **Custom Color Buttons**
   - Two buttons showing current light/dark colors
   - Click to open QColorDialog
   - Automatically switches mode to "Custom" when clicked
   - Disabled when a preset is selected

4. **Main Preview Grid**
   - Large 2×2 grid (80×80 pixels per square)
   - Real-time preview of current settings
   - Shows exactly how board will appear

5. **Reset Button**
   - 重設為預設值 (Reset to Defaults)
   - Restores Classic color scheme

6. **Action Buttons**
   - 確定 (OK): Apply changes and close
   - 取消 (Cancel): Discard changes and close

## User Interaction Flow

### Selecting a Preset
1. User opens dialog
2. User selects preset from dropdown (e.g., "藍灰配色")
3. Preview updates immediately
4. Custom color buttons are disabled
5. User clicks "確定" to apply

### Creating Custom Colors
1. User opens dialog
2. User selects "自訂顏色" from dropdown OR clicks a color button
3. Mode switches to Custom
4. Color buttons become enabled
5. User clicks "淺色方格" button
6. QColorDialog opens
7. User selects desired color
8. Preview updates in real-time
9. User repeats for "深色方格" if desired
10. User clicks "確定" to apply

### Resetting to Default
1. User clicks "重設為預設值"
2. Classic preset is selected
3. Colors revert to #F0D9B5 and #B58863
4. Preview updates
5. Dropdown shows "經典棕褐色"

## Implementation Details

### Preview Grid Layout
- QGridLayout with 2 rows × 2 columns
- Each cell is a QLabel with:
  - Minimum/Maximum size: 80×80 pixels
  - Frame style: QFrame::Box
  - Line width: 1 pixel
  - Background color set via stylesheet

### Preset Mini Previews
- Each preset has its own mini grid
- 2 rows × 2 columns
- 30×30 pixels per square
- Arranged horizontally with labels below

### Color Buttons
- QPushButton with:
  - Minimum size: 80×30 pixels
  - Background color set via stylesheet
  - 2px solid border (#333)

## Accessibility
- Clear Chinese labels (Traditional Chinese)
- Visual previews supplement text
- Large preview grid (160×160 pixels total)
- High contrast between UI elements
- Keyboard navigation supported
- Tab order: Dropdown → Light Color → Dark Color → Reset → OK → Cancel
