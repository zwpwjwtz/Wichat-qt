#include "loginwindow.h"
#include <QApplication>
#include <QTextCodec>


int main(int argc, char *argv[])
{
    // Enable scaling for HiDPI device
    if (qgetenv("QT_SCALE_FACTOR").isEmpty() &&
        qgetenv("QT_SCREEN_SCALE_FACTORS").isEmpty())
    {
        qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    // Set proper codec for strings in Qt 4
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
#endif

    QApplication a(argc, argv);
    LoginWindow w;
    w.show();

    return a.exec();
}
