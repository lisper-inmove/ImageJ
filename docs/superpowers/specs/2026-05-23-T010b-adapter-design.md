# T010b: OpenCV Link + ImageDocumentAdapter Design

## Overview
Link OpenCV to imagej_core and implement ImageDocumentAdapter for QImage ↔ ImageData conversion.

## CMake Change
Single line: add `${OpenCV_LIBS}` to `imagej_core` target_link_libraries in `config/main.cmake`.

## ImageDocumentAdapter Implementation

### Format mapping
| ImageData::PixelFormat | QImage::Format |
|------------------------|----------------|
| kGray8 | Format_Grayscale8 |
| kRGB24 | Format_RGB888 |
| kRGBA32 | Format_RGBA8888 |
| kUnknown | Format_Invalid |

### Key methods
- `to_qimage()` — wraps existing ImageData pixel buffer into QImage (no copy)
- `update_from_qimage()` — converts QImage, allocates ImageData via `create()`, copies pixels
- `is_valid()` — document_ non-null and document valid

### Dependencies
- T010a (ImageData::create()) — completed

## Files
| File | Action |
|------|--------|
| `config/main.cmake` | Add `${OpenCV_LIBS}` link |
| `src/core/image_document_adapter.cc` | Implement all stub methods |
| `tests/core/image_document_adapter_test.cc` | Expand tests |

## Tests
- Format mapping round-trip (every known format)
- to_qimage() from valid document returns non-null QImage
- update_from_qimage() writes pixels to document
- to_qimage → update_from_qimage round-trip (pixel data preserved)
- Null/invalid document handling
