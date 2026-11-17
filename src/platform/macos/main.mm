#import <AppKit/AppKit.h>

#import "SCAppDelegate.hh"

int main()
{
    @autoreleasepool {
        const auto* app{[NSApplication sharedApplication]};
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        const auto* delegate{[[SCAppDelegate alloc] init]};
        app.delegate = delegate;

        [app run];
    }
}
