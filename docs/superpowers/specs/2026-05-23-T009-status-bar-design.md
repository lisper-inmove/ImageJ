# T009: Status Bar Design

## Overview
Add QStatusBar to MainFrame with default message, permanent image info label, and temporary message support.

## Layout
Menu bar → Tool bar → Splitter → **Status bar** (bottom of window, managed via QVBoxLayout)

## Components
| Position | Content | Implementation |
|----------|---------|----------------|
| Left | Default message "就绪" | `QStatusBar::showMessage("就绪")` after creation |
| Right | "图像信息" placeholder | `QStatusBar::addPermanentWidget(new QLabel("图像信息"))` |
| Overlay | Temporary messages | `QStatusBar::showMessage(msg, timeout)` — right label persists |

## Files
| File | Change |
|------|--------|
| `include/frames/main_frame.h` | Add `QStatusBar` forward declaration, `setupStatusBar()` method, `status_bar_` member |
| `src/frames/main_frame.cc` | Implement `setupStatusBar()`, add to layout |
| `tests/main_frame_test.cc` | Add StatusBarTest test suite (4 tests) |

## Tests
1. StatusBarExists — QStatusBar child found
2. DefaultMessageIsReady — `currentMessage()` returns "就绪"
3. HasPermanentImageInfoLabel — QLabel with text "图像信息" found
4. ShowTemporaryMessage — `showMessage()` displays temporary text

## Dependencies
- T004 (MainFrame) — completed
- T007 (MenuBar) — completed
- T008 (ToolBar) — completed
