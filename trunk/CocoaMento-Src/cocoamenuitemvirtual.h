#ifndef COCOAMENUITEMVIRTUAL_H
#define COCOAMENUITEMVIRTUAL_H



class CocoaMenuItemVirtual
{
public:
   CocoaMenuItemVirtual();
   virtual void doTrigger();
   ~CocoaMenuItemVirtual();
   void* port;
};

#endif // COCOAMENUITEMVIRTUAL_H
