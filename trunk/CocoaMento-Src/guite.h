#ifndef GUITE_H
#define GUITE_H

#include <QWidget>

namespace Ui {
    class Guite;
}

class Guite : public QWidget
{
    Q_OBJECT

public:
    explicit Guite(QWidget *parent = 0);
    ~Guite();
public slots:
    void showAbout();
    void showHelp();
private:
    Ui::Guite *ui;
};

#endif // GUITE_H
