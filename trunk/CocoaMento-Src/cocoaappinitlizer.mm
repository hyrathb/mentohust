#include "cocoaappinitlizer.h"
#import <Cocoa/Cocoa.h>
#import <Security/Authorization.h>
#import <Growl/Growl.h>

@interface Growl : NSObject {
}
@end
@implementation Growl
- (id)init
{
    self = [super init];
    if (self) {
        [GrowlApplicationBridge setGrowlDelegate: self];
    }

    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void) notify:(NSString*)title
        detail:(NSString*)det
{
    [GrowlApplicationBridge notifyWithTitle: title
    description: det
    notificationName: @"CocoaMento"
    iconData: nil
    priority: 0
    isSticky: NO
    clickContext: nil];
}

- (NSDictionary *) registrationDictionaryForGrowl
{
    NSArray *notifications;
    notifications = [NSArray arrayWithObject: @"CocoaMento"];

    NSDictionary *dict;

    dict = [NSDictionary dictionaryWithObjectsAndKeys:
            notifications, GROWL_NOTIFICATIONS_ALL,
            notifications, GROWL_NOTIFICATIONS_DEFAULT, nil];

    return (dict);

}

@end
Growl* growlproxy;

namespace CocoaInitialize{

CocoaAppInitlizer* defaultApp;


class CocoaAppInitlizer::AutoReleasePoolWarp
{
public:
    NSAutoreleasePool * releasepool;
    NSConnection *connection;

};


CocoaAppInitlizer::CocoaAppInitlizer()
{
    w=new AutoReleasePoolWarp();
    w->releasepool=[[NSAutoreleasePool alloc] init];

}
CocoaAppInitlizer::~CocoaAppInitlizer()
{
     [w->connection release];
     free(w->releasepool);
     delete w;
}


bool CocoaAppInitlizer::checkRunning()
{
    w->connection= [NSConnection connectionWithRegisteredName:@"com.myapp.cocoamento" host:nil];
    if([w->connection isValid]==NO){
        [w->connection release];
        w->connection=[[NSConnection alloc] init];
        [w->connection registerName:@"com.myapp.cocoamento"];
        return 0;
    }
    else
    {
        return 1;
    }
}

void CocoaAppInitlizer::requestFocus()
{
    [NSApp activateIgnoringOtherApps:YES];
}

const char* mainBundlePath()
{
    if(defaultApp==NULL) return "";
    return [[[NSBundle mainBundle] bundlePath] UTF8String];
}

int auth()
{
    if(defaultApp==NULL) return 0;
    AuthorizationRef authMento;
    NSString *auth = [[[NSBundle mainBundle] bundlePath] stringByAppendingString: @"/Contents/MacOS/auth"];
    AuthorizationFlags authFlags = kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed | kAuthorizationFlagPreAuthorize | kAuthorizationFlagExtendRights;
    AuthorizationItem authItems[] = {kAuthorizationRightExecute, strlen([auth UTF8String]), (void*)auth, 0};
    AuthorizationRights authRights = {sizeof(authItems)/sizeof(AuthorizationItem), authItems};
    if(AuthorizationCreate(&authRights, kAuthorizationEmptyEnvironment, authFlags, &authMento) != errAuthorizationSuccess) return 0;
    if(AuthorizationExecuteWithPrivileges(authMento, [auth UTF8String], kAuthorizationFlagDefaults,NULL, NULL))return 0;
    sleep(800);
    if(AuthorizationExecuteWithPrivileges(authMento, [auth UTF8String], kAuthorizationFlagDefaults,NULL, NULL)) return 0;
    return 1;
}
void initCocoaApp(){
    if(defaultApp==NULL)
        defaultApp=new CocoaAppInitlizer();
}
int exitCocoaApp()
{
    if(growlproxy!=NULL) [growlproxy release];
    if(defaultApp!=NULL) delete defaultApp;
    return 0;
}

CocoaAppInitlizer* DefaultApp()
{
    return defaultApp;
}

void growl(const char*title,const char *msg)
{
        if(growlproxy==NULL) growlproxy=[[Growl alloc] init];
        [growlproxy notify:[NSString stringWithUTF8String:title] detail:[NSString stringWithUTF8String:msg]];

}
}
