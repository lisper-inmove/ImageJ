# Wheel Zoom & Scroll Design

## Overview
Implement Ctrl+Wheel zoom (pixel-anchored), Wheel vertical scroll, and Shift+Wheel horizontal scroll in ImageCanvas.

## Requirements
1. **Ctrl + Mouse Wheel**: zoom in/out, keeping the image pixel under the cursor fixed on screen
2. **Mouse Wheel**: scroll vertically (up/down)
3. **Shift + Mouse Wheel**: scroll horizontally (left/right)

## Zoom parameters
- Step: 1.1x per notch (multiply/divide)
- Range: [0.01, 100.0]
- Scroll delta: raw `angleDelta()` (~120px per notch)

## Data flow
```
QWheelEvent → wheelEvent()
  ├─ Ctrl pressed → compute image pixel under cursor → change zoom → adjust view_offset to keep pixel fixed → update()
  ├─ Shift pressed → view_offset.rx() -= delta → update()
  └─ else → view_offset.ry() -= delta → update()
```

## Zoom anchor logic
```
canvas_pos = event->pos()                    // screen position of cursor
image_pos  = canvas_to_image(canvas_pos)      // image pixel at cursor (before zoom)
zoom *= 1.1 (or / 1.1)
clamp(zoom, 0.01, 100.0)
new_canvas = image_to_canvas(image_pos)       // where that pixel lands at new zoom
view_offset += (canvas_pos - new_canvas)       // shift to keep pixel at same screen pos
```

## Files
| File | Action |
|------|--------|
| `src/widgets/image_canvas.cc` | Replace no-op `wheelEvent()` with implementation |

## Tests
- Ctrl+wheel zoom in keeps pixel under cursor
- Ctrl+wheel zoom out keeps pixel under cursor
- Plain wheel scrolls vertically
- Shift+wheel scrolls horizontally
- Zoom clamped to [0.01, 100.0]
- wheelEvent with no document does not crash
