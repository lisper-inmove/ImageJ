# T010a: ImageData Pixel Buffer Design

## Overview
Implement the core pixel buffer in ImageData: `create()`, accessors (`data()`, `pixel()`), format helpers (`channels()`, `stride()`, `byte_count()`), and lifecycle (`clear()`, `copy_from()`). All methods are currently stubs.

## Approach
- Use `std::vector<uint8_t>` for pixel storage (already declared)
- Header `image_data.h` unchanged — interface already defined
- OpenCV NOT needed for this subtask (raw buffer only)

## Files
| File | Action |
|------|--------|
| `src/core/image_data.cc` | Rewrite all stub implementations |
| `tests/core/image_data_test.cc` | Extend tests for pixel buffer operations |

## PixelFormat channel mapping
| Format | Channels |
|--------|----------|
| kUnknown | 0 |
| kGray8 | 1 |
| kRGB24 | 3 |
| kRGBA32 | 4 |

## Tests
- `Create` allocates correct buffer size
- `CreateMultipleFormats` tests kGray8/kRGB24/kRGBA32
- `DataReturnsValidPointer` after create
- `PixelIndexing` verifies correct byte offset per (x,y)
- `ChannelsPerFormat` static mapping
- `IsValidBoundaryConditions` — zeros, unknown format, empty buffer
- `ClearResetsState` — is_valid() false after clear
- `CopyFromCopiesPixelData` — deep copy
