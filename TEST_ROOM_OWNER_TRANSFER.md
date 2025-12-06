# Room Owner Transfer Feature - Test Plan

## Overview
This document describes how to test the new room owner transfer feature that automatically promotes a guest to host when the original host leaves the room.

## Test Scenarios

### Scenario 1: Host Leaves via "Leave Room" Button (Before Game Starts)
**Setup:**
1. Player A creates a room (becomes Host)
2. Player B joins the room (becomes Guest)
3. Both players are waiting to start the game

**Test Steps:**
1. Player A (Host) clicks the "Exit Room" button
2. Observe Player B's UI

**Expected Results:**
- Player B receives a message box: "角色變更: 原房主已離開，您已成為新房主。等待新對手加入房間..."
- Player B's role changes from Guest to Host
- Player B's status label shows: "⏳ 您已成為房主，等待對手加入..."
- Player B's Start button is hidden (no opponent yet)
- Player B's Exit Room button remains visible
- Player B can now wait for a new opponent to join the same room

### Scenario 2: Host Disconnects Unexpectedly (Network Issue)
**Setup:**
1. Player A creates a room (becomes Host)
2. Player B joins the room (becomes Guest)
3. Both players are waiting to start the game

**Test Steps:**
1. Simulate Player A (Host) disconnecting by closing their application or killing the connection
2. Observe Player B's UI

**Expected Results:**
- Same as Scenario 1
- Player B is promoted to Host automatically
- Player B can continue using the room with the same room number

### Scenario 3: Guest Can Accept New Opponents After Promotion
**Setup:**
1. Complete Scenario 1 or 2 (Player B is now the Host)

**Test Steps:**
1. Player C joins the room using the room number
2. Observe Player B's UI

**Expected Results:**
- Player B (now Host) receives notification: "✅ 對手已加入，請按開始鍵開始遊戲"
- Player B's Start button appears and is enabled
- Player B can start the game normally as the new host

### Scenario 4: Multiple Disconnections
**Setup:**
1. Player A creates a room (Host)
2. Player B joins (Guest)
3. Player A leaves (Player B becomes Host)
4. Player C joins (new Guest)

**Test Steps:**
1. Player B (current Host) leaves the room
2. Observe Player C's UI

**Expected Results:**
- Player C is promoted to Host
- Player C can continue using the room
- The pattern continues indefinitely

### Scenario 5: Last Player Leaves
**Setup:**
1. Player A creates a room (Host)
2. Player B joins (Guest)
3. Player A leaves (Player B becomes Host)
4. No other players join

**Test Steps:**
1. Player B (now Host) leaves the room
2. Check server state

**Expected Results:**
- Room is deleted from server (no players remain)
- Game timers for that room are cleaned up
- Room number can be reused

## Server-Side Verification

### Check Server Logs
The server should log:
```
[NetworkManager] Opponent left the room before game started
[NetworkManager] Promoted to host, changing role from Guest to Host
```

### Network Messages
When the host leaves, the server should send:
1. To all remaining players: `{ "action": "playerLeft", "room": "<roomId>" }`
2. To the new host (first remaining player): `{ "action": "promotedToHost", "room": "<roomId>" }`

## Edge Cases to Test

### Edge Case 1: Empty Room
- Host creates room
- Host immediately leaves
- Result: Room is deleted, no promotion needed

### Edge Case 2: During Game
- This feature is designed for pre-game state
- Once game starts, disconnect handling follows existing logic
- No role promotion should occur during active games

### Edge Case 3: Rapid Succession
- Host leaves
- New host is promoted
- New host leaves before notification is acknowledged
- Result: Next player in line gets promoted

## Manual Testing Steps

1. **Setup Test Environment:**
   ```bash
   # Start the WebSocket server
   node server.js
   ```

2. **Run Multiple Clients:**
   - Open 3 instances of Qt_Chess application
   - Use different machines or virtual machines if possible

3. **Execute Test Scenarios:**
   - Follow each scenario above
   - Document any deviations from expected behavior
   - Take screenshots of notification dialogs

4. **Verify Network Messages:**
   - Use browser developer tools or WebSocket client to monitor messages
   - Verify "promotedToHost" action is sent correctly

## Automated Testing (Future Enhancement)

Consider adding automated tests for:
- Server-side room management logic
- WebSocket message routing
- Role promotion state transitions
- Edge case handling

## Known Limitations

1. Only works for 2-player rooms (current design)
2. Role promotion only happens before game starts
3. Player's piece color may need adjustment when new opponents join

## Success Criteria

✅ All test scenarios pass without errors
✅ UI updates correctly reflect role changes
✅ New host can successfully accept new opponents
✅ Room management handles edge cases gracefully
✅ No memory leaks or orphaned rooms on server
