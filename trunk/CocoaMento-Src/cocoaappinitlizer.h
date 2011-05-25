#ifndef COCOAAPPINITLIZER_H
#define COCOAAPPINITLIZER_H

namespace CocoaInitialize{



class CocoaAppInitlizer
{
public:
    CocoaAppInitlizer();
    bool checkRunning();
    void requestFocus();
    ~CocoaAppInitlizer();

private:
    class AutoReleasePoolWarp;
    AutoReleasePoolWarp* w;
};



const char* mainBundlePath();
int auth();
void initCocoaApp();
int exitCocoaApp();
CocoaAppInitlizer* DefaultApp();
void growl(const char*title,const char* msg);

}

#endif // COCOAAPPINITLIZER_H


