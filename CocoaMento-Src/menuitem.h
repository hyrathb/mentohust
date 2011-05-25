//
//  menubridge.h
//  HelloCocoa
//
//  Created by 云尔 on 11-3-29.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//




#ifndef _MENU_ITEM_H
#define _MENU_ITEM_H

#include <QString>
#include "cocoamenuitemvirtual.h"
#include "menu.h"

class MenuItem:public CocoaMenuItemVirtual
{
public:
    MenuItem(const char* title);
    void * cocoaMenuItemRef();
    void setTitle(const char * title);
    void setEnable(bool isEnable);
    void setSubMenu(Menu* m);
    bool isEnable();
    ~MenuItem();

private:
    class MenuItemCocoaBridge;
   // class QtBridge;
    MenuItemCocoaBridge * b;
};


#endif
