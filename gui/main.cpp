#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QStringList>
#include <QImageReader>
#include <QLibraryInfo>
#include <QDir>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile styleFile(":/styles.css");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    } else {
        qCritical() << "Не удалось загрузить :/styles.css";
    }



    MainWindow window;
    window.show();
    return app.exec();
}
