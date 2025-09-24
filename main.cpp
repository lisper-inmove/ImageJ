#include "frames/MainFrame.h"
#include "utils/logger.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    setSpdlog();
    QStringList arguments = QCoreApplication::arguments();
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ImageJ_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    LOG_INFO("ImageJ Version 0.0.4 started...");
    // QString path = "C:\\Users\\Administrator\\Desktop\\Desktop.jpg";
    QString path = "";
    if (arguments.size() > 1) {
        LOG_INFO("Got image: {}", arguments[1].toStdString());
        path = arguments[1];
    } else {

    }
    MainFrame w;
    w.build(path);
    w.show();
    return a.exec();
}
