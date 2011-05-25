#ifndef ACCOUNTFORM_H
#define ACCOUNTFORM_H

#include <QWidget>
#include <QSettings>
#include <QMessageBox>

namespace Ui {
    class AccountForm;
}

class AccountForm : public QWidget
{
    Q_OBJECT

public:
    explicit AccountForm(QSettings* settingref,QWidget *parent = 0);
    ~AccountForm();

    void setAccount(const char* name=0,const char* pass=0);
    const char * Name();
    const char * Pass();
public slots:
    void show();
    void Confirm();

private:
    Ui::AccountForm *ui;
    QSettings* ref;
};

#endif // ACCOUNTFORM_H
