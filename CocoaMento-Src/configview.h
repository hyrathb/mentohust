#ifndef CONFIGVIEW_H
#define CONFIGVIEW_H

#include <QMacNativeWidget>
#include <QSettings>
#include "accountform.h"
#include <QTextStream>
#include <QtNetwork/QNetworkInterface>
#include <QList>



namespace Ui {
    class ConfigView;
}
bool readMentoFile(QIODevice &device,QSettings::SettingsMap &map);
bool writeMentoFile(QIODevice &device,const QSettings::SettingsMap &map);


class ConfigView : public QMacNativeWidget
{
    Q_OBJECT

public:
    explicit ConfigView();
    bool autoConnect();
    bool growlStatus();
    QSettings* Config();

    ~ConfigView();
public slots:
    void saveConfig(bool reset=false);
    void resetConfig();
    void getAdapter();
    void showVersionInputDialog();


private:
    int readConfig();
    Ui::ConfigView *ui;
    AccountForm* accountform;
    QSettings* config;

};

#endif // CONFIGVIEW_H
