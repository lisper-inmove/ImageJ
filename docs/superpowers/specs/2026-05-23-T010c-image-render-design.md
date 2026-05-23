# T010c: ImageCanvas Image Rendering Design

## Overview
Extend ImageCanvas::paintEvent() to render document image data after drawing background.

## paintEvent flow
1. Draw background (existing checkerboard/solid/transparent logic)
2. If document_ is valid:
   - Convert via ImageDocumentAdapter to QImage
   - Apply painter transform: translate(view_offset) + scale(zoom_factor)
   - drawImage(0, 0, qimage)

## Files
| File | Action |
|------|--------|
| `src/widgets/image_canvas.cc` | Add image rendering to paintEvent(), include adapter header |
| `tests/widgets/image_canvas_test.cc` | Add render-with-document tests |

## Tests
- PaintEventWithDocument: set_document with valid ImageData, repaint — no crash
- PaintEventWithImage: create pixel data via adapter, repaint, verify no crash
- RendersAfterDocumentChange: set_document triggers update, no crash

## Dependencies
- T010a (ImageData::create()) ✅
- T010b (ImageDocumentAdapter) ✅
