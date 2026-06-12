# Cut History List Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the undo/redo stack with a clickable history list in the sidebar showing each cut snapshot ("1. 1920x1080 → 320x240").

**Architecture:** `MainFrame` stores `QVector<CutHistoryEntry>` (snapshot + sizes). `RightSidebar` hosts a `QListWidget` + "还原原始" `QPushButton` under a "选择历史" label in the tools tab. Clicking an entry emits `history_item_selected(int)` → `MainFrame::onHistoryItemSelected` restores the snapshot.

**Tech Stack:** Qt6 Widgets (QListWidget, QPushButton), existing RightSidebar/MainFrame architecture, ImageData copy semantics.

---

## File Structure

| File | Change | Responsibility |
|------|--------|----------------|
| `include/frames/main_frame.h` | Modify | Add `CutHistoryEntry` struct, `history_` vector, `onHistoryItemSelected` slot; remove undo/redo members |
| `src/frames/main_frame.cc` | Modify | Snapshot on cut, restore on list click; remove undo/redo code |
| `include/frames/right_sidebar.h` | Modify | Add `QListWidget*`, `QPushButton*`, `add_cut_history_entry`, `clear_cut_history`, `history_item_selected` signal |
| `src/frames/right_sidebar.cc` | Modify | Build list UI in tools tab, wire signals |
| `include/core/crop_edit_operation.h` | Delete | No longer needed |
| `src/core/crop_edit_operation.cc` | Delete | No longer needed |
| `tests/core/crop_edit_operation_test.cc` | Delete | No longer needed |

---

### Task 1: Clean up undo/redo + add history struct in MainFrame

**Files:**
- Modify: `include/frames/main_frame.h`
- Modify: `src/frames/main_frame.cc`
- Delete: `include/core/crop_edit_operation.h`
- Delete: `src/core/crop_edit_operation.cc`
- Delete: `tests/core/crop_edit_operation_test.cc`

- [ ] **Step 1: Remove CropEditOperation files**

```bash
rm include/core/crop_edit_operation.h
rm src/core/crop_edit_operation.cc
rm tests/core/crop_edit_operation_test.cc
```

- [ ] **Step 2: Update main_frame.h**

Replace the undo/redo members with history. In `include/frames/main_frame.h`:

Remove `#include <memory>` and `#include <vector>`.

Remove forward declarations: `class EditOperation;` and `class QAction;`.

Remove `onUndo()` and `onRedo()` from private slots, add:
```cpp
void onHistoryItemSelected(int index);
```

Remove the undo/redo member block:
```
std::vector<std::unique_ptr<EditOperation>> undo_stack_;
std::vector<std::unique_ptr<EditOperation>> redo_stack_;
QAction* undo_action_;
QAction* redo_action_;
```

Add the history struct and vector:
```cpp
struct CutHistoryEntry {
    int index;
    ImageData image_data;   // snapshot of full image after this cut
    QSize before_size;
    QSize after_size;
};
QVector<CutHistoryEntry> cut_history_;
QSize original_size_;  // image size when first loaded
```

- [ ] **Step 3: Update main_frame.cc**

Remove `#include "core/crop_edit_operation.h"`.

Remove `undo_action_(nullptr), redo_action_(nullptr)` from constructor initializer list.

In `setupMenuBar()`, revert undo/redo actions back to locals:
```cpp
QAction *undo_action = edit_menu->addAction("撤销");
undo_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
undo_action->setEnabled(false);

QAction *redo_action = edit_menu->addAction("重做");
redo_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
redo_action->setEnabled(false);
```

In `connectSignals()`, remove the undo/redo connections:
```
// Undo/redo
connect(undo_action_, &QAction::triggered, this, &MainFrame::onUndo);
connect(redo_action_, &QAction::triggered, this, &MainFrame::onRedo);
```

- [ ] **Step 4: Replace onCutSelection**

```cpp
void MainFrame::onCutSelection() {
  ImageDocument* doc = image_canvas_->document();
  if (!doc || !doc->is_valid()) return;

  QRect sel = image_canvas_->selection();
  if (!sel.isValid()) return;

  // Record before size for history display
  QSize before_size(doc->image_data().width(), doc->image_data().height());

  ImageData extracted = extractSelection(doc->image_data(), sel);
  doc->set_image_data(extracted);

  // Push snapshot onto cut history
  CutHistoryEntry entry;
  entry.index = cut_history_.size() + 1;
  entry.image_data.copy_from(doc->image_data());
  entry.before_size = before_size;
  entry.after_size = QSize(extracted.width(), extracted.height());
  cut_history_.append(entry);

  image_canvas_->clear_selection();

  // Reset state as if a new image was loaded
  current_colorspace_ = 0;
  image_canvas_->fit_to_window();
  original_data_.copy_from(doc->image_data());

  if (right_sidebar_) {
    right_sidebar_->reset_color_space_combo();
    right_sidebar_->enable_color_space_combo(true);
    // Add to history list widget
    QString label = QString("%1. %2x%3 → %4x%5")
                        .arg(entry.index)
                        .arg(entry.before_size.width())
                        .arg(entry.before_size.height())
                        .arg(entry.after_size.width())
                        .arg(entry.after_size.height());
    right_sidebar_->add_cut_history_entry(label);
  }

  image_canvas_->update();

  if (status_bar_) {
    const auto& data = doc->image_data();
    status_bar_->showMessage(
        QString("已裁剪至: %1x%2").arg(data.width()).arg(data.height()), 5000);
  }
}
```

- [ ] **Step 5: Replace onUndo/onRedo with onHistoryItemSelected**

Remove `onUndo()` and `onRedo()` completely. Add:

```cpp
void MainFrame::onHistoryItemSelected(int index) {
  if (index < 0 || index >= cut_history_.size()) return;

  ImageDocument* doc = image_canvas_->document();
  if (!doc) return;

  // Restore snapshot
  doc->set_image_data(cut_history_[index].image_data);

  image_canvas_->clear_selection();
  image_canvas_->fit_to_window();
  image_canvas_->update();

  current_colorspace_ = 0;
  original_data_.copy_from(doc->image_data());

  if (right_sidebar_) {
    right_sidebar_->reset_color_space_combo();
    right_sidebar_->enable_color_space_combo(true);
  }
}
```

- [ ] **Step 6: Update setupLoadedDocument to clear history**

In `setupLoadedDocument()`, replace the undo/redo clearing block:
```cpp
undo_stack_.clear();
redo_stack_.clear();
if (undo_action_) undo_action_->setEnabled(false);
if (redo_action_) redo_action_->setEnabled(false);
```
With:
```cpp
// Clear cut history for new document
cut_history_.clear();
if (doc->is_valid()) {
    original_size_ = QSize(doc->image_data().width(), doc->image_data().height());
}
if (right_sidebar_) {
    right_sidebar_->clear_cut_history();
}
```

- [ ] **Step 7: Save original_size_ when loading image**

In `setupLoadedDocument()`, also store original_size_ (already done in step 6 above):
```cpp
original_data_.copy_from(doc->image_data());  // existing
original_size_ = QSize(doc->image_data().width(), doc->image_data().height());  // add
```

- [ ] **Step 8: Build to verify compilation**

Run: `cmake --build build --target ImageJ -j$(nproc)`
Expected: Compiles successfully.

---

### Task 2: Add cut history list UI to RightSidebar

**Files:**
- Modify: `include/frames/right_sidebar.h`
- Modify: `src/frames/right_sidebar.cc`

- [ ] **Step 1: Add declarations to header**

In `include/frames/right_sidebar.h`, add public methods:
```cpp
void add_cut_history_entry(const QString& label);
void clear_cut_history();
```

Add signal:
```cpp
void history_item_selected(int index);
```

Add private slot:
```cpp
void onCutHistoryItemClicked(QListWidgetItem* item);
```

Add member variables:
```cpp
QWidget* cut_history_widget_;
QListWidget* cut_history_list_;
QPushButton* restore_original_btn_;
```

- [ ] **Step 2: Initialize in constructor initializer list**

In `src/frames/right_sidebar.cc`, add:
```cpp
cut_history_widget_(nullptr),
cut_history_list_(nullptr),
restore_original_btn_(nullptr)
```

- [ ] **Step 3: Build UI in buildUi()**

In `buildUi()`, after the CLAHE button section (after `tools_layout->addLayout(btn_layout);`) and before `tools_layout->addStretch();`, add:

```cpp
// Cut history section
QLabel* history_label = new QLabel("选择历史:", tools_tab_);
tools_layout->addWidget(history_label);

restore_original_btn_ = new QPushButton("还原原始", tools_tab_);
tools_layout->addWidget(restore_original_btn_);

cut_history_list_ = new QListWidget(tools_tab_);
cut_history_list_->setMaximumHeight(150);
cut_history_list_->setSelectionMode(QAbstractItemView::SingleSelection);
tools_layout->addWidget(cut_history_list_);

connect(cut_history_list_, &QListWidget::itemClicked,
        this, &RightSidebar::onCutHistoryItemClicked);
```

Also add `#include <QListWidget>` if not already present.

- [ ] **Step 4: Implement add_cut_history_entry**

```cpp
void RightSidebar::add_cut_history_entry(const QString& label) {
  if (!cut_history_list_) return;
  QListWidgetItem* item = new QListWidgetItem(label, cut_history_list_);
  item->setData(Qt::UserRole, cut_history_list_->count() - 1);
  cut_history_list_->addItem(item);
}
```

- [ ] **Step 5: Implement clear_cut_history**

```cpp
void RightSidebar::clear_cut_history() {
  if (cut_history_list_) {
    cut_history_list_->clear();
  }
}
```

- [ ] **Step 6: Implement onCutHistoryItemClicked**

```cpp
void RightSidebar::onCutHistoryItemClicked(QListWidgetItem* item) {
  if (!item) return;
  int index = cut_history_list_->row(item);
  emit history_item_selected(index);
}
```

- [ ] **Step 7: Wire "还原原始" button**

In `buildUi()`, add after the restore_original_btn_ creation:
```cpp
connect(restore_original_btn_, &QPushButton::clicked, this, [this]() {
    emit history_item_selected(-1);  // -1 means restore original
});
```

- [ ] **Step 8: Handle -1 (restore original) in MainFrame**

In `MainFrame::onHistoryItemSelected`, handle index == -1 at the top:
```cpp
void MainFrame::onHistoryItemSelected(int index) {
  ImageDocument* doc = image_canvas_->document();
  if (!doc) return;

  if (index == -1) {
    // Restore original image
    ImageData original;
    original.copy_from(original_data_);
    // Actually, original_data_ changes after cut. We need to store the true
    // original separately. This is handled by storing original_size_ and
    // a separate original_image_data_.
    return;
  }
  // ... rest of handler
}
```

Wait — we need to store the TRUE original `ImageData` separately since `original_data_` gets overwritten after each cut. Let me fix this.

- [ ] **Step 9: Add true original image data storage**

In `include/frames/main_frame.h`, add member:
```cpp
ImageData original_image_data_;  // true original, never overwritten after cuts
```

In `setupLoadedDocument()`, separate from `original_data_`:
```cpp
original_data_.copy_from(doc->image_data());
original_image_data_.copy_from(doc->image_data());  // true original for "restore original"
original_size_ = QSize(doc->image_data().width(), doc->image_data().height());
```

In `onHistoryItemSelected`, for index == -1:
```cpp
void MainFrame::onHistoryItemSelected(int index) {
  ImageDocument* doc = image_canvas_->document();
  if (!doc) return;

  if (index == -1) {
    doc->set_image_data(original_image_data_);
    original_data_.copy_from(original_image_data_);
  } else if (index >= 0 && index < cut_history_.size()) {
    doc->set_image_data(cut_history_[index].image_data);
    original_data_.copy_from(cut_history_[index].image_data);
  } else {
    return;
  }

  image_canvas_->clear_selection();
  image_canvas_->fit_to_window();
  image_canvas_->update();

  current_colorspace_ = 0;
  if (right_sidebar_) {
    right_sidebar_->reset_color_space_combo();
    right_sidebar_->enable_color_space_combo(true);
  }
}
```

- [ ] **Step 10: Connect signal in MainFrame**

In `MainFrame::connectSignals()`, add:
```cpp
connect(right_sidebar_, &RightSidebar::history_item_selected,
        this, &MainFrame::onHistoryItemSelected);
```

- [ ] **Step 11: Build to verify compilation**

Run: `cmake --build build --target ImageJ -j$(nproc)`
Expected: Compiles successfully.

---

### Task 3: Build and test

- [ ] **Step 1: Full test suite**

Run: `cmake --build build --target ImageJ_tests -j$(nproc) && ./build/ImageJ_tests`
Expected: All tests pass (except pre-existing flaky `MouseMoveEmitsSignal` / `MouseMoveWithOffset`).

- [ ] **Step 2: Manual verification**

- Launch app, open image
- Ctrl+1, create selection, right-click → Cut
- Verify "选择历史" section shows "1. WxH → w×h"
- Cut again → entry "2. ..." appears
- Click entry 1 → image restores to post-first-cut state
- Click "还原原始" → image restores to original
- Load new image → history cleared
