#include "statusicon.h"


#import <Cocoa/Cocoa.h>



class StatusIcon::CocoaBridge
{
    public:
        NSStatusItem * statusicon;

};


StatusIcon::StatusIcon()
{
    b=new StatusIcon::CocoaBridge();
    b->statusicon=[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    setHighlightMode(true);
}

void StatusIcon::showIcon()
{
    this->setIcon("normal.png");
}

void StatusIcon::setIcon(const char *name)
{
    [b->statusicon setImage:[NSImage imageNamed:[NSString stringWithUTF8String: name]]];
}

void StatusIcon::setHighlightMode(bool highlight)
{
    if(highlight) [b->statusicon setHighlightMode:YES];
    else [b->statusicon setHighlightMode:NO];
}

bool StatusIcon::highlightMode()
{
    if([b->statusicon highlightMode]==NO) return false;
    return true;
}

void StatusIcon::hideIcon()
{
    [b->statusicon setImage:nil];
}

void StatusIcon::setMenu(Menu *menu)
{
    this->menu=menu;
    [b->statusicon setMenu:(NSMenu*) menu->cocoaMenuRef()];
}


StatusIcon::~StatusIcon(){
    [b->statusicon release];
   delete b;
}
