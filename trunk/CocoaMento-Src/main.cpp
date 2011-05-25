#include <QtGui/QApplication>
#include "mento.h"
#include "cocoaappinitlizer.h"
#include <QTextCodec>






int main(int argc, char *argv[])
{
    CocoaInitialize::initCocoaApp();
    QApplication a(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));
    Mento m;
    if(m.initSuccess())
          return a.exec();
    a.exit();

    return CocoaInitialize::exitCocoaApp();;
}
