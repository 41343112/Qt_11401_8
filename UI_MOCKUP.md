# Settings Button UI Mockup

## Before the Change
```
┌─────────────────────────────────────────────────┐
│ Menu Bar: [遊戲] [⚙ 設定] [❓ 說明]             │
├─────────────────────────────────────────────────┤
│                                                 │
│  [Move List]  [Chess Board]  [Time Control]    │
│                                               │
│                                 ⏱ 遊戲設定    │
│                                 ♔ 白方時間:   │
│                                 [slider]       │
│                                 ♚ 黑方時間:   │
│                                 [slider]       │
│                                 ⏳ 每著加秒:  │
│                                 [slider]       │
│                                               │
│                                 ⚔ 對弈模式:   │
│                                 [👥雙人] [🤖電腦] [🌐線上]│
│                                               │
│                                 [▶ 開始對弈]  │
│                                 [🏳 放棄認輸]  │
│                                 [🚪 退出房間]  │
│                                                 │
└─────────────────────────────────────────────────┘
```

## After the Change
```
┌─────────────────────────────────────────────────┐
│ Menu Bar: [遊戲] [❓ 說明]                      │
├─────────────────────────────────────────────────┤
│                                                 │
│  [Move List]  [Chess Board]  [Time Control]    │
│                                               │
│                                 ⏱ 遊戲設定    │
│                                 ♔ 白方時間:   │
│                                 [slider]       │
│                                 ♚ 黑方時間:   │
│                                 [slider]       │
│                                 ⏳ 每著加秒:  │
│                                 [slider]       │
│                                               │
│                                 ⚔ 對弈模式:   │
│                                 [👥雙人] [🤖電腦] [🌐線上]│
│                                               │
│                                 [▶ 開始對弈]  │
│                                 [🏳 放棄認輸]  │
│                                 [🚪 退出房間]  │
│                                 [⚙ 設定]  ◀── NEW!
│                                                 │
└─────────────────────────────────────────────────┘

Note: Settings menu removed from menu bar, now only in button
```

## When Settings Button is Clicked
```
┌─────────────────────────────────────────────────┐
│                                 [⚙ 設定]        │
│                                 ┌──────────────┐│
│                                 │🔊 音效設定   ││
│                                 │♟ 棋子圖標設定││
│                                 │🎨 棋盤顏色設定││
│                                 ├──────────────┤│
│                                 │🔃 反轉棋盤   ││
│                                 │📺 切換全螢幕 ││
│                                 ├──────────────┤│
│                                 │🎵 背景音樂 ☑││
│                                 └──────────────┘│
└─────────────────────────────────────────────────┘
```

## Button Style Details

### Settings Button (⚙ 設定)
- **Text**: "⚙ 設定" (Settings with gear icon)
- **Background**: Gradient from dark background to neon cyan (rgba(0, 217, 255, 0.7))
- **Border**: 3px solid neon cyan (#00D9FF)
- **Border Radius**: 10px rounded corners
- **Font**: 12pt, bold
- **Height**: 45px minimum
- **Hover Effect**: Brighter cyan gradient with lighter border (#00E5FF)
- **Press Effect**: Solid cyan background

### Menu Items
Each menu item follows Qt's standard QAction appearance:
- Text with emoji icons for visual recognition
- Hover effect with highlighted background
- Separator lines between groups
- Checkable option for background music (shows checkmark when enabled)

## User Flow

1. User clicks the "⚙ 設定" button in the time control panel
2. A popup menu appears below the button
3. User selects a setting option from the menu
4. The corresponding settings dialog opens
5. User makes changes and saves
6. The menu closes automatically

## Advantages

✅ **Quick Access**: One-click access to all settings
✅ **Always Visible**: Button is always available in the game panel
✅ **Modern Design**: Matches the tech/neon aesthetic of the app
✅ **Intuitive**: Familiar gear icon for settings
✅ **Simplified Menu Bar**: Settings removed from menu bar, cleaner interface
✅ **Non-intrusive**: Takes minimal space (45px height)
✅ **Consolidated**: All settings in one place, no duplication
