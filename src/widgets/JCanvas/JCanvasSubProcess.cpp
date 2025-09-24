#include "widgets/JCanvas.h"
#include <QResizeEvent>
#include <QFileDialog>
#include <QProcess>

void JCanvas::subProcess() {
    QPointF start = toImageCoord(startPos_);
    QPointF end = toImageCoord(endPos_);
    QImage selectedImage = img_.copy(QRect(start.toPoint(), end.toPoint()));

    QString tempFile = QDir::temp().filePath("selected_image.png");
    selectedImage.save(tempFile);

    // 使用 QProcess 启动一个新的应用实例显示选定的图像
    QProcess* process = new QProcess(this);

    // 假设你希望再次启动该应用程序的另一个实例
    QString program = QCoreApplication::applicationFilePath();  // 获取当前应用程序的路径
    QStringList arguments;
    arguments << tempFile;  // 传递选定部分的文件路径

    process->start(program, arguments);

    // QFile::remove(tempFile);
}
