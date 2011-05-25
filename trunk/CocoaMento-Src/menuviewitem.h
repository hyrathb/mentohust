#ifndef MENUVIEWITEM_H
#define MENUVIEWITEM_H

#include <QMacNativeWidget>
#include "menuitem.h"

class MenuViewItem : public MenuItem
{
public:
    MenuViewItem(const char* title=0,QMacNativeWidget* qw=0);
    void setWidget(QMacNativeWidget * qw);
    void hideView();
    void showView();
    ~MenuViewItem();
private:
    QWidget * w;
};

#endif // MENUVIEWITEM_H
