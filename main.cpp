#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <spdlog/spdlog.h>

#include "frames/MainFrame.h"
#include "logger/logger.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ImageJ_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    setSpdlog();

    LOG_INFO("ImageJ started...");
    MainFrame f;
    f.show();

    return a.exec();
}
