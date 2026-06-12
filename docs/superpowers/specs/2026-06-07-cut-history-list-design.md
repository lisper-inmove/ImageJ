# Cut History List

## Goal

After each cut, a history entry appears in a visible list in the right sidebar. Clicking any entry restores the image to that snapshot.

## Architecture

Replace the undo/redo stack with a flat history list where clicking any entry restores that snapshot.

### CutHistoryEntry (struct in main_frame.h)

```cpp
struct CutHistoryEntry {
    int index;
    ImageData image_data;   // snapshot of FULL image AFTER this cut
    QSize before_size;
    QSize after_size;
};
```

### Components

| Component | Responsibility |
|-----------|---------------|
| `CutHistoryEntry` struct | Stores snapshot + size metadata |
| `QListWidget` + `QPushButton` in RightSidebar tools tab | UI under "选择历史" label |
| `MainFrame::onHistoryItemSelected(int)` | Restores image to clicked snapshot |
| `RightSidebar::add_history_entry(QString)` | Adds entry text to list |
| `RightSidebar::clear_history()` | Clears list on new image |
| `RightSidebar::history_item_selected(int)` signal | Notifies MainFrame |

### UI Layout

```
选择历史:
[还原原始]
1. 1920x1080 -> 320x240
2. 320x240 -> 100x100
```

### Flow

1. Cut -> snapshot post-crop data, push entry, add list item, clear selection
2. Click entry -> restore snapshot, update display
3. Click "还原原始" -> restore original_data_
4. New image loaded -> clear history

## Files

| File | Change |
|------|--------|
| `include/frames/main_frame.h` | Add struct, history vector, slot |
| `src/frames/main_frame.cc` | Snapshot on cut, restore on click |
| `include/frames/right_sidebar.h` | Add list/button widgets, methods, signal |
| `src/frames/right_sidebar.cc` | Build UI, wire signal |

## Removals

- `crop_edit_operation.h/.cc` — no longer needed
- `tests/core/crop_edit_operation_test.cc` — no longer needed
- undo_stack_/redo_stack_/undo_action_/redo_action_ — replaced by history list
