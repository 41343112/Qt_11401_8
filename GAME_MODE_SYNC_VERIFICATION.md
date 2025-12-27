# Game Mode Synchronization Verification Guide

## Overview
This document describes the implementation of game mode synchronization from host to guest in online chess games.

## Problem Statement (Chinese)
當房主選擇什麼模式，房客也要執行什麼模式，並且同步棋盤，多餘的事不要做

## Problem Statement (English)
When the host chooses a game mode, the guest should execute the same mode and synchronize the board state, without doing unnecessary things.

## Implementation Summary

### Changes Made

#### 1. NetworkManager (C++)
**File: `src/networkmanager.h`**
- Updated `sendStartGame()` signature to include `const QMap<QString, bool>& gameModes` parameter
- Updated `startGameReceived` signal to include `const QMap<QString, bool>& gameModes` parameter
- Added private helper method `parseGameModesFromJson()` to eliminate code duplication

**File: `src/networkmanager.cpp`**
- Modified `sendStartGame()` to serialize game modes into JSON and include them in the message
- Added `parseGameModesFromJson()` helper to parse game modes from JSON messages
- Updated message processing in `processMessage()` to extract game modes from both new and old protocol formats
- Emits game modes to Qt_Chess via the `startGameReceived` signal

#### 2. Server (JavaScript)
**File: `server.js`**
- Modified the `startGame` action handler to forward `gameModes` from the host to all players in the room
- Game modes are included in the `gameStart` broadcast message

#### 3. Main Application (C++)
**File: `src/qt_chess.h`**
- Updated `onStartGameReceived()` signature to receive game modes

**File: `src/qt_chess.cpp`**
- Modified `onStartButtonClicked()` to send selected game modes when starting the game
- Updated `onStartGameReceived()` to store received game modes in `m_selectedGameModes`
- Added debug logging to verify mode synchronization

### Data Flow

```
1. Host selects modes in OnlineDialog
   ↓
2. Host clicks "Start Game"
   ↓
3. Qt_Chess sends game modes via NetworkManager::sendStartGame()
   ↓
4. NetworkManager serializes modes to JSON and sends to server
   ↓
5. Server receives startGame action with gameModes field
   ↓
6. Server broadcasts gameStart to all players in room (including host)
   ↓
7. Both host and guest receive gameStart message with gameModes
   ↓
8. NetworkManager parses gameModes and emits startGameReceived signal
   ↓
9. Qt_Chess::onStartGameReceived() stores modes in m_selectedGameModes
   ↓
10. Game modes are applied (霧戰, 地吸引力, etc.)
```

### Game Modes Supported
The following game modes are synchronized:
- **霧戰** (Fog of War) - Players can only see pieces in their movement range
- **地吸引力** (Gravity) - Pieces fall down after moves
- **傳送陣** (Teleportation) - Special teleportation mechanics
- **骰子** (Dice) - Random dice mechanics
- **踩地雷** (Mines) - Mine field mechanics

## Testing Instructions

### Prerequisites
- Qt5 with WebSockets module installed
- Two instances of the chess application (or two different computers)

### Test Procedure

1. **Start Host Instance**
   ```
   ./Qt_Chess
   ```
   - Click "線上對戰" (Online Play)
   - Click "創建房間" (Create Room)
   - In the game mode dialog, select one or more modes (e.g., check "霧戰" and "地吸引力")
   - Click "確定" (Confirm)
   - Copy the room number

2. **Start Guest Instance**
   ```
   ./Qt_Chess
   ```
   - Click "線上對戰" (Online Play)
   - Click "加入房間" (Join Room)
   - Paste the room number
   - Click "開始" (Start)

3. **Verify Synchronization**
   - On the host side, select a color and click "開始對弈" (Start Game)
   - Check the debug console output on both host and guest:
     - Host should log: "Host sending game modes: ..."
     - Guest should log: "Synchronized game modes: ..."
   - Verify that the selected modes are active on both sides:
     - If "霧戰" was selected, both players should see fog of war effect
     - If "地吸引力" was selected, both boards should be rotated and gravity should apply

### Expected Debug Output

**Host Console:**
```
[Qt_Chess::onStartButtonClicked] Host sending game modes:
  - 霧戰 : true
  - 地吸引力 : true
  - 傳送陣 : false
  - 骰子 : false
  - 踩地雷 : false
```

**Guest Console:**
```
[Qt_Chess::onStartGameReceived] Synchronized game modes:
  - 霧戰 : true
  - 地吸引力 : true
  - 傳送陣 : false
  - 骰子 : false
  - 踩地雷 : false
```

## Verification Checklist

- [ ] Host can select game modes in the dialog
- [ ] Game modes are sent to the server when host starts the game
- [ ] Server forwards game modes to all players
- [ ] Guest receives the same game modes as selected by host
- [ ] Both players see the same visual effects (fog, rotation, etc.)
- [ ] Game mechanics work correctly for both players
- [ ] No unnecessary actions or duplicate processing

## Code Quality

### Code Review Results
✅ No blocking issues found
⚠️ Minor nitpicks about comment language consistency (not critical)

### Security Analysis
✅ CodeQL analysis passed with 0 alerts
✅ No security vulnerabilities detected

## Files Modified

1. `src/networkmanager.h` - Updated method signatures and added helper method
2. `src/networkmanager.cpp` - Implemented game mode serialization/deserialization
3. `src/qt_chess.h` - Updated slot signature
4. `src/qt_chess.cpp` - Integrated game mode sending and receiving
5. `server.js` - Added game mode forwarding in server

## Backward Compatibility

The implementation maintains backward compatibility:
- Old clients that don't send game modes will work (server provides empty object)
- The `parseGameModesFromJson()` helper safely handles missing `gameModes` field
- Both old ("type" field) and new ("action" field) protocol formats are supported

## Date
2025-12-27
