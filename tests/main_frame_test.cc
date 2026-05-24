#include <gtest/gtest.h>
#include <QAction>
#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QSplitter>
#include <QStyle>
#include <QTest>
#include <QTimer>
#include <QToolBar>
#include <QLabel>
#include <QStatusBar>

#include "frames/main_frame.h"
#include "widgets/image_canvas.h"
#include "frames/right_sidebar.h"

class MainFrameTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 0;
        char* argv[] = {nullptr};
        app_ = std::make_unique<QApplication>(argc, argv);
        frame_ = std::make_unique<MainFrame>();
    }

    void TearDown() override {
        frame_.reset();
        app_.reset();
    }

    std::unique_ptr<QApplication> app_;
    std::unique_ptr<MainFrame> frame_;
};

// Comprehensive test class from the implementation plan
class MainFrameTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // Set up a test-specific QSettings path to avoid polluting user config
        // This should be done once before any tests run
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                          QDir::tempPath() + "/ImageJTest");
    }

    void SetUp() override {
        // Only create QApplication if it doesn't exist
        static int argc = 1;
        static char* argv[] = {const_cast<char*>("test")};
        if (!QApplication::instance()) {
            app_ = std::make_unique<QApplication>(argc, argv);
        }
    }

    void TearDown() override {
        // Don't destroy QApplication here - let it be destroyed at end of all tests
    }

    static void TearDownTestSuite() {
        // Clean up test settings after all tests
        QSettings test_settings(QSettings::IniFormat, QSettings::UserScope,
                               "ImageJTest", "ImageJTest");
        test_settings.clear();
        test_settings.sync();
    }

    std::unique_ptr<QApplication> app_;
};

// Tests from the implementation plan
TEST_F(MainFrameTest, WindowShowsAndHides) {
    MainFrame frame;
    frame.show();
    QTest::qWait(100);  // Give Qt time to process the show event
    EXPECT_TRUE(frame.isVisible());

    frame.hide();
    QTest::qWait(100);
    EXPECT_FALSE(frame.isVisible());
}

TEST_F(MainFrameTest, DefaultWindowSize) {
    MainFrame frame;
    frame.show();
    QTest::qWait(100);

    // Check that window has reasonable default size
    QSize size = frame.size();
    EXPECT_GT(size.width(), 0);
    EXPECT_GT(size.height(), 0);

    // Default should be at least 400x300 (reasonable minimum)
    EXPECT_GE(size.width(), 400);
    EXPECT_GE(size.height(), 300);
}

TEST_F(MainFrameTest, WindowTitle) {
    MainFrame frame;
    frame.show();
    QTest::qWait(100);

    // Window should have a title
    QString title = frame.windowTitle();
    EXPECT_FALSE(title.isEmpty());

    // Title should contain "ImageJ" or be something meaningful
    EXPECT_TRUE(title.contains("ImageJ", Qt::CaseInsensitive) ||
                title.contains("Image", Qt::CaseInsensitive));
}

TEST_F(MainFrameTest, SplitterLayout) {
    MainFrame frame;
    frame.show();
    QTest::qWait(100);

    // Find the splitter
    QSplitter* splitter = frame.findChild<QSplitter*>();
    ASSERT_NE(splitter, nullptr);

    // Splitter should have exactly 2 widgets
    EXPECT_EQ(splitter->count(), 2);

    // First widget should be ImageCanvas
    ImageCanvas* canvas = qobject_cast<ImageCanvas*>(splitter->widget(0));
    EXPECT_NE(canvas, nullptr);

    // Second widget should be RightSidebar
    RightSidebar* sidebar = qobject_cast<RightSidebar*>(splitter->widget(1));
    EXPECT_NE(sidebar, nullptr);

    // Splitter should be horizontal
    EXPECT_EQ(splitter->orientation(), Qt::Horizontal);
}

TEST_F(MainFrameTest, InitialSplitRatio) {
    MainFrame frame;
    frame.show();
    QTest::qWait(100);

    QSplitter* splitter = frame.findChild<QSplitter*>();
    ASSERT_NE(splitter, nullptr);

    // Get initial sizes
    QList<int> sizes = splitter->sizes();
    ASSERT_EQ(sizes.size(), 2);

    // Both panes should have non-zero size
    EXPECT_GT(sizes[0], 0);
    EXPECT_GT(sizes[1], 0);

    // Canvas (left) should be larger than sidebar (right)
    // Typically canvas takes 70-80% of width
    EXPECT_GT(sizes[0], sizes[1]);

    // Calculate ratio
    double ratio = static_cast<double>(sizes[0]) / (sizes[0] + sizes[1]);
    EXPECT_GT(ratio, 0.5);  // Canvas should be >50%
    EXPECT_LT(ratio, 0.9);  // But not too large (sidebar needs some space)
}

// Existing tests
TEST_F(MainFrameTestFixture, HasRequiredMethods) {
    // Test that MainFrame can be instantiated
    // This will fail to link until all methods are implemented
    EXPECT_NE(frame_.get(), nullptr);
}

TEST_F(MainFrameTestFixture, HasSplitterLayout) {
    frame_->show();

    // 验证splitter存在
    QSplitter* splitter = frame_->findChild<QSplitter*>();
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

TEST(SettingsPersistenceTest, MainFrameSettings) {
    // 需要QApplication实例
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);

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

// === Toolbar tests ===

class ToolBarTest : public ::testing::Test {
 protected:
  void SetUp() override {
    static int argc = 1;
    static char* argv[] = {const_cast<char*>("test")};
    if (!QApplication::instance()) {
      app_ = std::make_unique<QApplication>(argc, argv);
    }
  }

  std::unique_ptr<QApplication> app_;
};

TEST_F(ToolBarTest, ToolBarExists) {
  MainFrame frame;
  frame.show();
  QTest::qWait(50);

  QToolBar* tool_bar = frame.findChild<QToolBar*>();
  EXPECT_NE(tool_bar, nullptr);
}

TEST_F(ToolBarTest, ToolBarHasFiveActions) {
  MainFrame frame;
  frame.show();
  QTest::qWait(50);

  QToolBar* tool_bar = frame.findChild<QToolBar*>();
  ASSERT_NE(tool_bar, nullptr);

  QList<QAction*> actions = tool_bar->actions();
  EXPECT_GE(actions.size(), 5);
}

TEST_F(ToolBarTest, ToolBarActionTexts) {
  MainFrame frame;
  frame.show();
  QTest::qWait(50);

  QToolBar* tool_bar = frame.findChild<QToolBar*>();
  ASSERT_NE(tool_bar, nullptr);

  QStringList action_texts;
  for (QAction* action : tool_bar->actions()) {
    action_texts << action->text();
  }

  EXPECT_TRUE(action_texts.contains("打开"));
  EXPECT_TRUE(action_texts.contains("保存"));
  EXPECT_TRUE(action_texts.contains("适应窗口"));
  EXPECT_TRUE(action_texts.contains("放大"));
  EXPECT_TRUE(action_texts.contains("缩小"));
}

TEST_F(ToolBarTest, ToolBarActionsHaveToolTips) {
  MainFrame frame;
  frame.show();
  QTest::qWait(50);

  QToolBar* tool_bar = frame.findChild<QToolBar*>();
  ASSERT_NE(tool_bar, nullptr);

  for (QAction* action : tool_bar->actions()) {
    if (action->isSeparator()) continue;
    EXPECT_FALSE(action->toolTip().isEmpty())
        << "Action '" << action->text().toStdString() << "' has no tooltip";
  }
}

TEST_F(ToolBarTest, ToolBarActionsHaveIcon) {
  MainFrame frame;
  frame.show();
  QTest::qWait(50);

  QToolBar* tool_bar = frame.findChild<QToolBar*>();
  ASSERT_NE(tool_bar, nullptr);

  for (QAction* action : tool_bar->actions()) {
    if (action->isSeparator()) continue;
    EXPECT_FALSE(action->icon().isNull())
        << "Action '" << action->text().toStdString() << "' has no icon";
  }
}

// === Status bar tests ===

class StatusBarTest : public ::testing::Test {
 protected:
  void SetUp() override {
    static int argc = 1;
    static char* argv[] = {const_cast<char*>("test")};
    if (!QApplication::instance()) {
      app_ = std::make_unique<QApplication>(argc, argv);
    }
  }

  std::unique_ptr<QApplication> app_;
};

TEST_F(StatusBarTest, StatusBarExists) {
  MainFrame frame;
  frame.show();
  QTest::qWait(50);

  QStatusBar* status_bar = frame.findChild<QStatusBar*>();
  EXPECT_NE(status_bar, nullptr);
}

TEST_F(StatusBarTest, DefaultMessageIsReady) {
  MainFrame frame;
  frame.show();
  QTest::qWait(50);

  QStatusBar* status_bar = frame.findChild<QStatusBar*>();
  ASSERT_NE(status_bar, nullptr);

  // "就绪" is a normal widget (QLabel), not a temporary message
  bool found = false;
  for (QLabel* label : status_bar->findChildren<QLabel*>()) {
    if (label->text() == "就绪") {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(StatusBarTest, HasPermanentImageInfoLabel) {
  MainFrame frame;
  frame.show();
  QTest::qWait(50);

  QStatusBar* status_bar = frame.findChild<QStatusBar*>();
  ASSERT_NE(status_bar, nullptr);

  bool found = false;
  for (QLabel* label : status_bar->findChildren<QLabel*>()) {
    if (label->text() == "图像信息") {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(StatusBarTest, ShowTemporaryMessage) {
  MainFrame frame;
  frame.show();
  QTest::qWait(50);

  QStatusBar* status_bar = frame.findChild<QStatusBar*>();
  ASSERT_NE(status_bar, nullptr);

  status_bar->showMessage("测试消息", 5000);
  QTest::qWait(50);

  EXPECT_EQ(status_bar->currentMessage().toStdString(), "测试消息");

  status_bar->clearMessage();
  QTest::qWait(50);

  // After clearing, currentMessage is empty; left-side "就绪" QLabel persists
  EXPECT_TRUE(status_bar->currentMessage().isEmpty());
}