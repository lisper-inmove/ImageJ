# Canvas Background, Image Alignment & Boundary Clamp Design

## Overview
Three targeted improvements to ImageCanvas: replace checkerboard default background with solid gray, auto-align new images to top-left, and clamp view_offset so the image never exposes canvas edges.

## Requirements
1. **Remove checkerboard background** — default to solid dark gray
2. **New image aligns to top-left** — view_offset resets to (0,0) on set_document()
3. **Image stays within canvas bounds** — view_offset_ clamped after every change

## Changes

### 1. Default background
- `background_style_` default: `kCheckerboard` → `kSolidColor`
- `background_color_` default: `Qt::white` → `QColor(0x2D, 0x2D, 0x2D)`

### 2. Auto-align to top-left
- `set_document()`: reset `view_offset_ = QPoint(0, 0)` before `update()`

### 3. Boundary clamping
- New private method `clampViewOffset()`:
```
if no document: return
scaled_w = image_width * zoom
scaled_h = image_height * zoom
view_offset_.rx() = std::clamp(view_offset_.x(), -(scaled_w - canvas_w), 0)
view_offset_.ry() = std::clamp(view_offset_.y(), -(scaled_h - canvas_h), 0)
```
When image smaller than canvas, clamp range is invalid (min > max); skip clamping.
- Call `clampViewOffset()` in: `wheelEvent()`, `set_view_offset()`, `set_zoom_factor()`, `fit_to_window()`

## Files
| File | Action |
|------|--------|
| `include/widgets/image_canvas.h` | Add `clampViewOffset()` private method |
| `src/widgets/image_canvas.cc` | Change defaults, add reset in set_document(), implement clampViewOffset(), call it after offset/zoom changes |
| `tests/widgets/image_canvas_test.cc` | Update background & offset tests, add clamp tests |

## Tests
- Default background is solid color (not checkerboard)
- Default background color is `#2D2D2D`
- set_document resets view_offset to (0,0)
- Scroll clamp: cannot scroll image beyond canvas edges
- Zoom clamp: after zoom, offset stays within bounds
- Clamp with no document does not crash
