#ifndef MENU_H
#define MENU_H


class Menu
{
public:
    Menu(const char * title=0);
    void* cocoaMenuRef();
    void addMenuItem(void* item);
    void addMenuSeparator();
    ~Menu();

private:
    class CocoaMenuBridge;
    CocoaMenuBridge* b;
};

#endif // MENU_H
