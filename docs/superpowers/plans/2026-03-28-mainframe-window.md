# MainFrame主窗口实现计划

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 完善MainFrame主窗口，实现基本布局和窗口管理，支持配置持久化

**Architecture:** QSplitter分隔左右区域（ImageCanvas左侧80%，RightSidebar右侧20%），使用QSettings存储窗口几何信息和布局配置

**Tech Stack:** C++17, Qt6, Google C++ Style Guide, Google Test

---

## 文件结构

### 修改文件
1. `include/frames/main_frame.h` - 添加QSplitter、QSettings成员和方法声明
2. `src/frames/main_frame.cc` - 实现UI构建、配置加载/保存
3. `include/frames/right_sidebar.h` - 改为QWidget子类，添加UI成员
4. `src/frames/right_sidebar.cc` - 实现基本UI（显示"工具选项"标签）

### 测试文件
1. `tests/main_frame_test.cc` - 重新启用并扩展测试（从.disabled文件恢复）
2. `tests/right_sidebar_test.cc` - 新建测试文件

### 配置存储
使用Qt内置的QSettings，组织为：
- 应用: "ImageJ"
- 组织: "ImageJ"
- 键: Window/geometry, Window/windowState, Layout/splitterSizes

## Chunk 1: RightSidebar类重构

### Task 1: RightSidebar类重构为QWidget子类

**Files:**
- Modify: `include/frames/right_sidebar.h:1-11`
- Modify: `src/frames/right_sidebar.cc:1-4`
- Create: `tests/right_sidebar_test.cc`

- [ ] **Step 1: Write the failing test**

```cpp
// tests/right_sidebar_test.cc
#include <gtest/gtest.h>

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

#include "frames/right_sidebar.h"

class RightSidebarTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 0;
        char* argv[] = {nullptr};
        app_ = std::make_unique<QApplication>(argc, argv);
        sidebar_ = std::make_unique<RightSidebar>();
    }

    void TearDown() override {
        sidebar_.reset();
        app_.reset();
    }

    std::unique_ptr<QApplication> app_;
    std::unique_ptr<RightSidebar> sidebar_;
};

TEST_F(RightSidebarTest, InheritsFromQWidget) {
    // RightSidebar应该是QWidget的子类
    EXPECT_TRUE(dynamic_cast<QWidget*>(sidebar_.get()) != nullptr);
}

TEST_F(RightSidebarTest, ShowsPlaceholderText) {
    // 查找QLabel并验证文本
    sidebar_->show();

    // 查找子控件中的QLabel
    QLabel* label = sidebar_->findChild<QLabel*>();
    ASSERT_NE(label, nullptr);
    EXPECT_EQ(label->text().toStdString(), "工具选项");
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ && mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug && make ImageJ_tests -j4 && ./ImageJ_tests --gtest_filter="*RightSidebar*" -v`
Expected: FAIL with errors about RightSidebar not being QWidget subclass and missing methods

- [ ] **Step 3: Update RightSidebar header file**

```cpp
// include/frames/right_sidebar.h
#ifndef IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_
#define IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_

#include <QWidget>

class QLabel;
class QVBoxLayout;

class RightSidebar : public QWidget {
    Q_OBJECT
public:
    explicit RightSidebar(QWidget* parent = nullptr);

private:
    void buildUi();

    QLabel* placeholder_label_;
    QVBoxLayout* main_layout_;
};

#endif  // IMAGEJ_FRAMES_RIGHT_SIDEBAR_H_
```

- [ ] **Step 4: Run test to verify it fails differently**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make ImageJ_tests -j4 && ./ImageJ_tests --gtest_filter="*RightSidebar*" -v`
Expected: FAIL with "undefined reference to `RightSidebar::RightSidebar(QWidget*)'" (构造函数未实现)

- [ ] **Step 5: Implement RightSidebar constructor and buildUi**

```cpp
// src/frames/right_sidebar.cc
#include "frames/right_sidebar.h"

#include <QLabel>
#include <QVBoxLayout>

RightSidebar::RightSidebar(QWidget* parent)
    : QWidget(parent),
      placeholder_label_(nullptr),
      main_layout_(nullptr) {
    buildUi();
}

void RightSidebar::buildUi() {
    main_layout_ = new QVBoxLayout(this);

    placeholder_label_ = new QLabel("工具选项", this);
    placeholder_label_->setAlignment(Qt::AlignCenter);

    main_layout_->addWidget(placeholder_label_);
    main_layout_->addStretch();  // 添加弹性空间

    setLayout(main_layout_);
}
```

- [ ] **Step 6: Run test to verify it passes**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make ImageJ_tests -j4 && ./ImageJ_tests --gtest_filter="*RightSidebar*" -v`
Expected: PASS

- [ ] **Step 7: Commit**

```bash
cd /home/inmove/nvme1/QtProjects/ImageJ
git add include/frames/right_sidebar.h src/frames/right_sidebar.cc tests/right_sidebar_test.cc
git commit -m "feat: Refactor RightSidebar as QWidget with placeholder text"
```

## Chunk 2: MainFrame类扩展

### Task 2: Update MainFrame header with new members and methods

**Files:**
- Modify: `include/frames/main_frame.h:1-16`

- [ ] **Step 1: Write the failing test for MainFrame interface**

```cpp
// 添加到 tests/main_frame_test.cc (需要先启用)
// 暂时添加到临时测试文件验证
TEST(MainFrameTest, HasRequiredMethods) {
    // 测试MainFrame是否有必要的成员函数
    // 这些会在后续步骤中实现
}
```

- [ ] **Step 2: Update MainFrame header file**

```cpp
// include/frames/main_frame.h
#pragma once
#include <QWidget>

class QSplitter;
class QSettings;
class ImageCanvas;
class RightSidebar;

class MainFrame : public QWidget {
    Q_OBJECT
public:
    explicit MainFrame(QWidget* parent = nullptr);
    ~MainFrame() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildUi();
    void connectSignals();
    void loadWindowSettings();
    void saveWindowSettings();

    QSplitter* splitter_;
    ImageCanvas* image_canvas_;
    RightSidebar* right_sidebar_;
    QSettings* settings_;
};
```

- [ ] **Step 3: Run build to verify header changes compile**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make imagej_core -j4`
Expected: FAIL with "undefined reference to `MainFrame::~MainFrame()'" and other missing implementations

- [ ] **Step 4: Commit**

```bash
cd /home/inmove/nvme1/QtProjects/ImageJ
git add include/frames/main_frame.h
git commit -m "feat: Update MainFrame header with new members and methods"
```

### Task 3: Implement MainFrame constructor and destructor

**Files:**
- Modify: `src/frames/main_frame.cc:1-25`

- [ ] **Step 1: Update MainFrame constructor**

```cpp
// src/frames/main_frame.cc
#include "frames/main_frame.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QSettings>
#include <QCloseEvent>

#include "widgets/image_canvas.h"
#include "frames/right_sidebar.h"

MainFrame::MainFrame(QWidget* parent)
    : QWidget(parent),
      splitter_(nullptr),
      image_canvas_(nullptr),
      right_sidebar_(nullptr),
      settings_(nullptr) {
    // 创建配置对象
    settings_ = new QSettings("ImageJ", "ImageJ", this);

    loadWindowSettings();
    buildUi();
    connectSignals();
}

MainFrame::~MainFrame() {
    // QSettings和子控件由Qt父子关系自动清理
}
```

- [ ] **Step 2: Implement loadWindowSettings (placeholder)**

```cpp
void MainFrame::loadWindowSettings() {
    // 暂时为空实现，后续任务中完善
}
```

- [ ] **Step 3: Implement saveWindowSettings (placeholder)**

```cpp
void MainFrame::saveWindowSettings() {
    // 暂时为空实现，后续任务中完善
}
```

- [ ] **Step 4: Implement closeEvent**

```cpp
void MainFrame::closeEvent(QCloseEvent* event) {
    saveWindowSettings();
    event->accept();
}
```

- [ ] **Step 5: Run build to verify compilation**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make imagej_core -j4`
Expected: PASS (编译成功)

- [ ] **Step 6: Commit**

```bash
cd /home/inmove/nvme1/QtProjects/ImageJ
git add src/frames/main_frame.cc
git commit -m "feat: Implement MainFrame constructor, destructor and event handlers"
```

## Chunk 3: MainFrame UI构建

### Task 4: Implement buildUi method with QSplitter layout

**Files:**
- Modify: `src/frames/main_frame.cc:20-25` (现有buildUi方法)

- [ ] **Step 1: Write failing test for UI structure**

```cpp
// 添加到 tests/main_frame_test.cc
TEST(MainFrameTest, HasSplitterLayout) {
    MainFrame frame;
    frame.show();

    // 验证splitter存在
    QSplitter* splitter = frame.findChild<QSplitter*>();
    ASSERT_NE(splitter, nullptr);

    // 验证splitter有两个子控件
    EXPECT_EQ(splitter->count(), 2);

    // 验证左侧是ImageCanvas
    ImageCanvas* canvas = qobject_cast<ImageCanvas*>(splitter->widget(0));
    EXPECT_NE(canvas, nullptr);

    // 验证右侧是RightSidebar
    RightSidebar* sidebar = qobject_cast<RightSidebar*>(splitter->widget(1));
    EXPECT_NE(sidebar, nullptr);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make ImageJ_tests -j4 && ./ImageJ_tests --gtest_filter="*HasSplitterLayout*" -v`
Expected: FAIL with "splitter is null" or similar

- [ ] **Step 3: Implement buildUi method**

```cpp
void MainFrame::buildUi() {
    // 创建分割器
    splitter_ = new QSplitter(Qt::Horizontal, this);

    // 创建左侧图像画布
    image_canvas_ = new ImageCanvas(splitter_);

    // 创建右侧边栏
    right_sidebar_ = new RightSidebar(splitter_);

    // 添加到分割器
    splitter_->addWidget(image_canvas_);
    splitter_->addWidget(right_sidebar_);

    // 设置初始分割比例 (80% : 20%)
    QList<int> sizes;
    sizes << 800 << 200;  // 基于默认1024宽度计算
    splitter_->setSizes(sizes);

    // 设置主布局
    QHBoxLayout* main_layout = new QHBoxLayout(this);
    main_layout->addWidget(splitter_);
    setLayout(main_layout);

    // 设置窗口标题和默认大小
    setWindowTitle("ImageJ");
    resize(1024, 768);
}
```

- [ ] **Step 4: Update connectSignals (placeholder)**

```cpp
void MainFrame::connectSignals() {
    // 暂时为空，后续添加信号连接
}
```

- [ ] **Step 5: Run test to verify it passes**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make ImageJ_tests -j4 && ./ImageJ_tests --gtest_filter="*HasSplitterLayout*" -v`
Expected: PASS

- [ ] **Step 6: Run application to verify visual appearance**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make ImageJ -j4 && ./ImageJ &`
Expected: 窗口显示，左侧为ImageCanvas（空白），右侧显示"工具选项"

- [ ] **Step 7: Commit**

```bash
cd /home/inmove/nvme1/QtProjects/ImageJ
git add src/frames/main_frame.cc
git commit -m "feat: Implement MainFrame UI with QSplitter layout"
```

## Chunk 4: 配置系统实现

### Task 5: Implement configuration loading and saving

**Files:**
- Modify: `src/frames/main_frame.cc` (loadWindowSettings和saveWindowSettings方法)

- [ ] **Step 1: Write failing test for settings persistence**

```cpp
// 添加到 tests/main_frame_test.cc
TEST(MainFrameTest, SettingsPersistence) {
    // 使用测试专用配置
    QSettings test_settings("ImageJTest", "ImageJTest");
    test_settings.clear();  // 清理测试配置

    {
        // 创建第一个窗口并调整
        MainFrame frame1;
        frame1.resize(800, 600);
        frame1.move(100, 100);

        // 模拟调整splitter
        QSplitter* splitter = frame1.findChild<QSplitter*>();
        ASSERT_NE(splitter, nullptr);
        splitter->setSizes(QList<int>() << 600 << 200);

        // 关闭窗口触发保存
        frame1.close();
    }

    // 验证配置已保存（实际测试需要mock QSettings）
    // 这里主要测试接口可用性
}
```

- [ ] **Step 2: Run test to verify it fails (或至少编译)**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make ImageJ_tests -j4 && ./ImageJ_tests --gtest_filter="*SettingsPersistence*" -v`
Expected: 编译成功，测试运行（具体结果取决于实现）

- [ ] **Step 3: Implement loadWindowSettings method**

```cpp
void MainFrame::loadWindowSettings() {
    // 加载窗口几何信息
    QByteArray geometry_data = settings_->value("Window/geometry").toByteArray();
    if (!geometry_data.isEmpty()) {
        restoreGeometry(geometry_data);
    } else {
        // 首次运行或配置损坏，使用默认值
        resize(1024, 768);
        move(100, 100);  // 默认位置
    }

    // 加载窗口状态（最大化/正常）
    QByteArray window_state = settings_->value("Window/windowState").toByteArray();
    if (!window_state.isEmpty()) {
        restoreState(window_state);
    }

    // splitter状态在buildUi之后加载
}
```

- [ ] **Step 4: Update buildUi to load splitter settings**

```cpp
void MainFrame::buildUi() {
    // ... 现有代码 ...

    // 设置初始分割比例 (80% : 20%)
    QList<int> sizes;
    sizes << 800 << 200;  // 基于默认1024宽度计算
    splitter_->setSizes(sizes);

    // 尝试加载保存的splitter状态
    QByteArray splitter_state = settings_->value("Layout/splitterSizes").toByteArray();
    if (!splitter_state.isEmpty()) {
        splitter_->restoreState(splitter_state);
    }

    // ... 剩余代码 ...
}
```

- [ ] **Step 5: Implement saveWindowSettings method**

```cpp
void MainFrame::saveWindowSettings() {
    // 保存窗口几何信息
    settings_->setValue("Window/geometry", saveGeometry());

    // 保存窗口状态
    settings_->setValue("Window/windowState", saveState());

    // 保存splitter状态
    if (splitter_) {
        settings_->setValue("Layout/splitterSizes", splitter_->saveState());
    }

    // 确保立即写入磁盘
    settings_->sync();
}
```

- [ ] **Step 6: Run build to verify compilation**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make imagej_core -j4`
Expected: PASS

- [ ] **Step 7: Run application and test persistence**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && ./ImageJ`
手动测试：
1. 调整窗口大小和位置
2. 调整splitter分割线
3. 关闭应用
4. 重新启动应用
5. 验证窗口状态恢复

- [ ] **Step 8: Commit**

```bash
cd /home/inmove/nvme1/QtProjects/ImageJ
git add src/frames/main_frame.cc
git commit -m "feat: Implement configuration persistence with QSettings"
```

## Chunk 5: 测试完善

### Task 6: Enable and expand MainFrame tests

**Files:**
- Rename: `tests/main_frame_test.cc.disabled` -> `tests/main_frame_test.cc`
- Modify: `tests/main_frame_test.cc`

- [ ] **Step 1: Rename test file**

```bash
cd /home/inmove/nvme1/QtProjects/ImageJ
mv tests/main_frame_test.cc.disabled tests/main_frame_test.cc
```

- [ ] **Step 2: Update CMakeLists to include test file**

检查 `config/main.cmake` 是否已包含测试文件。通常 `file(GLOB_RECURSE TEST_SOURCES ...)` 会自动包含。

- [ ] **Step 3: Expand test file with comprehensive tests**

```cpp
// tests/main_frame_test.cc
#include <gtest/gtest.h>

#include <QApplication>
#include <QTest>
#include <QTimer>
#include <QSplitter>
#include <QSettings>

#include "frames/main_frame.h"
#include "widgets/image_canvas.h"
#include "frames/right_sidebar.h"

class MainFrameTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每次测试使用新的QApplication
        int argc = 0;
        char* argv[] = {nullptr};
        if (!qApp) {
            app_ = std::make_unique<QApplication>(argc, argv);
        }

        // 使用测试专用配置避免污染用户配置
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, "/tmp/imagej_test");
        frame_ = std::make_unique<MainFrame>();
    }

    void TearDown() override {
        frame_.reset();
    }

    std::unique_ptr<QApplication> app_;
    std::unique_ptr<MainFrame> frame_;
};

TEST_F(MainFrameTest, WindowShowsAndHides) {
    frame_->show();
    QTest::qWait(100);  // 给Qt事件队列处理时间
    EXPECT_TRUE(frame_->isVisible());

    frame_->hide();
    QTest::qWait(100);
    EXPECT_FALSE(frame_->isVisible());
}

TEST_F(MainFrameTest, DefaultWindowSize) {
    frame_->show();
    EXPECT_GE(frame_->width(), 100);
    EXPECT_GE(frame_->height(), 100);

    // 默认应该是1024x768，但可能有窗口装饰
    EXPECT_LE(frame_->width(), 1100);
    EXPECT_LE(frame_->height(), 850);
}

TEST_F(MainFrameTest, WindowTitle) {
    EXPECT_EQ(frame_->windowTitle().toStdString(), "ImageJ");
}

TEST_F(MainFrameTest, SplitterLayout) {
    frame_->show();

    QSplitter* splitter = frame_->findChild<QSplitter*>();
    ASSERT_NE(splitter, nullptr);
    EXPECT_EQ(splitter->count(), 2);

    // 验证子控件类型
    ImageCanvas* canvas = qobject_cast<ImageCanvas*>(splitter->widget(0));
    EXPECT_NE(canvas, nullptr);

    RightSidebar* sidebar = qobject_cast<RightSidebar*>(splitter->widget(1));
    EXPECT_NE(sidebar, nullptr);
}

TEST_F(MainFrameTest, InitialSplitRatio) {
    frame_->show();

    QSplitter* splitter = frame_->findChild<QSplitter*>();
    ASSERT_NE(splitter, nullptr);

    QList<int> sizes = splitter->sizes();
    ASSERT_EQ(sizes.size(), 2);

    // 验证大致比例 (80%/20%)
    int total = sizes[0] + sizes[1];
    float left_ratio = static_cast<float>(sizes[0]) / total;
    EXPECT_NEAR(left_ratio, 0.8f, 0.1f);  // 允许10%误差
}

// 注意：配置持久化测试可能需要mock QSettings或使用测试专用目录
```

- [ ] **Step 4: Run all tests to verify they pass**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make ImageJ_tests -j4 && ./ImageJ_tests --gtest_filter="*MainFrame*" -v`
Expected: All tests PASS

- [ ] **Step 5: Run full test suite**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && ./ImageJ_tests`
Expected: All tests PASS (包含之前的核心类测试)

- [ ] **Step 6: Commit**

```bash
cd /home/inmove/nvme1/QtProjects/ImageJ
git add tests/main_frame_test.cc
git rm tests/main_frame_test.cc.disabled
git commit -m "feat: Enable and expand MainFrame unit tests"
```

## Chunk 6: 错误处理与优化

### Task 7: Add error handling and robustness

**Files:**
- Modify: `src/frames/main_frame.cc` (错误处理)

- [ ] **Step 1: Add error handling to loadWindowSettings**

```cpp
void MainFrame::loadWindowSettings() {
    try {
        // 加载窗口几何信息
        QByteArray geometry_data = settings_->value("Window/geometry").toByteArray();
        if (!geometry_data.isEmpty()) {
            if (!restoreGeometry(geometry_data)) {
                qWarning() << "Failed to restore window geometry, using defaults";
                setDefaultGeometry();
            }
        } else {
            setDefaultGeometry();
        }

        // 加载窗口状态
        QByteArray window_state = settings_->value("Window/windowState").toByteArray();
        if (!window_state.isEmpty()) {
            if (!restoreState(window_state)) {
                qWarning() << "Failed to restore window state";
            }
        }
    } catch (const std::exception& e) {
        qCritical() << "Error loading window settings:" << e.what();
        setDefaultGeometry();
    }
}

void MainFrame::setDefaultGeometry() {
    resize(1024, 768);
    move(100, 100);
}
```

- [ ] **Step 2: Add error handling to buildUi**

```cpp
void MainFrame::buildUi() {
    try {
        // 创建分割器
        splitter_ = new QSplitter(Qt::Horizontal, this);
        if (!splitter_) {
            throw std::runtime_error("Failed to create QSplitter");
        }

        // 创建左侧图像画布
        image_canvas_ = new ImageCanvas(splitter_);
        if (!image_canvas_) {
            throw std::runtime_error("Failed to create ImageCanvas");
        }

        // 创建右侧边栏
        right_sidebar_ = new RightSidebar(splitter_);
        if (!right_sidebar_) {
            throw std::runtime_error("Failed to create RightSidebar");
        }

        // ... 剩余代码 ...

    } catch (const std::exception& e) {
        qCritical() << "Failed to build UI:" << e.what();
        // 创建最简单的后备布局
        QLabel* error_label = new QLabel("Failed to initialize application UI", this);
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->addWidget(error_label);
        setLayout(layout);
    }
}
```

- [ ] **Step 3: Add configuration validation**

```cpp
bool MainFrame::validateSettings() {
    // 检查必要的配置键是否存在且有效
    if (!settings_->contains("Window/geometry") &&
        !settings_->contains("Layout/splitterSizes")) {
        qDebug() << "No saved settings found, using defaults";
        return false;
    }
    return true;
}
```

- [ ] **Step 4: Run build to verify compilation**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && make imagej_core -j4`
Expected: PASS

- [ ] **Step 5: Run tests to verify error handling doesn't break existing functionality**

Run: `cd /home/inmove/nvme1/QtProjects/ImageJ/build && ./ImageJ_tests --gtest_filter="*MainFrame*" -v`
Expected: All tests still PASS

- [ ] **Step 6: Commit**

```bash
cd /home/inmove/nvme1/QtProjects/ImageJ
git add src/frames/main_frame.cc
git commit -m "feat: Add error handling and robustness to MainFrame"
```

## 验收测试

### 手动验收测试清单

- [ ] **窗口显示**: 应用启动显示窗口，标题为"ImageJ"
- [ ] **默认尺寸**: 初始窗口大小为1024×768（近似值）
- [ ] **布局结构**: 左侧为ImageCanvas（空白），右侧为RightSidebar显示"工具选项"
- [ ] **分割器功能**: 可以拖拽分割线调整左右区域宽度
- [ ] **配置持久化**:
  - 调整窗口大小和位置，关闭应用，重新打开验证恢复
  - 调整splitter分割线，关闭应用，重新打开验证恢复
- [ ] **错误恢复**: 删除配置文件（~/.config/ImageJ/ImageJ.conf），验证应用使用默认值启动
- [ ] **测试通过**: 运行`./ImageJ_tests`所有测试通过

### 自动化验收测试

```bash
# 1. 构建项目
cd /home/inmove/nvme1/QtProjects/ImageJ/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4

# 2. 运行所有测试
./ImageJ_tests

# 3. 运行应用进行基本验证
timeout 5s ./ImageJ  # 运行5秒后自动关闭

# 4. 检查配置文件创建
ls -la ~/.config/ImageJ/ImageJ.conf
```

## 已知问题与后续改进

### 已知限制
1. **多显示器支持有限**: 窗口位置恢复在显示器配置变更时可能不准确
2. **配置版本控制**: 不支持配置格式迁移，格式变更可能导致配置丢失
3. **RightSidebar功能简单**: 当前仅显示占位文本，需要后续任务完善

### 后续优化建议
1. **配置迁移**: 添加配置版本号和迁移逻辑
2. **布局预设**: 支持多种布局预设（如全画布模式、双栏模式等）
3. **状态恢复验证**: 添加更严格的配置有效性验证
4. **性能监控**: 添加UI构建和配置加载的性能日志

---
*计划版本: 1.0*
*创建日期: 2026-03-28*
*更新日期: 2026-03-28*
*计划者: Claude Code*

*注意: 此计划遵循TDD原则，每个任务先写测试后实现。使用superpowers:subagent-driven-development执行此计划。*