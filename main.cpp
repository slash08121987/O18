#include "o18mainwnd.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    O18MainWnd w;
    bool fs = true;
    for(int i = 1; i != argc; ++i)
    {
            QString arg = QString::fromLocal8Bit(argv[i]).toLower();
            if( arg == "--no-fullscreen" )
                    fs = false;
    }
    if( fs )
            //w.showFullScreen();
            w.show();
    else
            w.show();

    return a.exec();
}
