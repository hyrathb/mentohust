#include "configview.h"
#include "ui_configview.h"
#include "cocoaappinitlizer.h"
#include <QInputDialog>

ConfigView::ConfigView() :
    QMacNativeWidget(0),
    ui(new Ui::ConfigView)
{
    ui->setupUi(this);
    const QSettings::Format MentoFormat=QSettings::registerFormat(tr("mento"),readMentoFile,writeMentoFile,Qt::CaseSensitive);
    config=new QSettings(tr("%1/Contents/MacOS/mentohust.conf").arg(CocoaInitialize::mainBundlePath()),MentoFormat);
    accountform=new AccountForm(config);

    getAdapter();

    if(readConfig())
    {
        QMessageBox::warning(this,tr("警告"),tr("读取配置文件失败，所有配置参数将会被重置为默认值！"),QMessageBox::Ok,QMessageBox::NoButton);
        saveConfig(true);
    }

    connect(ui->AccountSetting,SIGNAL(clicked()),accountform,SLOT(show()));

    connect(ui->StartMode,SIGNAL(currentIndexChanged(int)),this,SLOT(saveConfig()));
    connect(ui->DHCPMode,SIGNAL(currentIndexChanged(int)),this,SLOT(saveConfig()));
    connect(ui->NetInterface,SIGNAL(currentIndexChanged(int)),this,SLOT(saveConfig()));
    connect(ui->RestartWait,SIGNAL(valueChanged(int)),this,SLOT(saveConfig()));
    //connect(ui->Version,SIGNAL(editingFinished()),this,SLOT(saveConfig()));
    connect(ui->Timeout,SIGNAL(valueChanged(int)),this,SLOT(saveConfig()));
    connect(ui->EchoInterval,SIGNAL(valueChanged(int)),this,SLOT(saveConfig()));
    connect(ui->GrowlStatus,SIGNAL(stateChanged(int)),this,SLOT(saveConfig()));
    connect(ui->AutoConnect,SIGNAL(stateChanged(int)),this,SLOT(saveConfig()));
    connect(ui->Version,SIGNAL(clicked()),this,SLOT(showVersionInputDialog()));
}

void ConfigView::resetConfig()
{
    saveConfig(true);
}


int ConfigView::readConfig()
{
    bool ok=true;
    ui->StartMode->setCurrentIndex(config->value(tr("StartMode")).toInt(&ok));
    if(!ok)return 1;
    ui->EchoInterval->setValue(config->value(tr("EchoInterval")).toInt(&ok));
    if(!ok)return 1;
    ui->Version->setText(config->value(tr("Version")).toString());
    ui->DHCPMode->setCurrentIndex(config->value(tr("DhcpMode")).toInt(&ok));
    if(!ok)return 1;
    ui->RestartWait->setValue(config->value(tr("RestartWait")).toInt(&ok));
    if(!ok)return 1;
    ui->Timeout->setValue(config->value(tr("Timeout")).toInt(&ok));
    if(!ok)return 1;
    ui->NetInterface->setCurrentIndex(ui->NetInterface->findText(config->value(tr("Nic")).toString()));
    ui->GrowlStatus->setChecked(config->value(tr("AllowError")).toBool());
    ui->AutoConnect->setChecked(config->value(tr("AutoConnect")).toBool());
    return 0;
}

QSettings* ConfigView::Config()
{
    return config;
}

void ConfigView::showVersionInputDialog()
{
    bool ok=false;
    QString v=QInputDialog::getText(0,tr("请输入版本号"),
                          tr("请输入希望CocoaMento被识别为的版本号。\n此设置视不同学校的情况不同，效果不同。\n如若需要通过V2认证，请参考使用说明设定必须的数据文件。"),
                          QLineEdit::Normal,ui->Version->text(),
                          &ok
                          );
    if(ok){
        ui->Version->setText(v);
        saveConfig();
    }
}

void ConfigView::saveConfig(bool reset)
{
    if(reset)
    {
        ui->EchoInterval->setValue(30);
        ui->StartMode->setCurrentIndex(1);
        ui->Version->setText(tr("3.95_1225"));
        ui->DHCPMode->setCurrentIndex(2);
        ui->RestartWait->setValue(15);
        ui->Timeout->setValue(8);
        ui->NetInterface->setCurrentIndex(-1);
        ui->GrowlStatus->setChecked(false);
        ui->AutoConnect->setChecked(false);
    }
    config->setValue(tr("EchoInterval"),QVariant(ui->EchoInterval->value()));
    config->setValue(tr("StartMode"),QVariant(ui->StartMode->currentIndex()));
    config->setValue(tr("Version"),QVariant(ui->Version->text()));
    config->setValue(tr("DhcpMode"),QVariant(ui->DHCPMode->currentIndex()));
    config->setValue(tr("RestartWait"),QVariant(ui->RestartWait->value()));
    config->setValue(tr("Timeout"),QVariant(ui->Timeout->value()));
    config->setValue(tr("Nic"),QVariant(ui->NetInterface->currentText()));
    config->setValue(tr("GrowlStatus"),QVariant(ui->GrowlStatus->isChecked()));
    config->setValue(tr("AutoConnect"),QVariant(ui->AutoConnect->isChecked()));
}

void ConfigView::getAdapter()
{
    QList<QNetworkInterface> nis=QNetworkInterface::allInterfaces();
    if(nis.length()==0) {
        ui->NetInterface->setCurrentIndex(-1);
        while(ui->NetInterface->count()>0) ui->NetInterface->removeItem(0);
        return;
    }
    foreach(QNetworkInterface ni,nis)
        ui->NetInterface->addItem(ni.name());
}

bool ConfigView::autoConnect()
{
    return config->value(tr("AutoConnect")).toBool();
}
bool ConfigView::growlStatus()
{
    return config->value(tr("GrowlStatus")).toBool();
}

ConfigView::~ConfigView()
{
    accountform->hide();
    accountform->deleteLater();
    delete ui;
}


bool readMentoFile(QIODevice &device, QSettings::SettingsMap &map)
{
    QStringList stl;
    QString str;
    QTextStream ts(&device);
    while(!ts.atEnd())
    {
        str=ts.readLine();
        if(str.at(0)==QChar(';')||str.at(0)==QChar('#')) continue;
        if (str==QString("[MentoHUST]")) continue;
        else
        {
            stl=str.split(QChar('='));
            if(stl.length()==2) map.insert(stl.at(0),QVariant(stl.at(1)));
            else if(stl.at(0)==QString("EncodePass"))
            {
                map.insert(stl.at(0),QVariant(str.mid(11)));
            }
            else return false;
        }
    }
    return true;
}
bool writeMentoFile(QIODevice &device, const QSettings::SettingsMap &map)
{

    device.write("[MentoHUST]\n");
    foreach(QString key,map.keys())
    {
        device.write(QString("%1=%2\n").arg(key).arg(map.value(key).toString()).toUtf8().data());
    }
    return true;
}
