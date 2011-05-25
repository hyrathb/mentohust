//
//  StatusIcon.h
//  HelloCocoa
//
//  Created by 云尔 on 11-3-28.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#ifndef _STATUS_ICON_H
#define _STATUS_ICON_H
#include "menu.h"
#include <QString>

class StatusIcon
{
public:
    StatusIcon();
    ~StatusIcon();
    void showIcon();
    void hideIcon();
    void setHighlightMode(bool highlight);
    bool highlightMode();
    void setMenu(Menu* menu);
    void setIcon(const char* name);

private:
    Menu* menu;
    class CocoaBridge;
    CocoaBridge* b;
};


#endif
