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


    MainWindow window;
    window.show();
    return app.exec();
}
