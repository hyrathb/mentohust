#ifndef MENTO_H
#define MENTO_H

#define SETTINGS_PATH "/Contents/Resources/mento.conf"

#include <QObject>
#include <QMessageBox>
#include <QDateTime>
#include <QTextBrowser>



#include "configview.h"
#include "StatusIcon.h"
#include "menu.h"
#include "menuitem.h"
#include "menuviewitem.h"
#include "cocoaappinitlizer.h"
#include "mentothread.h"
#include "guite.h"

#define SERVICE_NAME "com.mycompany.cocoamento"

class Mento : public QObject
{
    Q_OBJECT
public:
    explicit Mento(QObject *parent = 0);
    bool initSuccess();
    ~Mento();

public slots:
    void quitApp();
    void toggleConnect();
    void outputHandle(QString str);
    void exitHandle();
    void statusHandle(QString str,int status);
    void errorHandle(QString detail);
    void authMento();
    void noconfigHandle();
    void showAboutQt();
    void notifyHandle(QString str);
private:

    bool initsuccess;


    StatusIcon* icon;

    Menu* main;
    Menu* sub;

    MenuViewItem* configview;

    MenuItem* status;
    MenuItem* connectHandle;
    MenuItem* settings;
    MenuItem* resetsettings;
    MenuItem* logs;
    MenuItem* about;
    MenuItem* aboutQt;
    MenuItem* help;
    MenuItem* exit;

    MenuViewItem* cvcontainer;

    ConfigView* cv;
    MentoThread * mainthread;
    Guite* gt;

    QTextBrowser * log;


};

#endif // MENTO_H
