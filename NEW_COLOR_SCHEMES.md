# New Color Schemes Documentation

This document describes the 3 new board color schemes added to Qt_Chess.

## Overview
Three new preset color schemes have been added to provide more variety for players:
1. Purple/Pink (紫粉配色)
2. Wood/Dark (木紋深色)
3. Ocean/Blue (海洋藍配色)

## Color Scheme Details

### 1. Purple/Pink (紫粉配色)
An elegant color scheme featuring purple and pink tones, perfect for players who prefer softer, warm colors.

**Colors:**
- **Light Squares**: #E8C4E8 (Light purple/pink)
- **Dark Squares**: #9B6B9B (Dark purple)

**Visual Representation:**
```
╔═══════╦═══════╗
║ Light ║ Dark  ║  Light: Soft lavender-pink (#E8C4E8)
║#E8C4E8║#9B6B9B║  Dark:  Rich purple (#9B6B9B)
╠═══════╬═══════╣
║ Dark  ║ Light ║  Style: Elegant and soft
║#9B6B9B║#E8C4E8║  Mood:  Warm, calming
╚═══════╩═══════╝
```

**Best For:**
- Players who enjoy elegant, non-traditional colors
- Those who prefer warm, softer tones
- Evening/night play sessions

---

### 2. Wood/Dark (木紋深色)
A rich wooden theme with deep contrast, reminiscent of classic wooden chess boards with a modern darker appearance.

**Colors:**
- **Light Squares**: #D4A574 (Light wood)
- **Dark Squares**: #6B4423 (Dark wood/brown)

**Visual Representation:**
```
╔═══════╦═══════╗
║ Light ║ Dark  ║  Light: Honey wood (#D4A574)
║#D4A574║#6B4423║  Dark:  Deep mahogany (#6B4423)
╠═══════╬═══════╣
║ Dark  ║ Light ║  Style: Classic wood
║#6B4423║#D4A574║  Mood:  Warm, traditional
╚═══════╩═══════╝
```

**Best For:**
- Players who like traditional wooden chess boards
- Those who prefer high contrast for better visibility
- Classic chess enthusiasts
- Players who want a warmer tone than the default Classic scheme

---

### 3. Ocean/Blue (海洋藍配色)
A cool ocean-inspired theme with deep blue tones, providing excellent contrast and a modern appearance.

**Colors:**
- **Light Squares**: #A8D8EA (Light ocean blue)
- **Dark Squares**: #2E5B6D (Deep ocean blue)

**Visual Representation:**
```
╔═══════╦═══════╗
║ Light ║ Dark  ║  Light: Sky blue (#A8D8EA)
║#A8D8EA║#2E5B6D║  Dark:  Deep sea blue (#2E5B6D)
╠═══════╬═══════╣
║ Dark  ║ Light ║  Style: Modern and cool
║#2E5B6D║#A8D8EA║  Mood:  Calm, focused
╚═══════╩═══════╝
```

**Best For:**
- Players who prefer cool, calming colors
- Those who like strong visual contrast
- Modern aesthetic preferences
- Extended play sessions (easy on the eyes)

---

## Implementation Summary

### Code Changes
1. **Enum Addition**: Added `PurplePink`, `WoodDark`, and `OceanBlue` to `ColorScheme` enum
2. **Preset Settings**: Implemented color values in `getPresetSettings()` method
3. **UI Layout**: Added second row of preset previews in dialog
4. **Documentation**: Updated README and BOARD_COLOR_SETTINGS documentation

### UI Arrangement
The board color settings dialog now displays presets in two rows:

**Row 1 (Original):**
- 經典 (Classic) - Brown/Beige
- 藍灰 (Blue-Gray) - Blue/Gray
- 綠白 (Green-White) - Green/White

**Row 2 (New):**
- 紫粉 (Purple-Pink) - Purple/Pink
- 木紋深色 (Wood-Dark) - Wood/Dark
- 海洋藍 (Ocean-Blue) - Ocean/Blue

### Features Maintained
- ✅ All presets have clickable 2×2 preview thumbnails
- ✅ Hover effects show blue border
- ✅ 2×2 main preview grid updates in real-time
- ✅ Settings persist between sessions
- ✅ Custom color slots remain fully functional
- ✅ Reset to defaults functionality preserved

---

## Testing Notes

To test these new color schemes:
1. Launch Qt_Chess application
2. Click "設定" (Settings) menu → "棋盤顏色設定" (Board Color Settings)
3. View the second row of preset previews
4. Click on any of the three new color scheme thumbnails:
   - 紫粉 (Purple-Pink)
   - 木紋深色 (Wood-Dark)
   - 海洋藍 (Ocean-Blue)
5. Observe the 2×2 preview grid update
6. Click "確定" (OK) to apply the scheme to the board
7. Verify the board colors match the preview
8. Start a new game and verify colors persist

---

## Color Accessibility Considerations

All three new color schemes have been designed with the following principles:
- **Sufficient Contrast**: Light and dark squares are easily distinguishable
- **Color Blindness Friendly**: Good luminance contrast for various color vision deficiencies
- **Eye Comfort**: No overly bright or harsh colors that could cause eye strain
- **Traditional Feel**: Maintain the classic chessboard alternating pattern appearance

---

## Future Enhancements

Potential improvements for consideration:
- Allow users to rate or favorite color schemes
- Add more nature-inspired themes (sunset, forest, desert)
- Implement color scheme import/export
- Add seasonal themes
- Provide color scheme suggestions based on time of day
