# Quick Start Guide: Board Color Settings

## 快速使用指南：棋盤顏色設定

### How to Access / 如何使用

1. Launch Qt_Chess application
2. Click **設定** (Settings) in the menu bar
3. Select **棋盤顏色設定** (Board Color Settings)

### Visual Guide / 視覺指南

```
┌──────────────────────────────────────────┐
│  Main Menu Bar                           │
│  ┌────────┬─────────┬─────────┐         │
│  │ 遊戲   │  設定 ▼│  說明   │         │
│  └────────┴─────────┴─────────┘         │
│           │                               │
│           ├─ 音效設定                    │
│           ├─ 棋子圖標設定                │
│           └─ 棋盤顏色設定  ← Click here! │
└──────────────────────────────────────────┘
```

### Three Simple Steps / 三個簡單步驟

#### Option A: Use a Preset / 使用預設配色
```
Step 1: Select from dropdown
        ┌──────────────────┐
        │ 經典棕褐色 ▼    │ ← Choose one
        └──────────────────┘
        Options:
        • 經典棕褐色 (Classic Brown/Beige)
        • 藍灰配色 (Blue/Gray)
        • 綠白配色 (Green/White)

Step 2: See preview update
        ┌────┬────┐
        │ ░░ │ ██ │
        ├────┼────┤
        │ ██ │ ░░ │
        └────┴────┘

Step 3: Click "確定" (OK)
```

#### Option B: Create Custom Colors / 自訂顏色
```
Step 1: Click color button
        淺色方格: [████]  ← Click
        
Step 2: Pick color from dialog
        ┌─────────────────┐
        │  Color Picker   │
        │  🎨             │
        │                 │
        └─────────────────┘

Step 3: Repeat for dark squares
        深色方格: [████]  ← Click

Step 4: Click "確定" (OK)
```

### Preview Explained / 預覽說明

The dialog shows **FOUR** preview grids:

```
1. Three small presets (30×30 per square)
   ┌──┐ ┌──┐ ┌──┐
   │░█│ │░█│ │░█│  ← Click dropdown to select
   │█░│ │█░│ │█░│
   └──┘ └──┘ └──┘
   經典  藍灰  綠白

2. One large preview (80×80 per square)
   ┌────────┬────────┐
   │░░░░░░░░│████████│
   │░░░░░░░░│████████│  ← Shows selected colors
   ├────────┼────────┤
   │████████│░░░░░░░░│
   │████████│░░░░░░░░│
   └────────┴────────┘
   預覽 (2×2)
```

### Color Schemes Comparison / 配色方案比較

```
Classic (經典棕褐色)          Blue/Gray (藍灰配色)
┌──┬──┐                        ┌──┬──┐
│░░│██│  #F0D9B5 / #B58863    │░░│██│  #DEE3E6 / #8CA2AD
├──┼──┤  Traditional           ├──┼──┤  Modern
│██│░░│  Warm tones            │██│░░│  Cool tones
└──┴──┘                        └──┴──┘

Green/White (綠白配色)          Custom (自訂顏色)
┌──┬──┐                        ┌──┬──┐
│░░│██│  #FFFFDD / #86A666    │░░│██│  Your choice!
├──┼──┤  Natural               ├──┼──┤  Unlimited
│██│░░│  Fresh look            │██│░░│  possibilities
└──┴──┘                        └──┴──┘
```

### Buttons Explained / 按鈕說明

```
┌──────────────────────────────────────────┐
│  [重設為預設值]              [確定] [取消]│
│      ↑                          ↑     ↑   │
│      │                          │     │   │
│  Reset to                      OK  Cancel │
│  Classic                    Apply  Discard│
└──────────────────────────────────────────┘
```

### Tips / 使用技巧

✨ **Quick Preview**: See colors before applying by looking at the 2×2 grid

🎨 **Experiment**: Try different presets to find your favorite

🔄 **Easy Reset**: Use "重設為預設值" button to go back to Classic

💾 **Auto-Save**: Your choice is saved automatically

🎯 **Pick Similar**: Match your piece icons with board colors

### Keyboard Shortcuts / 鍵盤快捷鍵

- **Tab**: Navigate between controls
- **Enter**: Activate selected button
- **Esc**: Cancel dialog
- **Alt+S**: Open Settings menu (system dependent)

### Common Use Cases / 常見用途

#### 1. Reduce Eye Strain / 減少眼睛疲勞
```
Problem: Default colors too bright
Solution: Try Blue/Gray preset
         or use darker custom colors
```

#### 2. Match Piece Icons / 配合棋子圖標
```
Problem: Pieces hard to see
Solution: Choose contrasting board colors
         e.g., Dark pieces → Light board
```

#### 3. Personal Preference / 個人喜好
```
Problem: Want unique look
Solution: Use Custom color picker
         Create your signature style!
```

#### 4. Better Screenshots / 更好的截圖
```
Problem: Need high contrast for sharing
Solution: Use Green/White preset
         or create custom high-contrast scheme
```

### Troubleshooting / 故障排除

**Q: Colors not changing?**
A: Make sure to click "確定" (OK) button

**Q: Want to undo changes?**
A: Click "取消" (Cancel) or "重設為預設值" (Reset)

**Q: Preview looks different than board?**
A: Preview is accurate - restart app if needed

**Q: Lost my custom colors?**
A: Custom colors are saved automatically
   Check if "自訂顏色" is selected in dropdown

### Compatibility / 兼容性

✅ Works with all piece icon sets
✅ Works with all sound settings
✅ Preserves game state
✅ Compatible with all game features
✅ Settings saved between sessions

### Before and After Examples / 前後對比示例

```
Default Board:                  After Blue/Gray:
┌─────────────────┐            ┌─────────────────┐
│♜♞♝♛♚♝♞♜│            │♜♞♝♛♚♝♞♜│
│♟♟♟♟♟♟♟♟│            │♟♟♟♟♟♟♟♟│
│░█░█░█░█│            │░█░█░█░█│
│█░█░█░█░│            │█░█░█░█░│
│        │            │        │
│♙♙♙♙♙♙♙♙│            │♙♙♙♙♙♙♙♙│
│♖♘♗♕♔♗♘♖│            │♖♘♗♕♔♗♘♖│
└─────────────────┘            └─────────────────┘
Brown tones                    Cool blue-gray tones
```

### Remember / 記住

🎯 **The 2×2 preview shows exactly how your board will look!**

💡 **Try all three presets before creating custom colors**

⚡ **Changes apply immediately when you click OK**

🔒 **Your settings are saved automatically**

---

## Need Help? / 需要幫助？

See detailed documentation:
- BOARD_COLOR_SETTINGS.md (Technical details)
- BOARD_COLOR_UI_PREVIEW.md (UI design)
- IMPLEMENTATION_BOARD_COLORS.md (Developer info)

---

**Enjoy your personalized chess board! / 享受您個性化的棋盤！** ♟️🎨
