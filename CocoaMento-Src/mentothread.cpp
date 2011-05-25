#include "mentothread.h"
#include <cocoaappinitlizer.h>
#include <QDebug>

QProcess* pr;

MentoThread::MentoThread(QSettings* settings) :
    QThread(0)
{
    this->settings=settings;
    connect(this,SIGNAL(terminated()),this,SLOT(terminatedHandle()));
    //connect(this,SIGNAL(error(QString)),this,SLOT(terminate()));
    //pr->moveToThread(this);
}

void MentoThread::processFinished(int status, QProcess::ExitStatus qstatus)
{
    if(qstatus==QProcess::CrashExit) emit processCrashed();
    else if(status!=0) emit error(tr("%d").setNum(status));
    else emit exitSuccess();
}


void MentoThread::run()
{
    pr=new QProcess();
    pr->setWorkingDirectory(tr("%1/Contents/MacOS/").arg(tr(CocoaInitialize::mainBundlePath())));
    pr->start(tr("./mentohust"));
    if(!pr->waitForStarted(3000))
    {
        emit startFailure();
        delete pr;
        return;
    }
    while(pr->state()==QProcess::Running)
    {
        if(pr->waitForReadyRead(100))
        {
            while(pr->canReadLine())
            {
                QString out(pr->readLine());
                emit output(out);
                if(out.at(0)=='$') emit notify(out.mid(3));
                else if(out.at(0)=='>')
                {
                    emit statusChanged(out.mid(3),(out.at(1)=='<')?2:0);
                    if(out.at(1)=='!') emit exitSuccess();
                }
                else if(out.at(0)=='!')
                {

                    if(settings->value(tr("AllowError")).toBool())
                    {
                        emit statusChanged(out.mid(3),1);
                        continue;
                    }
                    if(out.at(1)=='%') emit noConfig();
                    else emit error(out.mid(3));
                    return;
                }
                else if(out.at(0)=='~') emit authError();
            }
        }
    }

}

void MentoThread::terminate()
{
    if(pr==NULL){
        QThread::terminate();
        return;
    }
    pr->terminate();
    pr->waitForFinished();
    pr->close();

    if(pr->exitCode()==0||pr->exitStatus()==QProcess::NormalExit)
            emit exitSuccess();
        else emit error(tr("%d").setNum(pr->exitStatus()));
        QThread::terminate();

    //pr->thread()->terminate();
}

void MentoThread::terminatedHandle()
{
    delete pr;
}

MentoThread::~MentoThread(){
    this->terminate();
}
