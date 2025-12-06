# Test Plan: Auto-End Game on Quit - Display Room Number Only

## Overview
This document describes how to test the new feature that automatically ends the game when a player quits mid-game, and displays only the room number (not the server URL) in all messages.

## Changes Made

### 1. Auto-End Game on Opponent Disconnect (Mid-Game)
**File:** `src/qt_chess.cpp` - `onOpponentDisconnected()`

**Change:** 
- Detects if game has started (`m_gameStarted == true`)
- If game is in progress:
  - Shows message: "對手已退出遊戲\n房號: XXXX\n\n遊戲自動結束"
  - Stops game timer
  - Marks game as ended
  - Calls `handleGameEnd()` to clean up game state
- If game hasn't started:
  - Shows message: "對手已斷開連接\n房號: XXXX"
- Room number is displayed in both cases

### 2. Display Room Number When Promoted to Host
**File:** `src/qt_chess.cpp` - `onPromotedToHost()`

**Change:**
- Message now includes room number: "原房主已離開，您已成為新房主。\n房號: XXXX\n\n等待新對手加入房間..."

### 3. Remove Server URL from Room Info Dialog
**File:** `src/qt_chess.cpp` - `showRoomInfoDialog()`

**Changes:**
- Removed server URL from the detail label
- Before: "房間號碼：XXXX<br>伺服器：chess-server-mjg6.onrender.com"
- After: "房間號碼：XXXX"
- Updated room info label to show only: "🎮 房號: XXXX"
- Before: "🎮 房號: XXXX | 伺服器: chess-server-mjg6.onrender.com"

## Test Scenarios

### Scenario 1: Room Creation - Verify Room Number Display Only
**Setup:**
1. Launch Qt_Chess application
2. Click "線上" (Online) button
3. Select "我要創建房間" (Create Room)
4. Click "開始" (Start)

**Expected Results:**
- Room creation dialog appears
- Dialog shows room number in large text
- Detail section shows: "房間號碼：XXXX" (without server URL)
- Bottom status bar shows: "🎮 房號: XXXX" (without server URL)
- No mention of "chess-server-mjg6.onrender.com" anywhere

### Scenario 2: Opponent Disconnects Before Game Starts
**Setup:**
1. Player A creates a room (becomes Host)
2. Player B joins the room (becomes Guest)
3. Both players are in the waiting state (game not started)

**Test Steps:**
1. Player B closes the application or disconnects
2. Observe Player A's UI

**Expected Results:**
- Player A sees message: "對手已斷開連接\n房號: XXXX"
- Game does NOT auto-end (game hasn't started yet)
- Room number is displayed in the message

### Scenario 3: Opponent Disconnects During Active Game
**Setup:**
1. Player A creates a room (becomes Host)
2. Player B joins the room (becomes Guest)
3. Host clicks "開始對弈" to start the game
4. Game is active (at least one move has been made)

**Test Steps:**
1. Player B closes the application or disconnects
2. Observe Player A's UI

**Expected Results:**
- Player A sees message: "對手已退出遊戲\n房號: XXXX\n\n遊戲自動結束"
- Game timer stops
- Game is marked as ended
- Game end state is triggered (board layout changes, time panels move, etc.)
- Room number is displayed in the message

### Scenario 4: Host Leaves, Guest Promoted to Host
**Setup:**
1. Player A creates a room (becomes Host)
2. Player B joins the room (becomes Guest)
3. Both players are waiting to start

**Test Steps:**
1. Player A (Host) clicks "退出房間" (Exit Room) button
2. Observe Player B's UI

**Expected Results:**
- Player B sees message: "原房主已離開，您已成為新房主。\n房號: XXXX\n\n等待新對手加入房間..."
- Room number is displayed in the message
- Player B's role changes from Guest to Host
- Status label shows: "⏳ 您已成為房主，等待對手加入..."
- Start button is hidden (waiting for new opponent)
- Exit Room button remains visible

### Scenario 5: Multiple Disconnections During Game
**Setup:**
1. Player A creates room and starts game with Player B
2. Several moves have been made

**Test Steps:**
1. Player B disconnects
2. Verify message shows room number
3. Create a new room with Player A
4. Player C joins
5. Start game
6. Player A disconnects this time
7. Verify Player C sees the room number in disconnect message

**Expected Results:**
- Both disconnections during active game show: "對手已退出遊戲\n房號: XXXX\n\n遊戲自動結束"
- Room number is displayed each time
- Game auto-ends in both cases

## Edge Cases

### Edge Case 1: Disconnect Immediately After Game Start
**Test Steps:**
1. Start online game
2. Make exactly one move (so game is "started")
3. Opponent disconnects immediately

**Expected Results:**
- Should trigger auto-end game path
- Message shows: "對手已退出遊戲\n房號: XXXX\n\n遊戲自動結束"

### Edge Case 2: Disconnect During Promotion Animation
**Test Steps:**
1. Start game with time controls
2. Play until a pawn reaches promotion position
3. While promotion dialog is open, opponent disconnects

**Expected Results:**
- Game should still handle disconnect properly
- Auto-end should occur
- Room number displayed in message

### Edge Case 3: Empty Room Number
**Test Steps:**
1. Simulate a state where room number is empty (edge case bug)

**Expected Results:**
- If room number is empty, message should not show "\n房號: " at all
- Code checks: `roomNumber.isEmpty() ? "" : QString("\n房號: %1").arg(roomNumber)`

## Verification Checklist

### Room Creation
- [ ] Room info dialog shows room number without server URL
- [ ] Room info label (bottom) shows room number without server URL
- [ ] Detail section does not mention "chess-server-mjg6.onrender.com"

### Opponent Disconnect (Before Game)
- [ ] Message shows "對手已斷開連接"
- [ ] Room number is displayed: "房號: XXXX"
- [ ] Game does NOT auto-end

### Opponent Disconnect (During Game)
- [ ] Message shows "對手已退出遊戲"
- [ ] Room number is displayed: "房號: XXXX"
- [ ] Message includes "遊戲自動結束"
- [ ] Game timer stops
- [ ] `m_gameStarted` becomes `false`
- [ ] `handleGameEnd()` is called
- [ ] Board UI updates to end-game state

### Host Promotion
- [ ] Message shows "原房主已離開，您已成為新房主"
- [ ] Room number is displayed: "房號: XXXX"
- [ ] Message includes "等待新對手加入房間..."
- [ ] Role changes to Host
- [ ] Can accept new opponents

## Manual Testing Steps

1. **Setup Test Environment:**
   ```bash
   # Build the application
   cd /home/runner/work/Qt_Chess/Qt_Chess
   qmake Qt_Chess.pro
   make
   ```

2. **Run Multiple Clients:**
   - Open 2-3 instances of Qt_Chess
   - Use different machines/VMs if possible
   - Test on same machine with multiple windows

3. **Execute Each Test Scenario:**
   - Follow scenarios 1-5 above
   - Document any deviations
   - Take screenshots of messages showing room numbers

4. **Verify All Checklist Items:**
   - Go through each item in the verification checklist
   - Mark as complete only when verified
   - Note any issues or unexpected behavior

## Success Criteria

✅ Room creation shows ONLY room number (no server URL)
✅ Room info label shows ONLY room number (no server URL)
✅ Opponent disconnect before game shows room number
✅ Opponent disconnect during game auto-ends game
✅ Disconnect during game shows room number
✅ Host promotion message shows room number
✅ All messages display correctly formatted room numbers
✅ No crashes or errors occur
✅ Game state is properly cleaned up on auto-end

## Known Limitations

1. Server-side behavior unchanged (still sends same messages)
2. Only client-side display is modified
3. Room number must be available from NetworkManager

## Notes for Developers

### Code Changes Summary
- `onOpponentDisconnected()`: Added game state check and auto-end logic
- `onPromotedToHost()`: Added room number to message
- `showRoomInfoDialog()`: Removed server URL display

### Important Variables
- `m_gameStarted`: Indicates if game is in active play
- `m_networkManager->getRoomNumber()`: Gets the current room number
- `roomInfo`: Formatted string with room number for display

### Testing Tips
- Use debug output to verify `m_gameStarted` state
- Check NetworkManager has valid room number
- Verify `handleGameEnd()` is called on auto-end
- Confirm timer stops when game ends
