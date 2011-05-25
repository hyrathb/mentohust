//
//  cocoamenuitem.m
//  HelloCocoa
//
//  Created by 云尔 on 11-3-29.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#include "menuitem.h"

#import <Cocoa/Cocoa.h>

@interface cocoamenuitem : NSMenuItem {
    @public
    CocoaMenuItemVirtual * p;

}

-(void) setWarp:(CocoaMenuItemVirtual* ) b;
-(IBAction) trigger:(id)sender;

@end

@implementation cocoamenuitem

- (id)init
{
    self = [super init];
    if (self) {
        // Initialization code here.
        [self setTarget:self];
        [self setAction:@selector(trigger:)];
    }
    
    return self;
}
- (void) setWarp:(CocoaMenuItemVirtual*) base
{
    self->p = base;
}

- (IBAction) trigger:(id)sender
{
    if(self->p) self->p->doTrigger();
}


- (void)dealloc
{
    [super dealloc];
}

@end

class MenuItem::MenuItemCocoaBridge
{
public:
    cocoamenuitem * item;
};

MenuItem::MenuItem(const char * title)
{
    CocoaMenuItemVirtual();
    b=new MenuItemCocoaBridge();
    b->item=[[cocoamenuitem alloc] init ];
    setTitle(title);
    [b->item setWarp:this];
    [b->item retain];
}

void MenuItem::setTitle(const char* title)
{
    [b->item setTitle:[NSString stringWithUTF8String:title]];
}

void* MenuItem::cocoaMenuItemRef()
{
    return (void *) this->b->item;
}

void MenuItem::setEnable(bool isEnable)
{
    if(isEnable) [b->item  setEnabled:YES];
    else [b->item setEnabled:NO];
}

bool MenuItem::isEnable()
{
    if([b->item isEnabled]==NO) return false;
    return true;
}

void MenuItem::setSubMenu(Menu *m)
{
    if(m!=0){
        [b->item setSubmenu:(NSMenu*)m->cocoaMenuRef()];
    }
}

MenuItem::~MenuItem()
{
    [b->item release];
    delete b;
}
