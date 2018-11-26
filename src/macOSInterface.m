#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "macOSInterface.hpp"

const void* initMetal(const void* pView) {
    NSView* view = [(__bridge NSWindow*)pView contentView];
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    CAMetalLayer *metalLayer = [CAMetalLayer layer];
    metalLayer.device = device;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.frame = view.bounds;
    [view.layer addSublayer:metalLayer];
    return (__bridge void*)metalLayer;
}

@interface MyWindowDelegate : NSObject <NSWindowDelegate>

@property void (*callback)(void*);
@property void* userDataPtr;

@end

void* createMacOsApp () {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    id applicationName = [[NSProcessInfo processInfo] processName];
    id window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(100, 100, 640, 640)
        styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask
        backing:NSBackingStoreBuffered
        defer:NO
    ];
    [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
    [window setTitle: applicationName];
    [window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
    return (__bridge void*) window;
}

void runMacOsApp(void* pWindow, void (*callback_)(void*), void* userDataPtr) {
    NSWindow* window = (__bridge NSWindow*)pWindow;
    MyWindowDelegate* windowDelegate = [MyWindowDelegate alloc];
    windowDelegate.callback = callback_;
    windowDelegate.userDataPtr = userDataPtr;
    [window setDelegate:windowDelegate];
    [NSApp run];
}

@implementation MyWindowDelegate

@synthesize callback;
@synthesize userDataPtr;

- (void)windowDidUpdate:(NSNotification *)notification {
    callback(userDataPtr);
}

- (void)windowWillClose:(NSNotification *)notification {
    [NSApp terminate:self];
}

@end
