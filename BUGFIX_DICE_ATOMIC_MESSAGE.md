# Third Bug Fix: Atomic Check Interruption in Move Message

## Issue Report

**Reporter**: @41343112  
**Issue**: "當攻擊對方王的時候沒有換邊"

**Translation**: "When attacking the opponent's king, it doesn't switch sides."

## Problem Analysis

Despite the previous fix adding server synchronization for check interruption, the turn still wasn't switching correctly when a check occurred. This was due to a **timing/race condition** issue.

### The Race Condition

**Previous Implementation Flow**:
```
1. Player A makes move (puts B in check)
2. Client calls movePiece() → turn switches locally to B
3. Client calls sendMove() → sends move to server
4. Server receives move:
   - Decrements diceRolls[roomId].movesRemaining
   - Checks if movesRemaining > 0
   - Decides whether to switch turn
5. Client detects check interruption (AFTER sendMove)
6. Client calls sendDiceCheckInterruption() → sends to server
7. Server receives diceCheckInterruption (TOO LATE)
   - Move already processed
   - Turn switch decision already made
```

### Why It Failed

The key issue was **message ordering**:

1. **Move message** sent first → Server processes it immediately
2. **Check interruption message** sent second → Server receives it after move processed

By the time the server received the interruption notification, it had already:
- Decremented the dice counter
- Made the turn switch decision based on old state
- Possibly kept the turn on the attacker (if dice remaining) OR switched incorrectly

Example scenario that breaks:
- Player A has 1 dice move remaining
- A moves and puts B in check
- Client sends "move" → Server: movesRemaining 1 → 0, switches to B
- Client sends "interruption" → Server: Already switched, confused state

### Server-Side Logic Conflict

In `server.js` lines 191-200:
```javascript
if(diceRolls[roomId].movesRemaining > 0) {
    diceRolls[roomId].movesRemaining--;
}
if(diceRolls[roomId].movesRemaining > 0) {
    shouldSwitchPlayer = false;  // Don't switch
} else {
    shouldSwitchPlayer = true;   // Switch
}
```

The server made the turn switch decision based on the dice count at move processing time, NOT considering the upcoming interruption message.

## Solution

### Atomic Message Design

Instead of sending two separate messages (move, then interruption), **include the interruption information in the move message itself**.

**New Flow**:
```
1. Player A makes move (puts B in check)
2. Client calls movePiece() → turn switches locally to B
3. Client detects check interruption FIRST
4. Client calls sendMove() WITH interruption flag
   → message includes: diceCheckInterruption=true, savedDiceMoves=X
5. Server receives single move message with all info:
   - Sees diceCheckInterruption flag
   - Saves interrupted player and remaining moves
   - Forces turn switch regardless of dice count
6. Result: Correct turn switch guaranteed
```

### Implementation

#### 1. Modified Client Method Signature (networkmanager.h)

```cpp
// Before:
void sendMove(const QPoint& from, const QPoint& to, 
              PieceType promotionType = PieceType::None, 
              QPoint finalPosition = QPoint(-1, -1));

// After:
void sendMove(const QPoint& from, const QPoint& to, 
              PieceType promotionType = PieceType::None, 
              QPoint finalPosition = QPoint(-1, -1),
              bool causesCheckInterruption = false,
              int savedDiceMoves = 0);
```

#### 2. Enhanced Message Format (networkmanager.cpp)

```cpp
// Added to JSON message:
if (causesCheckInterruption && savedDiceMoves > 0) {
    message["diceCheckInterruption"] = true;
    message["savedDiceMoves"] = savedDiceMoves;
}
```

#### 3. Pre-Detection in Client (qt_chess.cpp)

**Before**: Detect AFTER sendMove()
```cpp
m_networkManager->sendMove(from, to, prom, final);  // Send first

// Then detect interruption
if (opponentInCheck && !allMoved()) {
    sendDiceCheckInterruption(saved);  // Send second
}
```

**After**: Detect BEFORE sendMove()
```cpp
// Detect interruption FIRST
bool willInterrupt = false;
int saved = 0;
if (opponentInCheck && !allMoved()) {
    willInterrupt = true;
    saved = m_diceMovesRemaining;
}

// Send with interruption info
m_networkManager->sendMove(from, to, prom, final, willInterrupt, saved);
```

#### 4. Atomic Server Processing (server.js)

```javascript
if(diceRolls[roomId]) {
    // Check for interruption flag in move message
    if(msg.diceCheckInterruption && msg.savedDiceMoves > 0) {
        console.log('[Server] Check interruption detected!');
        
        // Save interrupted player state
        diceRolls[roomId].interruptedPlayer = playerWhoJustMoved;
        diceRolls[roomId].savedMovesRemaining = msg.savedDiceMoves;
        diceRolls[roomId].movesRemaining = 0;
        
        // Force turn switch
        shouldSwitchPlayer = true;
    } else {
        // Normal dice logic
        if(diceRolls[roomId].movesRemaining > 0) {
            diceRolls[roomId].movesRemaining--;
        }
        shouldSwitchPlayer = (diceRolls[roomId].movesRemaining <= 0);
    }
}
```

### Key Improvements

1. **Atomic Processing**: Interruption and move processed together
2. **No Race Condition**: Single message means single processing path
3. **Guaranteed Order**: Interruption info available when server makes turn decision
4. **Simpler Logic**: One message handler instead of two
5. **More Reliable**: No network timing dependencies

## Testing Verification

### Test Scenario
1. Player A rolls 3 dice: Knight, Bishop, Rook
2. A moves Knight ✓ (2 remaining)
3. A moves Bishop → Puts B in check ❗
4. **Verify**: 
   - Client sends move with `diceCheckInterruption=true, savedDiceMoves=1`
   - Server receives ONE message with all info
   - Server saves state: `interruptedPlayer=A, savedMovesRemaining=1`
   - Server forces `shouldSwitchPlayer=true`
   - Server switches turn to B ✓
5. B moves to escape check
6. Turn restores to A with 1 move remaining
7. A moves Rook (0 remaining)
8. Turn switches to B normally

### Expected Results After Fix
- ✅ Turn switches immediately when check occurs
- ✅ No race condition between messages
- ✅ Server has complete info to make decision
- ✅ Interrupted state saved correctly
- ✅ Game flow works as intended

## Commit Information

**Commit Hash**: 46b18c4  
**Commit Message**: Fix: Include check interruption info in move message

Move check interruption detection before sendMove() and include the flag directly in the move message. This ensures the server receives interruption info atomically with the move, preventing race conditions and ensuring proper turn switching.

## Files Modified

1. **src/networkmanager.h**: Modified `sendMove()` signature (2 new parameters)
2. **src/networkmanager.cpp**: Added interruption fields to move message (8 lines)
3. **src/qt_chess.cpp**: 
   - Moved interruption detection before `sendMove()` (20 lines)
   - Removed separate `sendDiceCheckInterruption()` calls (4 lines removed)
   - Applied to both click-move and drag-drop handlers
4. **server.js**: Check interruption flag in move handler (20 lines)

**Total Changes**: 51 lines added, 27 lines removed/modified

## Related Documentation

- [DICE_MODE_CHECK_RULE.md](docs/DICE_MODE_CHECK_RULE.md) - Feature documentation
- [BUGFIX_DICE_CHECK_MOVEMENT.md](BUGFIX_DICE_CHECK_MOVEMENT.md) - First bug fix
- [BUGFIX_DICE_SERVER_SYNC.md](BUGFIX_DICE_SERVER_SYNC.md) - Second bug fix
- [IMPLEMENTATION_SUMMARY_DICE_CHECK.md](IMPLEMENTATION_SUMMARY_DICE_CHECK.md) - Implementation summary

## Architecture Lessons

### Distributed Systems Principle

This bug demonstrates a fundamental distributed systems principle: **Atomic operations require atomic messages**.

**Anti-Pattern** (Previous approach):
- Send action A (move)
- Send action B (interruption)
- Hope they're processed in order

**Best Practice** (Current approach):
- Bundle related actions in single message
- Server has complete context for decision
- No timing dependencies

### Message Design Guidelines

1. **Complete Context**: Include all decision-making info in one message
2. **Atomic Operations**: Don't split related state changes across messages
3. **No Assumptions**: Don't assume message ordering over network
4. **Idempotent When Possible**: Design messages to be replayable
5. **Explicit State**: Include state flags rather than inferring from sequence

### Application to Future Features

Any feature that requires coordinated state changes between client and server should:
- Package related data in single message
- Avoid multi-step protocols unless necessary
- Use explicit flags for special conditions
- Process atomically on server side

## Summary

This third bug fix completes the check interruption feature by ensuring proper turn switching through atomic message design. By including interruption information directly in the move message, we eliminated the race condition that prevented correct turn switching, resulting in a robust and reliable implementation.
