#include "accountform.h"
#include "ui_accountform.h"
#include <QDebug>

static const unsigned char base64Tab[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
static const char xorRuijie[] = {"~!:?$*<(qw2e5o7i8x12c6m67s98w43d2l45we82q3iuu1z4xle23rt4oxclle34e54u6r8m"};

static int encodePass(char *dst, const char *osrc) {
    unsigned char in[3], buf[70];
        unsigned char *src = buf;
        int sz = strlen(osrc);
    int i, len;
        if (sizeof(xorRuijie) < sz)
                return -1;
        for(i=0; i<sz; i++)
                src[i] = osrc[i] ^ xorRuijie[i];
    while (sz > 0) {
        for (len=0, i=0; i<3; i++, sz--) {
                        if (sz > 0) {
                                len++;
                                in[i] = src[i];
            } else in[i] = 0;
        }
        src += 3;
        if (len) {
                        dst[0] = base64Tab[ in[0] >> 2 ];
                        dst[1] = base64Tab[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
                        dst[2] = len > 1 ? base64Tab[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=';
                        dst[3] = len > 2 ? base64Tab[ in[2] & 0x3f ] : '=';
            dst += 4;
        }
    }
    *dst = '\0';
        return 0;
}

static int decodePass(char *dst, const char *src) {
        unsigned esi = 0, idx = 0;
        int i=0, j=0, equal=0;
        for(; src[i]!='\0'; i++) {
                if (src[i] == '=') {
                        if (++equal > 2)
                                return -1;
                } else {
                        for(idx=0; base64Tab[idx]!='\0'; idx++) {
                                if(base64Tab[idx] == src[i])
                                        break;
                        }
                        if (idx == 64)
                                return -1;
                        esi += idx;
                }
                if(i%4 == 3) {
                        dst[j++] = (char)(esi>>16);
                        if(equal < 2)
                                dst[j++] = (char)(esi>>8);
                        if(equal < 1)
                                dst[j++] = (char)esi;
                        esi = 0;
                        equal = 0;
                }
                esi <<= 6;
        }
        if (i%4!=0 || sizeof(xorRuijie)<j)
                return -1;
        for(i=0; i<j; i++)
                dst[i] ^= xorRuijie[i];
        dst[j] = '\0';
        return 0;
}


AccountForm::AccountForm(QSettings *settingref, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccountForm)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);
    ref=settingref;
    connect(ui->Cancel,SIGNAL(clicked()),this,SLOT(close()));
    connect(ui->Confirm,SIGNAL(clicked()),this,SLOT(Confirm()));
    connect(ui->Name,SIGNAL(returnPressed()),this,SLOT(Confirm()));
    connect(ui->Pass,SIGNAL(returnPressed()),this,SLOT(Confirm()));
}

AccountForm::~AccountForm()
{
    delete ui;
}

void AccountForm::setAccount(const char *name, const char *pass)
{
    char pw[65*4/3]={0};
    if(ref==0)return;
    if(name!=0) ref->setValue(tr("Username"),tr(name));
    if(pass!=0)
    {

        if(encodePass(pw,pass))
        {
            QMessageBox::critical(this,"错误","设定密码失败，无法进行加密操作！",QMessageBox::Cancel,QMessageBox::NoButton);
            return;
        }
        ref->setValue(tr("EncodePass"),tr(pw));
    }
    if(!ref->isWritable()){
        QMessageBox::critical(this,"错误","写入配置文件失败！",QMessageBox::Cancel,QMessageBox::NoButton);
        return;
    }
    ui->Name->setText(ref->value(tr("Username")).toString());
    //qDebug()<<ref->value(tr("Username")).toString();
    if(decodePass(pw,ref->value(tr("EncodePass")).toString().toUtf8().data()))
    {
        QMessageBox::critical(this,"错误","解析密码失败！请重设密码！",QMessageBox::Ok,QMessageBox::NoButton);
        ui->Pass->setText(tr(""));
        ref->remove(tr("EncodePass"));
        this->show();
    }
    else ui->Pass->setText(tr(pw));
}
void AccountForm::show()
{
    setAccount();
    QWidget::show();
}

void AccountForm::Confirm()
{
    if(ui->Name->text().length()==0 || ui->Pass->text().length()==0)
    {
        QMessageBox::warning(this,"Warning","用户名和密码不能为空！",QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    setAccount(ui->Name->text().toUtf8().data(),ui->Pass->text().toUtf8().data());
    QWidget::close();
}
