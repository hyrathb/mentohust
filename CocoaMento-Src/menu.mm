#include "menu.h"
#include "menuitem.h"
#import <Cocoa/Cocoa.h>

class Menu::CocoaMenuBridge
{
    public:
        NSMenu* menu;
};

Menu::Menu(const char *title)
{
    b=new CocoaMenuBridge();
    if(title==0) title="";
    b->menu=[[NSMenu alloc] initWithTitle:[NSString stringWithUTF8String: title]];
    if(title!=0){
        [b->menu addItemWithTitle:[NSString stringWithUTF8String: title]  action:nil keyEquivalent:@""];
        [[b->menu itemAtIndex:0] setEnabled:NO];
    }
    [b->menu setAutoenablesItems: NO];
    [b->menu retain];
}

void Menu::addMenuItem(void *item)
{
    [b->menu addItem:((NSMenuItem * )((MenuItem*)item)->cocoaMenuItemRef())];
}
void Menu::addMenuSeparator()
{
    [b->menu addItem:[NSMenuItem separatorItem]];
}

void* Menu::cocoaMenuRef()
{
    return (void*) b->menu;
}

Menu::~Menu()
{
    [b->menu release];
    delete b;
}
