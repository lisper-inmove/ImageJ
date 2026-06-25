#pragma once
#include <QWidget>

#include <QVector>
#include <QSize>

#include "core/image_data.h"

class QLabel;
class QSplitter;
class QSettings;
class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QMenu;
class QMenuBar;
class QStatusBar;
class ImageCanvas;
class ImageDocument;
class RightSidebar;

/**
 * @brief 应用程序主窗口，管理布局、菜单、信号连接和剪切历史。
 *
 * MainFrame 是应用的中枢控制器，负责：
 * - 构建 UI 布局（菜单栏 → 分割器（ImageCanvas | RightSidebar）→ 状态栏）
 * - 拖放加载图像文件（支持 RAW 和标准格式）
 * - 窗口几何信息的持久化（QSettings）
 * - 信号中转：将 RightSidebar 的用户操作转发到 ImageCanvas，反之亦然
 * - 色彩空间转换和通道增益应用
 * - 剪切操作历史和"还原原始"功能
 *
 * 数据流：
 * @dot
 * digraph data_flow {
 *   rankdir=LR;
 *   RightSidebar -> MainFrame [label="信号"];
 *   MainFrame -> ImageCanvas [label="调用方法"];
 *   ImageCanvas -> RightSidebar [label="信号"];
 * }
 * @enddot
 *
 * 剪切历史设计：
 * - `cut_history_` 存储每次剪切后的完整图像快照（不覆盖原始图像）
 * - `original_image_data_` 保存最初始的图像（"还原原始"按钮使用）
 * - `original_data_` 是色彩空间转换的基准，每次切换历史/剪切后更新
 */
class MainFrame : public QWidget {
    Q_OBJECT
public:
    explicit MainFrame(QWidget* parent = nullptr);
    ~MainFrame() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    /// 色彩空间下拉框选择变化处理
    void onColorSpaceChanged(int index);
    /// 通道增益滑块变化处理
    void onChannelGainsChanged(const QVector<int>& gains);
    /// 右键菜单"另存为"→ 弹出保存文件对话框
    void onSaveSelection();
    /**
     * @brief 执行剪切操作：提取选区、替换文档图像、记录快照到历史列表。
     *
     * 操作流程：
     * 1. 记录剪切前尺寸（before_size）
     * 2. 调用 extractSelection() 提取选区像素
     * 3. 设置裁剪后的图像到文档
     * 4. 创建 CutHistoryEntry 并追加到 cut_history_
     * 5. 重置色彩空间状态并通知侧边栏添加历史条目
     */
    void onCutSelection();
    /// 侧边栏微调框改变 → 调用 ImageCanvas::resize_selection()
    void onSelectionSizeChanged(int width, int height);
    /**
     * @brief 从剪切历史恢复图像快照。
     * @param index 历史索引（0-based），-1 表示恢复原始图像
     */
    void onHistoryItemSelected(int index);

private:
    void buildUi();
    void setupMenuBar();
    void setupStatusBar();
    void connectSignals();
    void loadDroppedFile(const QString& file_path);
    void setupLoadedDocument(ImageDocument* doc, const QString& file_path);
    void updateStatusBarPixelInfo(const QPoint& image_pos);
    void loadWindowSettings();
    void saveWindowSettings();
    void setDefaultGeometry();
    bool validateSettings();
    void applyChannelGains(ImageData& data);

    QSplitter* splitter_;         ///< 水平分割器（ImageCanvas : RightSidebar = 19:1）
    ImageCanvas* image_canvas_;   ///< 图像显示画布
    RightSidebar* right_sidebar_; ///< 右侧边栏
    QSettings* settings_;         ///< 窗口配置持久化
    QMenuBar* menu_bar_;          ///< 菜单栏
    QStatusBar* status_bar_;      ///< 状态栏
    QLabel* pixel_info_label_;    ///< 状态栏像素信息（"就绪 (x,y) R: G: B:"）
    ImageData original_data_;     ///< 色彩空间转换的基准数据（每次剪切/切换历史后更新）
    int current_colorspace_;      ///< 当前色彩空间索引
    QVector<int> channel_gains_;  ///< 通道增益值缓存

    /**
     * @brief 剪切历史条目，记录每次剪切后的完整图像快照和尺寸变化。
     *
     * 每次执行剪切时：
     * - index 从 1 开始自增
     * - image_data 保存剪切后的完整图像（用于点击恢复）
     * - before_size / after_size 用于列表显示文本
     */
    struct CutHistoryEntry {
        int index;              ///< 显示序号，1-based
        ImageData image_data;   ///< 该次剪切后的完整图像快照
        QSize before_size;      ///< 剪切前的图像尺寸
        QSize after_size;       ///< 剪切后的图像尺寸
    };
    QVector<CutHistoryEntry> cut_history_;  ///< 剪切历史记录（按时间顺序）
    ImageData original_image_data_;          ///< 首次加载时的原始图像数据，永不覆盖
    QSize original_size_;                    ///< 首次加载时的图像尺寸
};

/**
 * @brief 从源图像数据中提取矩形区域的像素。
 * @param src 源图像数据
 * @param rect 要提取的矩形区域（图像坐标）
 * @return 提取出的新 ImageData，尺寸为 rect.size()，像素格式与 src 相同
 *
 * 使用 memcpy 逐行复制每个通道的像素数据。
 * 注意：调用者应确保 rect 在 src 边界内，本函数不做边界检查。
 */
ImageData extractSelection(const ImageData& src, const QRect& rect);

