#ifndef MENTOTHREAD_H
#define MENTOTHREAD_H

#include <QSettings>
#include <QThread>
#include <QProcess>

class MentoThread : public QThread
{
    Q_OBJECT
public:
    explicit MentoThread(QSettings * settings);
    void run();
    ~MentoThread();

signals:
    void processCrashed();
    void output(QString str);
    void notify(QString str);
    void successNotify();
    void statusChanged(QString str,int status);
    void error(QString errdetail);
    void exitSuccess();
    void startFailure();
    void authError();
    void noConfig();
public slots:
    void terminate();
    void terminatedHandle();
    void processFinished(int status,QProcess::ExitStatus qstatus);
private:
    QSettings * settings;
};

#endif // MENTOTHREAD_H
