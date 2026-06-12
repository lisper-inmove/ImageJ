# Cut Undo/Redo Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add undo/redo history for the cut-selection operation, allowing users to revert a crop and restore the pre-crop image.

**Architecture:** A `CropEditOperation` concrete subclass of `EditOperation` stores both pre-crop and post-crop `ImageData` for symmetric undo/redo. `MainFrame` manages `undo_stack_` and `redo_stack_` (vectors of `unique_ptr<EditOperation>`), wires up the existing Ctrl+Z / Ctrl+Y menu actions, and restores canvas state on undo/redo.

**Tech Stack:** Existing `EditOperation` base class, Qt6 signals/slots, `ImageData` copy semantics.

---

## File Structure

| File | Change | Responsibility |
|------|--------|----------------|
| `include/core/crop_edit_operation.h` | Create | `CropEditOperation` class declaration |
| `src/core/crop_edit_operation.cc` | Create | `CropEditOperation` implementation |
| `include/frames/main_frame.h` | Modify | Add stack members, action pointers, undo/redo slots |
| `src/frames/main_frame.cc` | Modify | History management in `onCutSelection`, undo/redo slot implementations, wire actions |
| `tests/core/crop_edit_operation_test.cc` | Create | Unit tests for `CropEditOperation` |

Note: `PROJECT_SOURCES` and `PROJECT_HEADERS` are globbed in `config/qtconfig.cmake:16-24`, so new files in `src/core/` and `include/core/` are automatically picked up. Tests are globbed too.

---

### Task 1: Create CropEditOperation

**Files:**
- Create: `include/core/crop_edit_operation.h`
- Create: `src/core/crop_edit_operation.cc`

- [ ] **Step 1: Write the header**

In `include/core/crop_edit_operation.h`:

```cpp
#ifndef IMAGEJ_CORE_CROP_EDIT_OPERATION_H_
#define IMAGEJ_CORE_CROP_EDIT_OPERATION_H_

#include "core/edit_operation.h"
#include "core/image_data.h"

class CropEditOperation : public EditOperation {
 public:
  // Takes both pre-crop and post-crop snapshots for symmetric undo/redo.
  // Does NOT take ownership of document.
  CropEditOperation(ImageDocument* document,
                    const ImageData& before_data,
                    const ImageData& after_data);

  bool apply() override;
  bool undo() override;
  std::string description() const override;
  bool can_undo() const override { return true; }
  bool can_redo() const override { return true; }

 private:
  ImageData before_data_;
  ImageData after_data_;
};

#endif  // IMAGEJ_CORE_CROP_EDIT_OPERATION_H_
```

- [ ] **Step 2: Write the implementation**

In `src/core/crop_edit_operation.cc`:

```cpp
#include "core/crop_edit_operation.h"
#include "core/image_document.h"

CropEditOperation::CropEditOperation(ImageDocument* document,
                                     const ImageData& before_data,
                                     const ImageData& after_data)
    : before_data_(), after_data_() {
  set_target_document(document);
  before_data_.copy_from(before_data);
  after_data_.copy_from(after_data);
}

bool CropEditOperation::apply() {
  if (!target_document_ || !after_data_.is_valid()) return false;
  target_document_->set_image_data(after_data_);
  return true;
}

bool CropEditOperation::undo() {
  if (!target_document_ || !before_data_.is_valid()) return false;
  target_document_->set_image_data(before_data_);
  return true;
}

std::string CropEditOperation::description() const {
  return "裁剪";
}
```

- [ ] **Step 3: Build to verify compilation**

Run: `cmake --build build --target imagej_core -j$(nproc)`
Expected: Compiles successfully.

---

### Task 2: Add undo/redo to MainFrame

**Files:**
- Modify: `include/frames/main_frame.h`
- Modify: `src/frames/main_frame.cc`

- [ ] **Step 1: Add declarations to header**

In `include/frames/main_frame.h`, add `#include <memory>` and `#include <vector>` at the top.

Add to private slots:
```cpp
void onUndo();
void onRedo();
```

Add to private members (after `channel_gains_`):
```cpp
// Undo/redo history
std::vector<std::unique_ptr<EditOperation>> undo_stack_;
std::vector<std::unique_ptr<EditOperation>> redo_stack_;
QAction* undo_action_;
QAction* redo_action_;
```

- [ ] **Step 2: Initialize new members in constructor initializer list**

In `src/frames/main_frame.cc`, add to constructor initializer list:
```cpp
undo_action_(nullptr),
redo_action_(nullptr)
```

- [ ] **Step 3: Promote undo/redo QActions from locals to members**

In `setupMenuBar()`, change:
```cpp
QAction *undo_action = edit_menu->addAction("撤销");
undo_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
undo_action->setEnabled(false);

QAction *redo_action = edit_menu->addAction("重做");
redo_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
redo_action->setEnabled(false);
```
To:
```cpp
undo_action_ = edit_menu->addAction("撤销");
undo_action_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
undo_action_->setEnabled(false);

redo_action_ = edit_menu->addAction("重做");
redo_action_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
redo_action_->setEnabled(false);
```

- [ ] **Step 4: Connect undo/redo actions in connectSignals()**

In `connectSignals()`, add:
```cpp
connect(undo_action_, &QAction::triggered, this, &MainFrame::onUndo);
connect(redo_action_, &QAction::triggered, this, &MainFrame::onRedo);
```

- [ ] **Step 5: Modify onCutSelection to save history**

In `onCutSelection()`, before `doc->set_image_data(extracted)`:

```cpp
void MainFrame::onCutSelection() {
  ImageDocument* doc = image_canvas_->document();
  if (!doc || !doc->is_valid()) return;

  QRect sel = image_canvas_->selection();
  if (!sel.isValid()) return;

  // Snapshot pre-crop data for undo
  ImageData before_data;
  before_data.copy_from(doc->image_data());

  ImageData extracted = extractSelection(doc->image_data(), sel);

  doc->set_image_data(extracted);

  // Push onto undo stack, clear redo stack
  auto op = std::make_unique<CropEditOperation>(doc, before_data, extracted);
  undo_stack_.push_back(std::move(op));
  redo_stack_.clear();

  if (undo_action_) undo_action_->setEnabled(true);
  if (redo_action_) redo_action_->setEnabled(false);

  // ... rest of the method unchanged
```

Also add `#include "core/crop_edit_operation.h"` at the top.

- [ ] **Step 6: Implement onUndo slot**

```cpp
void MainFrame::onUndo() {
  if (undo_stack_.empty()) return;

  auto op = std::move(undo_stack_.back());
  undo_stack_.pop_back();

  if (!op->undo()) {
    // If undo fails, push back (shouldn't happen normally)
    undo_stack_.push_back(std::move(op));
    return;
  }

  redo_stack_.push_back(std::move(op));

  // Update menu state
  undo_action_->setEnabled(!undo_stack_.empty());
  redo_action_->setEnabled(!redo_stack_.empty());

  // Restore canvas state
  image_canvas_->clear_selection();
  image_canvas_->fit_to_window();
  image_canvas_->update();

  // Reset colorspace state
  current_colorspace_ = 0;
  ImageDocument* doc = image_canvas_->document();
  if (doc && doc->is_valid()) {
    original_data_.copy_from(doc->image_data());
  }
  if (right_sidebar_) {
    right_sidebar_->reset_color_space_combo();
    right_sidebar_->enable_color_space_combo(true);
  }
}
```

- [ ] **Step 7: Implement onRedo slot**

```cpp
void MainFrame::onRedo() {
  if (redo_stack_.empty()) return;

  auto op = std::move(redo_stack_.back());
  redo_stack_.pop_back();

  if (!op->apply()) {
    redo_stack_.push_back(std::move(op));
    return;
  }

  undo_stack_.push_back(std::move(op));

  // Update menu state
  undo_action_->setEnabled(!undo_stack_.empty());
  redo_action_->setEnabled(!redo_stack_.empty());

  // Restore canvas state
  image_canvas_->clear_selection();
  image_canvas_->fit_to_window();
  image_canvas_->update();

  // Reset colorspace state
  current_colorspace_ = 0;
  ImageDocument* doc = image_canvas_->document();
  if (doc && doc->is_valid()) {
    original_data_.copy_from(doc->image_data());
  }
  if (right_sidebar_) {
    right_sidebar_->reset_color_space_combo();
    right_sidebar_->enable_color_space_combo(true);
  }
}
```

- [ ] **Step 8: Clear history when loading new image**

In `setupLoadedDocument()`, add after the existing code:
```cpp
// Clear undo/redo history for new document
undo_stack_.clear();
redo_stack_.clear();
if (undo_action_) undo_action_->setEnabled(false);
if (redo_action_) redo_action_->setEnabled(false);
```

- [ ] **Step 9: Build to verify compilation**

Run: `cmake --build build --target ImageJ -j$(nproc)`
Expected: Compiles successfully.

---

### Task 3: Unit Tests

**Files:**
- Create: `tests/core/crop_edit_operation_test.cc`

- [ ] **Step 1: Write tests**

In `tests/core/crop_edit_operation_test.cc`:

```cpp
#include <gtest/gtest.h>
#include "core/crop_edit_operation.h"
#include "core/image_document.h"

TEST(CropEditOperationTest, ApplySetsAfterData) {
  ImageDocument doc;
  doc.image_data().create(10, 10, ImageData::PixelFormat::kGray8);

  ImageData before;
  before.create(10, 10, ImageData::PixelFormat::kGray8);
  // Fill before with value 128
  for (int i = 0; i < 100; ++i) before.data()[i] = 128;

  ImageData after;
  after.create(5, 5, ImageData::PixelFormat::kGray8);
  for (int i = 0; i < 25; ++i) after.data()[i] = 200;

  // Setup doc with before data
  doc.set_image_data(before);
  EXPECT_EQ(doc.image_data().width(), 10);
  EXPECT_EQ(doc.image_data().data()[0], 128);

  CropEditOperation op(&doc, before, after);

  // Apply: should set doc to after (5x5, value 200)
  EXPECT_TRUE(op.apply());
  EXPECT_EQ(doc.image_data().width(), 5);
  EXPECT_EQ(doc.image_data().data()[0], 200);
}

TEST(CropEditOperationTest, UndoRestoresBeforeData) {
  ImageDocument doc;
  doc.image_data().create(10, 10, ImageData::PixelFormat::kGray8);

  ImageData before;
  before.create(10, 10, ImageData::PixelFormat::kGray8);
  for (int i = 0; i < 100; ++i) before.data()[i] = 128;

  ImageData after;
  after.create(5, 5, ImageData::PixelFormat::kGray8);
  for (int i = 0; i < 25; ++i) after.data()[i] = 200;

  doc.set_image_data(after);
  EXPECT_EQ(doc.image_data().width(), 5);

  CropEditOperation op(&doc, before, after);

  // Undo: should restore to before (10x10, value 128)
  EXPECT_TRUE(op.undo());
  EXPECT_EQ(doc.image_data().width(), 10);
  EXPECT_EQ(doc.image_data().data()[0], 128);
}

TEST(CropEditOperationTest, Description) {
  ImageDocument doc;
  ImageData before, after;
  before.create(10, 10, ImageData::PixelFormat::kGray8);
  after.create(5, 5, ImageData::PixelFormat::kGray8);

  CropEditOperation op(&doc, before, after);
  EXPECT_EQ(op.description(), "裁剪");
}

TEST(CropEditOperationTest, CanUndoRedo) {
  ImageDocument doc;
  ImageData before, after;
  before.create(10, 10, ImageData::PixelFormat::kGray8);
  after.create(5, 5, ImageData::PixelFormat::kGray8);

  CropEditOperation op(&doc, before, after);
  EXPECT_TRUE(op.can_undo());
  EXPECT_TRUE(op.can_redo());
}

TEST(CropEditOperationTest, ApplyFailsWithNullDocument) {
  ImageData before, after;
  before.create(10, 10, ImageData::PixelFormat::kGray8);
  after.create(5, 5, ImageData::PixelFormat::kGray8);

  CropEditOperation op(nullptr, before, after);
  EXPECT_FALSE(op.apply());
}

TEST(CropEditOperationTest, UndoFailsWithInvalidBeforeData) {
  ImageDocument doc;
  ImageData before; // empty
  ImageData after;
  after.create(5, 5, ImageData::PixelFormat::kGray8);

  CropEditOperation op(&doc, before, after);
  EXPECT_FALSE(op.undo());
}
```

- [ ] **Step 2: Run tests**

Run: `cmake --build build --target ImageJ_tests -j$(nproc) && ./build/ImageJ_tests --gtest_filter="*CropEditOperation*"`
Expected: 6 tests pass.

- [ ] **Step 3: Run all tests to confirm no regressions**

Run: `cmake --build build --target ImageJ_tests -j$(nproc) && ./build/ImageJ_tests`
Expected: All existing tests pass.

---

### Task 4: Manual Verification

- [ ] **Step 1: Launch app and open an image**
- [ ] **Step 2: Verify undo/redo menu items are disabled initially**
- [ ] **Step 3: Ctrl+1, create a selection, right-click → Cut**
- [ ] **Step 4: Verify Undo (Ctrl+Z) is now enabled**
- [ ] **Step 5: Press Ctrl+Z → image restores to pre-crop state**
- [ ] **Step 6: Verify Redo (Ctrl+Y) is now enabled**
- [ ] **Step 7: Press Ctrl+Y → image returns to cropped state**
- [ ] **Step 8: Perform cut, undo, then do another cut → verify redo is now empty (disabled)**
- [ ] **Step 9: Load a new image → verify undo/redo both disabled**
