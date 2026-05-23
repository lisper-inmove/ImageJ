# T010d: Image Loading Flow Design

## Overview
Implement load_from_file() with cv::imread, connect "打开" actions to QFileDialog, update window title and status bar.

## Data flow
QFileDialog → path → cv::imread → cv::Mat → BGR→RGB → QImage → Adapter → ImageData → set_document → title/statusbar update

## Implementation
- `ImageDocument::load_from_file()`: cv::imread, BGR→RGB, adapter, metadata
- `MainFrame::connectSignals()`: connect open action → file dialog → load → display
- `MainFrame::openImage()`: new private slot

## Files
| File | Action |
|------|--------|
| `src/core/image_document.cc` | Implement load_from_file() |
| `include/frames/main_frame.h` | Add openImage() slot declaration |
| `src/frames/main_frame.cc` | Implement connectSignals() + openImage() |

## Tests
- load valid PNG/BMP file
- load nonexistent file returns false
- metadata populated after load
