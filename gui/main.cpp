#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QStringList>
#include <QImageReader>
#include <QSvgRenderer>
#include <QLibraryInfo>
#include <QDir>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    // Проверяем, что CMake действительно встроил все ресурсы в executable.
    const QStringList requiredResources = {
        QStringLiteral(":/styles.css"),
        QStringLiteral(":/up.svg"),
        QStringLiteral(":/down.svg")
    };

    for (const QString &resourcePath : requiredResources) {
        if (!QFile::exists(resourcePath)) {
            qCritical().noquote() << "Не найден Qt resource:" << resourcePath;
        }
    }

    QFile styleFile(QStringLiteral(":/styles.css"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    } else {
        qCritical() << "Не удалось загрузить :/styles.css";
    }



    MainWindow window;
    window.show();
    return app.exec();
}
