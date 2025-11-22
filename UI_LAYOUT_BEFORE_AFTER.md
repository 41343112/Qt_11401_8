# UI Layout Diagram - Before and After

## BEFORE (Original Layout)

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ 國際象棋 - 雙人對弈                                    [ _ □ X ]  ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ 設定 ▾                                                            ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃                                                                    ┃
┃  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓  ┏━━━━━━━━━━━━━━━━━━━━┓  ┃
┃  ┃ ┌──────┐                          ┃  ┃   時間控制          ┃  ┃
┃  ┃ │10:00 │ ← Overlay on board      ┃  ┃                     ┃  ┃
┃  ┃ └──────┘                          ┃  ┃ 總時間:             ┃  ┃
┃  ┃  ╔════╦════╦════╦════╦════╦════╗ ┃  ┃ [ 10分鐘 ▾ ]       ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃                     ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃ 每著加秒:           ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃ [5秒]               ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃ │                   ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃ │ ← Vertical       ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃ o    slider        ┃  ┃
┃  ┃  ║    ║    ║ ♔  ║    ║    ║    ║ ┃  ┃ │                   ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃ │                   ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃ │                   ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃ │                   ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃                     ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃                     ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃                     ┃  ┃
┃  ┃  ╚════╩════╩════╩════╩════╩════╝ ┃  ┃                     ┃  ┃
┃  ┃                          ┌──────┐ ┃  ┃                     ┃  ┃
┃  ┃         Overlay on board │10:00 │ ┃  ┃                     ┃  ┃
┃  ┃                          └──────┘ ┃  ┃ [開始]              ┃  ┃
┃  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛  ┗━━━━━━━━━━━━━━━━━━━━┛  ┃
┃                                                                    ┃
┃  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓  ┃
┃  ┃                        新遊戲                                 ┃  ┃
┃  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛  ┃
┃                                                                    ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

Problems with original design:
  ✗ Combo box limited to preset times
  ✗ Vertical slider wastes space
  ✗ Time displays overlay chess pieces
  ✗ Inconsistent UI direction
```

## AFTER (New Layout) ✨

```
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃ 國際象棋 - 雙人對弈                                    [ _ □ X ]  ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ 設定 ▾                                                            ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃                                                                    ┃
┃  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓  ┏━━━━━━━━━━━━━━━━━━━━┓  ┃
┃  ┃                                   ┃  ┃   時間控制          ┃  ┃
┃  ┃  ╔════╦════╦════╦════╦════╦════╗ ┃  ┃                     ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃ 總時間:             ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃ [5分鐘]             ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃ ├───o──────────┤   ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃ 0          3600    ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃  ↑ Horizontal       ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃    fills width      ┃  ┃
┃  ┃  ║    ║    ║ ♔  ║    ║    ║    ║ ┃  ┃                     ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃ 每著加秒:           ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃ [5秒]               ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃ ├─o────────────┤   ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃ 0            60    ┃  ┃
┃  ┃  ╠════╬════╬════╬════╬════╬════╣ ┃  ┃  ↑ Horizontal       ┃  ┃
┃  ┃  ║    ║    ║    ║    ║    ║    ║ ┃  ┃    fills width      ┃  ┃
┃  ┃  ╚════╩════╩════╩════╩════╩════╝ ┃  ┃                     ┃  ┃
┃  ┃                                   ┃  ┃ 黑方時間:           ┃  ┃
┃  ┃  Clean board - no overlays! ✨    ┃  ┃ ┌────────────────┐ ┃  ┃
┃  ┃                                   ┃  ┃ │    05:00       │ ┃  ┃
┃  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛  ┃ └────────────────┘ ┃  ┃
┃                                         ┃                     ┃  ┃
┃  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓  ┃ 白方時間:           ┃  ┃
┃  ┃         新遊戲                    ┃  ┃ ┌────────────────┐ ┃  ┃
┃  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛  ┃ │    05:00       │ ┃  ┃
┃                                         ┃ └────────────────┘ ┃  ┃
┃                                         ┃                     ┃  ┃
┃                                         ┃ [開始]              ┃  ┃
┃                                         ┗━━━━━━━━━━━━━━━━━━━━┛  ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

Improvements in new design:
  ✓ Continuous time adjustment (0-3600 seconds)
  ✓ Horizontal sliders fill available width
  ✓ Time displays in dedicated panel area
  ✓ Consistent horizontal UI direction
  ✓ Clean chessboard without overlays
  ✓ Better space utilization
```

## Key Visual Differences

### Time Limit Control
| Before | After |
|--------|-------|
| ▼ Dropdown (9 options) | ├──o──┤ Slider (continuous) |
| "10分鐘 ▾" | "5分鐘" + slider |
| Fixed width | Full width |

### Increment Control
| Before | After |
|--------|-------|
| │ Vertical | ├─o─┤ Horizontal |
| Takes vertical space | Fills horizontal width |
| Ticks on left | Ticks below |

### Time Display
| Before | After |
|--------|-------|
| Overlaid on board corners | In dedicated panel sections |
| Can obscure pieces | Never blocks board |
| Floating | Structured layout |

## Color Coding

**Active Player Timer:**
```
┌─────────────────┐
│    05:00        │  ← Green background
└─────────────────┘     rgba(76, 175, 80, 200)
```

**Inactive Player Timer:**
```
┌─────────────────┐
│    05:00        │  ← Gray background
└─────────────────┘     rgba(51, 51, 51, 200)
```

## Responsive Behavior

Both sliders automatically expand to fill the available width in the time control panel:

```
Small window:
├─o──┤  (compact but usable)

Medium window:
├───o────┤  (comfortable)

Large window:
├─────o──────┤  (spacious)
```

## Implementation Details

### Total Time Slider
- Range: 0 to 3600 seconds
- 0 = "不限時" (unlimited)
- < 60 = "X秒"
- ≥ 60 and divisible by 60 = "X分鐘"
- Mixed = "X分Y秒"
- Ticks every 300 seconds (5 minutes)

### Increment Slider
- Range: 0 to 60 seconds
- Display: "X秒"
- Ticks every 5 seconds

### Layout
Both sliders use `QSlider(Qt::Horizontal)` and are added to a `QVBoxLayout`, which automatically expands them horizontally to fill the group box width.
