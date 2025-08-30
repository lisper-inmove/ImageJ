#include <gtest/gtest.h>

// 如果你的测试会用到 QWidget / MainFrame，就需要 QApplication
#include <QApplication>
#include <QTest>

// 自定义 gtest 的 main，把 QApplication 建起来再 RUN_ALL_TESTS
int main(int argc, char** argv) {
    // 如需无头环境（CI），可以在外部设置 QT_QPA_PLATFORM=offscreen
    QApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    // 注意：不要调用 app.exec()，单元测试不需要进入事件循环
    return RUN_ALL_TESTS();
}
