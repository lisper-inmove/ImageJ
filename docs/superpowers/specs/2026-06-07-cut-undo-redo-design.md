# Cut Undo/Redo

## Goal

After performing a "cut selection" operation, the user can undo back to the pre-cut state and redo forward again.

## Architecture

### `CropEditOperation` (new concrete `EditOperation`)

Stores a snapshot of the full `ImageData` before the cut.

- **Construction**: takes pre-crop `ImageData` and target `ImageDocument*`
- **`apply()`**: replaces document's image data with stored pre-crop data (used during redo? no — during construction the cut already happened; this is a no-op or stores the cropped state). Actually: construction takes both `before` and `after` snapshots. `apply()` sets `after`, `undo()` sets `before`. This makes redo work correctly.
- **`undo()`**: restores `before` image data
- **`can_undo()`** / **`can_redo()`**: return true

### Edit history in `MainFrame`

- `undo_stack_`: `vector<unique_ptr<EditOperation>>` — operations that can be undone
- `redo_stack_`: `vector<unique_ptr<EditOperation>>` — operations that can be redone
- Promote `undo_action_` and `redo_action_` from locals in `setupMenuBar()` to member pointers

### Flow

1. Cut → snapshot pre-crop data → push `CropEditOperation` → clear redo stack → enable undo
2. Undo (Ctrl+Z) → pop undo → call `undo()` → push to redo → restore canvas state → enable redo
3. Redo (Ctrl+Y) → pop redo → call `apply()` → push to undo → restore canvas state → enable undo
4. New cut after undo → clear redo stack (standard pattern)
5. Load new image → clear both stacks

## Files

| File | Change |
|------|--------|
| `include/core/crop_edit_operation.h` | New |
| `src/core/crop_edit_operation.cc` | New |
| `include/frames/main_frame.h` | Add stacks, action pointers, slots |
| `src/frames/main_frame.cc` | Wire undo/redo, history management |
| `CMakeLists.txt` | Add new source |
