#include "guite.h"
#include "ui_guite.h"

Guite::Guite(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Guite)
{
    ui->setupUi(this);
}

Guite::~Guite()
{
    delete ui;
}

void Guite::showAbout()
{
    ui->Direction->setCurrentIndex(0);
    this->show();
}

void Guite::showHelp()
{
    ui->Direction->setCurrentIndex(1);
    this->show();
}
