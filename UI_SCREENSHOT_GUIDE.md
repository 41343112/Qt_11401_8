# UI Screenshot Guide

## What to Look For

When taking screenshots to verify the implementation, capture these views:

### 1. Application Startup (No Game)
**Location**: Main window after launch  
**Expected**: 
- Left panel shows "回放控制" title
- Four buttons (⏮ ◀ ▶ ⏭) visible but grayed out (disabled)
- Move list is empty

### 2. During Active Game
**Location**: Main window during gameplay  
**Expected**:
- Buttons remain visible on left panel
- All four buttons still grayed out (disabled)
- Move list shows current moves
- Chess board shows game in progress

### 3. Game Ended (Not in Replay)
**Location**: Main window after game ends  
**Expected**:
- Buttons visible and now colored/clickable (enabled)
- "匯出 PGN" and "複製棋譜" buttons visible
- Move list shows all moves
- Time control panel visible again

### 4. In Replay Mode - Initial Position
**Location**: After clicking "⏮" (First Step) button  
**Expected**:
- Board shows starting position
- "⏮" and "◀" buttons grayed out (at start)
- "▶" and "⏭" buttons enabled
- "退出回放" button appears
- Move list cleared or shows position indicator

### 5. In Replay Mode - Middle Position
**Location**: During replay navigation  
**Expected**:
- All four navigation buttons enabled
- Current move highlighted in list
- Board shows mid-game position

### 6. In Replay Mode - Final Position
**Location**: After clicking "⏭" (Last Step) button  
**Expected**:
- Board shows final position
- "⏮" and "◀" buttons enabled
- "▶" and "⏭" buttons grayed out (at end)
- Last move highlighted in list

## Screenshot Annotations

When annotating screenshots:
- ✅ Green checkmark = Correct/Expected
- ❌ Red X = Incorrect/Unexpected  
- 🔵 Blue circle = Point of interest
- ➡️ Arrow = Direction of change

## Button State Legend

- **Enabled**: Normal color, clickable, cursor changes to pointer
- **Disabled**: Grayed out, not clickable, cursor stays as arrow
- **Hidden**: Not visible at all (should NOT happen in this implementation)

## Verification Checklist

For each screenshot, verify:
- [ ] Buttons are visible
- [ ] Button states match expected (enabled/disabled)
- [ ] Tooltips appear on hover
- [ ] No layout issues or overlap
- [ ] Button symbols (⏮◀▶⏭) display correctly
- [ ] Consistent spacing between buttons

## Tools for Taking Screenshots

### Qt Application
- Use built-in screenshot functionality if available
- Or use OS screenshot tool (e.g., Shift+Cmd+4 on Mac, Win+Shift+S on Windows)

### Annotating
- Preview (Mac)
- Microsoft Paint (Windows)
- GIMP (Cross-platform)
- Online tools like draw.io

## File Naming Convention

Suggested names for screenshots:
```
1_startup_no_game.png
2_during_game.png
3_game_ended.png
4_replay_initial.png
5_replay_middle.png
6_replay_final.png
7_button_hover_tooltip.png
8_window_resized.png
```

## What NOT to Screenshot

- Private/sensitive information
- Unrelated parts of the screen
- Development tools/IDE (unless relevant)

## Submission

Place screenshots in a new folder:
```
Qt_Chess/screenshots/replay_buttons/
```

Or attach them to the PR as image attachments in a comment.
