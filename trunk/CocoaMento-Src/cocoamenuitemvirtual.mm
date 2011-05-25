#include "cocoamenuitemvirtual.h"
#include "qtport.h"


CocoaMenuItemVirtual::CocoaMenuItemVirtual()
{
    port=(void *)(new QtPort());
}

void CocoaMenuItemVirtual::doTrigger()
{
    ((QtPort*)port)->emitEvent();
}

CocoaMenuItemVirtual::~CocoaMenuItemVirtual()
{
   delete (QtPort*)port;
}


