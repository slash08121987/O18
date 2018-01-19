#include "o18mainwnd.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //QStringList paths = QCoreApplication::libraryPaths();
    //paths.append(".");
    //paths.append("imageformats");
    //paths.append("platforms");
    //paths.append("sqldrivers");
    //QCoreApplication::setLibraryPaths(paths);

    QApplication a(argc, argv);
    O18MainWnd w;
    bool fs = false;
    for(int i = 1; i != argc; ++i)
    {
        QString arg = QString::fromLocal8Bit(argv[i]).toLower();
        if( arg == "--fullscreen" )
                fs = true;
    }
    if( fs )
    {
        w.showFullScreen();
    }
    else
    {
        w.setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
        w.setFixedSize(QSize(1024, 800));
        w.show();
    }

    return a.exec();

}
