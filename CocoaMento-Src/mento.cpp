#include "mento.h"



Mento::Mento(QObject *parent) :
    QObject(parent)
{
    if(CocoaInitialize::DefaultApp()->checkRunning())
    {
        initsuccess=false;
        QMessageBox::warning(0,"Warning","Cocoa Mento 正在运行当中！",QMessageBox::Ok,QMessageBox::Ok);
        return;
    }
    else initsuccess=true;

    log=new QTextBrowser();
    log->setWindowModality(Qt::WindowModal);
    log->setWindowFlags(Qt::WindowStaysOnTopHint|Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);
    log->setGeometry(200,200,400,500);
    log->setWindowTitle(tr("认证日志"));

    gt=new Guite();

    main=new Menu("Cocoa Mento");
    sub=new Menu("Settings");

    status=new MenuItem("[状态:正常] 未连接");
    connectHandle=new MenuItem("开始认证");
    settings=new MenuItem("偏好设置");
    about=new MenuItem("关于CocoaMento");
    aboutQt=new MenuItem("关于Qt");
    help=new MenuItem("使用说明");
    exit=new MenuItem("退出");
    resetsettings=new MenuItem("重置偏好设置");
    logs=new MenuItem("显示日志");

    cv=new ConfigView();
    cvcontainer=new MenuViewItem("Settings",cv);

    cv->setPalette(Qt::white);
    cv->show();

    settings->setSubMenu(sub);

    main->addMenuItem(status);
    main->addMenuSeparator();
    main->addMenuItem(connectHandle);
    main->addMenuItem(settings);
    main->addMenuItem(resetsettings);
    main->addMenuItem(logs);
    main->addMenuSeparator();
    main->addMenuItem(help);
    main->addMenuItem(about);
    main->addMenuItem(aboutQt);
    main->addMenuSeparator();
    main->addMenuItem(exit);

    sub->addMenuItem(cvcontainer);

    icon=new StatusIcon();
    icon->setMenu(main);
    icon->showIcon();

    mainthread=new MentoThread(cv->Config());

    connect((QObject*)exit->port,SIGNAL(trigger()),this,SLOT(quitApp()));
    connect((QObject*)resetsettings->port,SIGNAL(trigger()),cv,SLOT(resetConfig()));
    connect((QObject*)connectHandle->port,SIGNAL(trigger()),this,SLOT(toggleConnect()));
    connect((QObject*)logs->port,SIGNAL(trigger()),log,SLOT(show()));
    connect((QObject*)about->port,SIGNAL(trigger()),gt,SLOT(showAbout()));
    connect((QObject*)aboutQt->port,SIGNAL(trigger()),this,SLOT(showAboutQt()));
    connect((QObject*)help->port,SIGNAL(trigger()),gt,SLOT(showHelp()));


    connect(mainthread,SIGNAL(output(QString)),this,SLOT(outputHandle(QString)));
    connect(mainthread,SIGNAL(exitSuccess()),this,SLOT(exitHandle()));
    connect(mainthread,SIGNAL(statusChanged(QString,int)),this,SLOT(statusHandle(QString,int)));
    connect(mainthread,SIGNAL(error(QString)),this,SLOT(errorHandle(QString)));
    connect(mainthread,SIGNAL(authError()),this,SLOT(authMento()));
    connect(mainthread,SIGNAL(noConfig()),this,SLOT(noconfigHandle()));
    connect(mainthread,SIGNAL(notify(QString)),this,SLOT(notifyHandle(QString)));

    if(cv->autoConnect()) toggleConnect();
}

void Mento::noconfigHandle()
{
    QMessageBox::critical(0,tr("配置信息不全！"),tr("用户名密码、认证使用网卡等信息未设置，连接无法继续。\n请仔细阅读使用说明，对认证信息进行正确配置！"),QMessageBox::Ok,QMessageBox::NoButton);
    gt->showHelp();
}

void Mento::exitHandle()
{
    CocoaInitialize::growl("CocoaMento","链接已断开");
    status->setTitle("[状态:正常] 连接已断开");
    connectHandle->setTitle("开始认证");
    icon->setIcon("normal.png");
}

void Mento::notifyHandle(QString str)
{
    CocoaInitialize::growl("CocoaMento",str.toUtf8().data());
}

void Mento::statusHandle(QString str, int status)
{
    if(status==0||status==2){
        this->status->setTitle(tr("[状态:正常] %1").arg(str).toUtf8().data());
        if(cv->growlStatus()) CocoaInitialize::growl("CocoaMento",str.toUtf8().data());
        icon->setIcon((status==2)?"normal.png":"connecting.png");
    }
    else
    {
        this->status->setTitle(tr("[状态:警告] %1").arg(str).toUtf8().data());
        icon->setIcon("warning.png");
    }
}

void Mento::errorHandle(QString detail)
{
    mainthread->terminate();
    icon->setIcon("critical.png");
    QMessageBox::critical(0,tr("错误！"),tr("Mentohust 遇到严重错误，已经终止认证!\n\n错误细节: %1").arg(detail),QMessageBox::Ok,QMessageBox::Ok);
}

void Mento::toggleConnect()
{
    if(mainthread->isRunning())
    {
        mainthread->terminate();

    }
    else
    {
        status->setTitle("[状态:正常] 正在链接...");
        connectHandle->setTitle("退出认证");
        log->setTextColor(Qt::red);
        log->setFontItalic(true);
        log->append("\n\n");
        log->append(QDateTime::currentDateTime().toString());

        log->setTextColor(Qt::black);
        mainthread->start();

    }

}

void Mento::outputHandle(QString str)
{
    log->append(str);

    qDebug()<<str;
}

void Mento::quitApp()
{
    if(mainthread->isRunning())
    {

        if(QMessageBox::Ok==QMessageBox::warning(
                0,
                tr("警告"),tr("Mentohust 正在运行，要退出认证么？"),
                QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok))
             mainthread->terminate();
        else return;
    }
    cv->close();
    log->close();
    gt->close();
}
bool Mento::initSuccess()
{
    return initsuccess;
}

void Mento::authMento()
{

    icon->setIcon("warning.png");
    mainthread->terminate();
    if(CocoaInitialize::auth()==1)mainthread->start();
    else
    {
        icon->setIcon("critical.png");
        QMessageBox::warning(0,tr("获取权限失败"),tr("获取权限失败，认证无法继续，请正确授予 Cocoa Mento 系统权限！"));
        this->connectHandle->setTitle("开始连接");
        icon->setIcon("normal.png");
    }
}
void Mento::showAboutQt()
{
    QMessageBox::aboutQt(0);
}

Mento::~Mento()
{
    if(initsuccess)
    {
        cv->deleteLater();
        mainthread->deleteLater();
        gt->deleteLater();
        delete logs;
        delete main;
        delete sub;
        delete status;
        delete settings;
        delete about;
        delete aboutQt;
        delete help;
        delete exit;
        delete cvcontainer;
        delete icon;
        delete resetsettings;
        delete connectHandle;

    }
}



