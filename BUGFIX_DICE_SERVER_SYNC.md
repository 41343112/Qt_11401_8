# Second Bug Fix: Server Synchronization for Dice Check Interruption

## Issue Report

**Reporter**: @41343112  
**Issue**: "修改骰子模式 當我攻擊對方的王時，應該要換成對手下棋去防止我攻擊她的王，當對方下完那一步時，我還有骰子還沒下完就應該要在換成我把剩下的骰子下完，再輪到對方骰棋子下棋"

**Translation**: "Modify dice mode - When I attack the opponent's king, it should switch to the opponent's turn to defend. When the opponent finishes that move, if I still have dice moves remaining, it should switch back to me to finish the remaining dice, then it's the opponent's turn to roll dice."

## Problem Analysis

The user was describing the EXPECTED behavior, which indicated the feature was not working correctly despite the previous bug fix.

### Root Cause

The first bug fix (commit 9f22488) only addressed the client-side piece selection issue by adding `m_diceRespondingToCheck`. However, there was a fundamental synchronization problem between client and server:

1. **Client-side logic**: Check interruption was handled locally
   - Client saved dice state
   - Client cleared local `m_diceMovesRemaining = 0`
   - Client allowed opponent to move freely

2. **Server-side logic**: Server had no knowledge of check interruption
   - Server still tracked `movesRemaining > 0` for the attacker
   - Server did NOT switch turn (lines 195-197 in server.js)
   - Server kept it as attacker's turn

3. **Result**: Opponent couldn't actually move because:
   - Server thought it was still attacker's turn
   - Server would reject opponent's move
   - Or game would become desynchronized

### The Conflict

```javascript
// Server logic (server.js lines 188-201)
if(diceRolls[roomId].movesRemaining > 0) {
    diceRolls[roomId].movesRemaining--;
}
if(diceRolls[roomId].movesRemaining > 0) {
    shouldSwitchPlayer = false;  // Don't switch!
    console.log('[Server] Dice moves remaining:', diceRolls[roomId].movesRemaining, '- NOT switching player');
}
```

When Player A put Player B in check with 1 dice move remaining:
- Client: Set local `movesRemaining = 0`, expect turn switch
- Server: Still has `movesRemaining = 1`, does NOT switch turn
- **Mismatch!** Game broken.

## Solution

Added explicit server communication for check interruption state.

### Implementation

#### 1. New Client Methods (networkmanager.h/cpp)

```cpp
void sendDiceCheckInterruption(int savedMovesRemaining);
void sendDiceCheckResolved();
```

These methods send messages to the server about check interruption state changes.

#### 2. Client Calls Server Methods (qt_chess.cpp)

**On check interruption** (lines 2596, 3747):
```cpp
// 通知伺服器骰子回合被中斷
if (m_networkManager) {
    m_networkManager->sendDiceCheckInterruption(m_diceSavedMovesRemaining);
}
```

**On check resolved** (line 6301):
```cpp
// 通知伺服器將軍已解除，恢復骰子回合
if (m_networkManager) {
    m_networkManager->sendDiceCheckResolved();
}
```

#### 3. Server Handles Interruption (server.js)

**Handle `diceCheckInterruption`** (new code):
```javascript
else if(msg.action === "diceCheckInterruption"){
    const roomId = msg.room;
    if(rooms[roomId] && diceRolls[roomId] && gameTimers[roomId]){
        // Save interrupted player and remaining moves
        diceRolls[roomId].interruptedPlayer = diceRolls[roomId].currentPlayer;
        diceRolls[roomId].savedMovesRemaining = msg.savedMovesRemaining;
        diceRolls[roomId].movesRemaining = 0;  // Clear to allow opponent to move
        
        // Force switch to opponent
        const timer = gameTimers[roomId];
        timer.currentPlayer = (timer.currentPlayer === "White") ? "Black" : "White";
        diceRolls[roomId].currentPlayer = timer.currentPlayer;
        
        // Broadcast to all clients
        rooms[roomId].forEach(client => {
            if(client.readyState === WebSocket.OPEN){
                client.send(JSON.stringify({
                    action: "diceCheckInterrupted",
                    room: roomId,
                    currentPlayer: timer.currentPlayer
                }));
            }
        });
    }
}
```

**Handle `diceCheckResolved`** (new code):
```javascript
else if(msg.action === "diceCheckResolved"){
    const roomId = msg.room;
    if(rooms[roomId] && diceRolls[roomId] && gameTimers[roomId]){
        // Restore interrupted player's turn and remaining moves
        const interruptedPlayer = diceRolls[roomId].interruptedPlayer;
        const savedMoves = diceRolls[roomId].savedMovesRemaining || 0;
        
        if(interruptedPlayer && savedMoves > 0){
            const timer = gameTimers[roomId];
            timer.currentPlayer = interruptedPlayer;
            diceRolls[roomId].currentPlayer = interruptedPlayer;
            diceRolls[roomId].movesRemaining = savedMoves;
            
            // Clear interruption state
            delete diceRolls[roomId].interruptedPlayer;
            delete diceRolls[roomId].savedMovesRemaining;
            
            // Broadcast to all clients
            rooms[roomId].forEach(client => {
                if(client.readyState === WebSocket.OPEN){
                    client.send(JSON.stringify({
                        action: "diceCheckRestored",
                        room: roomId,
                        currentPlayer: timer.currentPlayer,
                        movesRemaining: savedMoves
                    }));
                }
            });
        }
    }
}
```

## Complete Game Flow

### Normal Flow (No Check)
1. A rolls dice: Knight (K), Bishop (B), Rook (R) → 3 moves
2. A moves K → Server: movesRemaining = 2
3. A moves B → Server: movesRemaining = 1
4. A moves R → Server: movesRemaining = 0, switch to B
5. B rolls dice and plays turn

### With Check Interruption
1. A rolls dice: K, B, R → 3 moves
2. A moves K → Server: movesRemaining = 2
3. A moves B → **Puts B's king in check!**
   - Client detects check (not checkmate)
   - Client saves: R (1 move remaining)
   - **Client → Server: "diceCheckInterruption" {savedMovesRemaining: 1}**
   - **Server saves: interruptedPlayer="White", savedMovesRemaining=1**
   - **Server sets: movesRemaining=0, switches to Black**
   - **Server broadcasts turn change**
4. B moves King to escape check
   - Client detects check resolved
   - **Client → Server: "diceCheckResolved"**
   - **Server restores: currentPlayer="White", movesRemaining=1**
   - **Server broadcasts restoration**
5. A moves R → Server: movesRemaining = 0, switch to B
6. B rolls dice and plays turn

## Testing Verification

### Test Scenario
1. Start online game with dice mode
2. Player A rolls 3 pieces
3. Player A makes 2 moves
4. Player A's 2nd move puts Player B in check (not checkmate)
5. **Verify**: Turn switches to Player B (server confirms)
6. **Verify**: Player B can select and move any piece
7. Player B moves to escape check
8. **Verify**: Turn switches back to Player A (server confirms)
9. **Verify**: Player A can make final dice move
10. Player A completes final move
11. **Verify**: Turn switches to Player B
12. **Verify**: Player B rolls new dice

### Expected Results After Fix
- ✅ Server properly tracks check interruption
- ✅ Turn switches occur at server level
- ✅ Both clients stay synchronized
- ✅ Opponent can move during interruption
- ✅ Interrupted player's turn is properly restored
- ✅ Game flow matches user's description

## Commit Information

**Commit Hash**: f733f97  
**Commit Message**: Fix: Add server synchronization for dice mode check interruption

Notify server when check interruption occurs and when it's resolved. Server now properly manages turn switching during check interruption, ensuring both players can move correctly.

## Files Modified

1. **src/networkmanager.h**: Added 2 new method declarations
2. **src/networkmanager.cpp**: Implemented 2 new methods (35 lines)
3. **src/qt_chess.cpp**: Added 3 server notification calls (10 lines)
4. **server.js**: Added 2 new message handlers (70 lines)

**Total Changes**: 115 lines added across 4 files

## Related Documentation

- [DICE_MODE_CHECK_RULE.md](DICE_MODE_CHECK_RULE.md) - Feature documentation
- [BUGFIX_DICE_CHECK_MOVEMENT.md](BUGFIX_DICE_CHECK_MOVEMENT.md) - First bug fix
- [IMPLEMENTATION_SUMMARY_DICE_CHECK.md](IMPLEMENTATION_SUMMARY_DICE_CHECK.md) - Implementation summary

## Key Insights

This bug revealed an important architectural consideration: **Any game state that affects turn management must be synchronized with the server**. Client-side optimizations or shortcuts can lead to desynchronization issues in multiplayer games.

The solution demonstrates proper client-server communication:
1. Client detects game event (check interruption)
2. Client notifies server explicitly
3. Server becomes source of truth for turn state
4. Server broadcasts state changes to all clients
5. Clients stay synchronized

This pattern should be applied to any future game features that affect turn order or player actions.
