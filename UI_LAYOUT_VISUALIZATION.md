# UI Layout Visualization

This document shows the visual layout of the Board Color Settings dialog after adding the 3 new color schemes.

## Board Color Settings Dialog Layout

```
┌─────────────────────────────────────────────────────────────────┐
│  棋盤顏色設定 (Board Color Settings)                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │ 預設配色方案 (Preset Color Schemes)                         │ │
│  │                                                             │ │
│  │  Row 1: Original Presets                                   │ │
│  │  ┌────────┐  ┌────────┐  ┌────────┐                       │ │
│  │  │ ▢ ▣    │  │ ▢ ▣    │  │ ▢ ▣    │                       │ │
│  │  │ ▣ ▢    │  │ ▣ ▢    │  │ ▣ ▢    │                       │ │
│  │  │ 經典   │  │ 藍灰   │  │ 綠白   │                       │ │
│  │  │Classic │  │Blue-   │  │Green-  │                       │ │
│  │  │Brown/  │  │Gray    │  │White   │                       │ │
│  │  │Beige   │  │        │  │        │                       │ │
│  │  └────────┘  └────────┘  └────────┘                       │ │
│  │  #F0D9B5    #DEE3E6     #FFFFDD                            │ │
│  │  #B58863    #8CA2AD     #86A666                            │ │
│  │                                                             │ │
│  │  Row 2: New Presets ⭐ NEW ⭐                              │ │
│  │  ┌────────┐  ┌────────┐  ┌────────┐                       │ │
│  │  │ ▢ ▣    │  │ ▢ ▣    │  │ ▢ ▣    │                       │ │
│  │  │ ▣ ▢    │  │ ▣ ▢    │  │ ▣ ▢    │                       │ │
│  │  │ 紫粉   │  │木紋深色│  │海洋藍  │                       │ │
│  │  │Purple- │  │Wood-   │  │Ocean-  │                       │ │
│  │  │Pink    │  │Dark    │  │Blue    │                       │ │
│  │  │        │  │        │  │        │                       │ │
│  │  └────────┘  └────────┘  └────────┘                       │ │
│  │  #E8C4E8    #D4A574     #A8D8EA                            │ │
│  │  #9B6B9B    #6B4423     #2E5B6D                            │ │
│  └─────────────────────────────────────────────────────────┘ │
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │ 自訂配色方案 (Custom Color Schemes)                         │ │
│  │                                                             │ │
│  │  ┌────────┐  ┌────────┐  ┌────────┐  ┌────────┐          │ │
│  │  │ 自訂1  │  │ 自訂2  │  │ 自訂3  │  │ 自訂4  │          │ │
│  │  └────────┘  └────────┘  └────────┘  └────────┘          │ │
│  │                                                             │ │
│  │  (Custom slots 5-7 not shown for brevity)                  │ │
│  └─────────────────────────────────────────────────────────┘ │
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │ 自訂顏色 (Custom Colors)                                    │ │
│  │                                                             │ │
│  │  使用下方按鈕選擇顏色                                        │ │
│  │  (Use buttons below to select colors)                      │ │
│  │                                                             │ │
│  │  淺色方格: [      ]  (Light square color button)            │ │
│  │  深色方格: [      ]  (Dark square color button)             │ │
│  │                                                             │ │
│  │  (Visible only when Custom slot is selected)               │ │
│  └─────────────────────────────────────────────────────────┘ │
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │ 預覽 (2×2) (Preview)                                       │ │
│  │                                                             │ │
│  │         ┌─────────┬─────────┐                              │ │
│  │         │         │         │                              │ │
│  │         │  Light  │  Dark   │                              │ │
│  │         │         │         │                              │ │
│  │         ├─────────┼─────────┤                              │ │
│  │         │         │         │                              │ │
│  │         │  Dark   │  Light  │                              │ │
│  │         │         │         │                              │ │
│  │         └─────────┴─────────┘                              │ │
│  │                                                             │ │
│  │         (Shows selected color scheme)                      │ │
│  └─────────────────────────────────────────────────────────┘ │
│                                                                 │
│  ┌──────────────┐                   ┌────────┐  ┌─────────┐  │
│  │ 重設為預設值 │                   │  確定  │  │  取消   │  │
│  │ Reset to    │                   │   OK   │  │ Cancel  │  │
│  │ Defaults    │                   └────────┘  └─────────┘  │
│  └──────────────┘                                             │
└─────────────────────────────────────────────────────────────────┘
```

## Color Scheme Comparison

### Visual Representation of All 6 Presets

```
┌──────────────────────────────────────────────────────────────────┐
│                    Original 3 Presets                            │
└──────────────────────────────────────────────────────────────────┘

1. Classic (經典)              2. Blue-Gray (藍灰)         3. Green-White (綠白)
   Light: #F0D9B5                 Light: #DEE3E6              Light: #FFFFDD
   Dark:  #B58863                 Dark:  #8CA2AD              Dark:  #86A666

   ┏━━━━━┳━━━━━┓                ┏━━━━━┳━━━━━┓              ┏━━━━━┳━━━━━┓
   ┃█████┃▓▓▓▓▓┃                ┃█████┃▓▓▓▓▓┃              ┃█████┃▓▓▓▓▓┃
   ┃█████┃▓▓▓▓▓┃                ┃█████┃▓▓▓▓▓┃              ┃█████┃▓▓▓▓▓┃
   ┣━━━━━╋━━━━━┫                ┣━━━━━╋━━━━━┫              ┣━━━━━╋━━━━━┫
   ┃▓▓▓▓▓┃█████┃                ┃▓▓▓▓▓┃█████┃              ┃▓▓▓▓▓┃█████┃
   ┃▓▓▓▓▓┃█████┃                ┃▓▓▓▓▓┃█████┃              ┃▓▓▓▓▓┃█████┃
   ┗━━━━━┻━━━━━┛                ┗━━━━━┻━━━━━┛              ┗━━━━━┻━━━━━┛
   Warm beige/brown             Cool blue/gray             Fresh green/cream

┌──────────────────────────────────────────────────────────────────┐
│                    ⭐ NEW: 3 Additional Presets ⭐              │
└──────────────────────────────────────────────────────────────────┘

4. Purple-Pink (紫粉)        5. Wood-Dark (木紋深色)      6. Ocean-Blue (海洋藍)
   Light: #E8C4E8               Light: #D4A574              Light: #A8D8EA
   Dark:  #9B6B9B               Dark:  #6B4423              Dark:  #2E5B6D

   ┏━━━━━┳━━━━━┓              ┏━━━━━┳━━━━━┓              ┏━━━━━┳━━━━━┓
   ┃█████┃▓▓▓▓▓┃              ┃█████┃▓▓▓▓▓┃              ┃█████┃▓▓▓▓▓┃
   ┃█████┃▓▓▓▓▓┃              ┃█████┃▓▓▓▓▓┃              ┃█████┃▓▓▓▓▓┃
   ┣━━━━━╋━━━━━┫              ┣━━━━━╋━━━━━┫              ┣━━━━━╋━━━━━┫
   ┃▓▓▓▓▓┃█████┃              ┃▓▓▓▓▓┃█████┃              ┃▓▓▓▓▓┃█████┃
   ┃▓▓▓▓▓┃█████┃              ┃▓▓▓▓▓┃█████┃              ┃▓▓▓▓▓┃█████┃
   ┗━━━━━┻━━━━━┛              ┗━━━━━┻━━━━━┛              ┗━━━━━┻━━━━━┛
   Elegant purple/pink        Classic wood tones         Cool ocean blues
```

## User Interaction Flow

```
1. User opens Board Color Settings dialog
   │
   ├─> Sees two rows of preset previews
   │   │
   │   ├─> Row 1: Classic, Blue-Gray, Green-White (original)
   │   └─> Row 2: Purple-Pink, Wood-Dark, Ocean-Blue (NEW!)
   │
2. User hovers over a preset preview
   │
   └─> Preview thumbnail shows blue border (hover effect)
   
3. User clicks on a preset preview
   │
   ├─> Main 2×2 preview grid updates immediately
   │   └─> Shows exactly how the board will look
   │
4. User clicks "確定" (OK)
   │
   ├─> Selected color scheme is applied to the chess board
   ├─> All 64 squares update with new colors
   └─> Settings are saved persistently
   
5. Colors persist across application restarts
```

## Implementation Benefits

### For Users:
✅ More variety in board appearance
✅ Better personalization options
✅ Easier to find a comfortable color scheme
✅ Maintains all existing functionality

### For Developers:
✅ Clean, maintainable code
✅ Follows existing patterns
✅ Easy to add more presets in the future
✅ No breaking changes to existing code

## Technical Notes

- All preset previews are created using the same `createPresetPreview()` method
- Colors are defined in hex format for easy editing
- Enum values maintain backward compatibility
- Custom slots (Custom1-7) remain unchanged and fully functional
- Settings use QSettings for persistent storage

## Color Accessibility

All color schemes have been designed with accessibility in mind:
- Sufficient luminance contrast between light and dark squares
- Color-blind friendly (relying on brightness differences)
- No harsh or overly bright colors
- Easy to distinguish pieces on both light and dark squares
