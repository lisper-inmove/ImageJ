# T005: ImageCanvas Implementation Design

## Overview
Implement the core functionality of ImageCanvas, the primary image display widget. The header interface is already fully declared; all methods are currently stubs.

## Acceptance Criteria (from todo-list)
1. ImageCanvas correctly inherits QWidget (done)
2. Implement `paintEvent` to draw checkerboard background
3. Support basic mouse events (click, move)
4. Component can correctly resize
5. Add tests to verify drawing functionality

## Design

### State
```cpp
ImageDocument* document_ = nullptr;
double zoom_factor_ = 1.0;
QPoint view_offset_ = {0, 0};
BackgroundStyle background_style_ = BackgroundStyle::kCheckerboard;
QColor background_color_ = Qt::white;
```

### Checkerboard Background (paintEvent)
- Two alternating colors: light `#CCCCCC` + dark `#999999`, 16x16px squares
- Fill entire widget area using `QPainter::fillRect` row-by-row
- When document has valid image, draw image on top of checkerboard (image rendering deferred to T010)

### Mouse Events
- `mousePressEvent`: emit `image_clicked(canvas_to_image(pos), button)`
- `mouseMoveEvent`: emit `mouse_over_image(canvas_to_image(pos))`
- `mouseReleaseEvent`: reserved for future tool support

### Coordinate Conversion
- `image_to_canvas`: `image_point * zoom_factor_ + offset_`
- `canvas_to_image`: `(canvas_point - offset_) / zoom_factor_`

### Zoom
- `set_zoom_factor`: store + `update()`
- `fit_to_window`: calculate ratio from widget size / image size
- `reset_zoom`: restore to 1.0

### Document Binding
- `set_document`: store pointer, connect change listener, emit `document_changed`
- `on_document_modified`: trigger `update()`

### Tests
- Default values after construction
- `set_zoom_factor` / `reset_zoom` / `fit_to_window` correctness
- Coordinate conversion round-trip accuracy
- Document binding and signal emission
- `paintEvent` does not crash (requires QApplication)
- Mouse event signal emission
- Resize updates `visible_image_rect`
