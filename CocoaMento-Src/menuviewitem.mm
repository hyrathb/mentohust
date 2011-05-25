#include "menuviewitem.h"

#import "Cocoa/Cocoa.h"

MenuViewItem::MenuViewItem(const char *title, QMacNativeWidget *qw):MenuItem(title)
{
    this->setTitle(title);
    setWidget(qw);
}

void MenuViewItem::setWidget(QMacNativeWidget *qw){
    this->w=qw;
    showView();
}

void MenuViewItem::showView()
{
    if(this->w!=0) {
        [(NSMenuItem*)this->cocoaMenuItemRef() setView:(reinterpret_cast<NSView *>(this->w->winId()))];
        this->w->show();
    }
    else{
        hideView();
    }
}

void MenuViewItem::hideView()
{
    [(NSMenuItem*)this->cocoaMenuItemRef() setView:nil];
}

MenuViewItem::~MenuViewItem()
{
}
