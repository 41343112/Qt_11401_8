# Visual Guide: Move List Clearing and Unlimited Time Display

## Overview
This guide demonstrates the two implemented features with examples of before and after behavior.

## Feature 1: Clear Move List on Start (清空棋譜)

### Before Implementation
```
[Initial State]
┌─────────────────────┐
│ 棋譜 (Move List)     │
│ Empty               │
└─────────────────────┘

[After Playing Some Moves]
┌─────────────────────┐
│ 棋譜 (Move List)     │
│ 1. e4 e5            │
│ 2. Nf3 Nc6          │
│ 3. Bc4 Bc5          │
└─────────────────────┘

[Click New Game → Start]
┌─────────────────────┐
│ 棋譜 (Move List)     │
│ 1. e4 e5            │  ← OLD MOVES STILL VISIBLE
│ 2. Nf3 Nc6          │  ← PROBLEM: Not cleared!
│ 3. Bc4 Bc5          │
└─────────────────────┘
```

### After Implementation ✅
```
[Initial State]
┌─────────────────────┐
│ 棋譜 (Move List)     │
│ Empty               │
└─────────────────────┘

[After Playing Some Moves]
┌─────────────────────┐
│ 棋譜 (Move List)     │
│ 1. e4 e5            │
│ 2. Nf3 Nc6          │
│ 3. Bc4 Bc5          │
└─────────────────────┘

[Click New Game → Start]
┌─────────────────────┐
│ 棋譜 (Move List)     │
│ Empty               │  ✓ FIXED: Properly cleared!
└─────────────────────┘
```

## Feature 2: Display Unlimited Time (無限制)

### Before Implementation
```
Time Control Settings:
┌────────────────────────┐
│ 白方時間: [====○══════] │ Position 6 (5 minutes)
│ 黑方時間: [○══════════] │ Position 0 (unlimited)
└────────────────────────┘

When game starts:
┌─────────┐         ┌─────────┐
│ 00:00   │ Black   │ 05:00   │ White
└─────────┘         └─────────┘
    ↑                    ↑
  PROBLEM!           Correct
  Looks like         Shows 5 min
  time expired!      remaining
```

### After Implementation ✅
```
Time Control Settings:
┌────────────────────────┐
│ 白方時間: [====○══════] │ Position 6 (5 minutes)
│ 黑方時間: [○══════════] │ Position 0 (unlimited)
└────────────────────────┘

When game starts:
┌─────────┐         ┌─────────┐
│ 無限制   │ Black   │ 05:00   │ White
└─────────┘         └─────────┘
    ↑                    ↑
  FIXED!             Correct
  Clearly shows      Shows 5 min
  unlimited time     remaining
```

## Detailed Scenarios

### Scenario A: Both Players Timed
```
Settings:
  White: Position 6 (5 minutes)
  Black: Position 4 (3 minutes)

Display During Game:
╔═══════════════════════════════════╗
║        CHESS BOARD                ║
║                                   ║
║  [03:00]  ⬛⬜⬛⬜  [05:00]        ║
║   Black   ⬜⬛⬜⬛   White         ║
║          ⬛⬜⬛⬜                 ║
╚═══════════════════════════════════╝

After 1 minute of play:
╔═══════════════════════════════════╗
║        CHESS BOARD                ║
║                                   ║
║  [02:00]  ⬛⬜⬛⬜  [04:00]        ║
║   Black   ⬜⬛⬜⬛   White         ║
║          ⬛⬜⬛⬜                 ║
╚═══════════════════════════════════╝
```

### Scenario B: One Player Unlimited
```
Settings:
  White: Position 6 (5 minutes)
  Black: Position 0 (unlimited)

Display During Game:
╔═══════════════════════════════════╗
║        CHESS BOARD                ║
║                                   ║
║  [無限制]  ⬛⬜⬛⬜  [05:00]       ║
║   Black   ⬜⬛⬜⬛   White         ║
║          ⬛⬜⬛⬜                 ║
╚═══════════════════════════════════╝

After 1 minute of play:
╔═══════════════════════════════════╗
║        CHESS BOARD                ║
║                                   ║
║  [無限制]  ⬛⬜⬛⬜  [04:00]       ║
║   Black   ⬜⬛⬜⬛   White         ║
║  (never    ⬛⬜⬛⬜   (counts)      ║
║  changes)                 (down)  ║
╚═══════════════════════════════════╝
```

### Scenario C: Both Players Unlimited
```
Settings:
  White: Position 0 (unlimited)
  Black: Position 0 (unlimited)

Display During Game:
╔═══════════════════════════════════╗
║        CHESS BOARD                ║
║                                   ║
║  [無限制]  ⬛⬜⬛⬜  [無限制]     ║
║   Black   ⬜⬛⬜⬛   White         ║
║          ⬛⬜⬛⬜                 ║
╚═══════════════════════════════════╝

Game can continue indefinitely with no time pressure.
Both displays remain at "無限制" throughout the entire game.
```

## Time Display Format Reference

| Slider Position | Time Value | Display |
|----------------|------------|---------|
| 0 | 0 ms (unlimited) | **無限制** |
| 1 | 30 seconds | 00:30 → 00:29 → ... → 0:9.8 → 0:9.7 |
| 2 | 1 minute | 01:00 → 00:59 → ... |
| 3 | 2 minutes | 02:00 → 01:59 → ... |
| ... | ... | ... |
| 6 | 5 minutes | 05:00 → 04:59 → ... |
| ... | ... | ... |
| 31 | 30 minutes | 30:00 → 29:59 → ... |

**Note**: When time drops below 10 seconds, format changes to show decimals (e.g., "0:9.8")

## User Interaction Flow

### Starting a New Game with Time Control

```
1. Launch Application
   ↓
2. Adjust Time Sliders
   White: [====○══════] 5 minutes
   Black: [○══════════] unlimited
   ↓
3. Click "開始" (Start)
   ↓
4. Move List Cleared ✓
   Board Reset ✓
   Time Displays:
   - Black: "無限制" ✓
   - White: "05:00" ✓
   ↓
5. Game Begins
   - White's timer counts down
   - Black's time stays "無限制"
```

### Starting a Game After Previous Moves

```
1. Game in Progress
   Move List:
   ┌──────────────┐
   │ 1. e4 e5     │
   │ 2. Nf3 Nc6   │
   │ 3. Bc4 Bc5   │
   └──────────────┘
   ↓
2. Click "遊戲" → "新遊戲"
   ↓
3. Adjust Time Settings (if needed)
   ↓
4. Click "開始" (Start)
   ↓
5. Move List Cleared ✓
   ┌──────────────┐
   │ Empty        │
   └──────────────┘
   ↓
6. Fresh Game Ready
```

## Technical Behavior

### Timer Countdown Logic
```
Every 100ms (10 times per second):

IF current_player == White THEN
    IF white_time_ms > 0 THEN
        white_time_ms -= 100
        Update display
    ELSE
        Display "無限制"
        No countdown
    END IF
END IF

IF current_player == Black THEN
    IF black_time_ms > 0 THEN
        black_time_ms -= 100
        Update display
    ELSE
        Display "無限制"
        No countdown
    END IF
END IF
```

### Move List Clearing Trigger
```
onStartButtonClicked() {
    // Reset board state
    resetBoardState();
    
    // Clear move list widget ← NEW CODE
    if (m_moveListWidget != null) {
        m_moveListWidget.clear();
    }
    
    // Continue with normal start logic...
}
```

## Summary

Both features work together to provide a better user experience:

1. **Move List Clearing**: Ensures clean slate for each new game
2. **Unlimited Time Display**: Makes it clear when a player has no time limit

The changes are minimal (10 lines), focused, and don't affect other functionality.
