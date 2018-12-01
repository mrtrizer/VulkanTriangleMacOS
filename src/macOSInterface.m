#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include "macOSInterface.hpp"

/** The main view controller for the demo storyboard. */
@interface DemoViewController : NSViewController
    @property void *userDataPtr;
    @property void (*updateHandler)();
    -(void)setUserData: (void*) userData;
@end

/** The Metal-compatibile view for the demo Storyboard. */
@interface DemoView : NSView
@end

@interface MyWindowDelegate : NSObject <NSWindowDelegate>
@end

MacOsApp createMacOsApp (void (*updateHandler)(void*)) {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    id applicationName = [[NSProcessInfo processInfo] processName];
    NSWindow* window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(100, 100, 640, 640)
        styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask
        backing:NSBackingStoreBuffered
        defer:NO
    ];
    [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
    [window setTitle: applicationName];
    [window makeKeyAndOrderFront:nil];
    MyWindowDelegate* windowDelegate = [MyWindowDelegate alloc];
    [window setDelegate:windowDelegate];
    [NSApp activateIgnoringOtherApps:YES];
    DemoViewController* controller = [[DemoViewController alloc] initWithNibName : nil bundle: nil];
    controller.updateHandler = updateHandler;
    // Setting contentViewControlelr initiates DemoView creating and [DemoViewController viewDidLoad]
    window.contentViewController = controller;
    // Next lines are executed when DemoView is fully initialized along with CAMetalLayer
    MacOsApp macOsApp;
    macOsApp.nsViewController = (__bridge void*) controller;
    macOsApp.caMetalLayer = (__bridge void*) window.contentView.layer;
    macOsApp.nsWindow = (__bridge void*) window;
    return macOsApp;
}

void setUserData(const MacOsApp* macOsApp, void* userDataPtr) {
    [((__bridge DemoViewController*)macOsApp->nsViewController) setUserData : userDataPtr];
}

// Using of global flags is bad design. But I don't know how to pass an event to
// following do-while cycle better way and have to time and will to sort this out.
// I feel like MyWindowDelegate should emit some event about closing the window,
// or may be I evendon't need the MyWindowDelegate class and I could just
// catch this event in main event loop. Honestly I don't really know. :)
bool run = true;

void runMacOsApp(MacOsApp* macOsApp) {
    (void)macOsApp;
    do {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:[NSDate distantFuture]
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        [NSApp sendEvent:event];
        [NSApp updateWindows];
    } while (run);
    [(__bridge NSWindow*)macOsApp->nsWindow close];
}

typedef struct UpdateClosure {
    void (*func)(void* userDataPtr);
    void* userDataPtr;
} UpdateClosure;

/** Rendering loop callback function for use with a CVDisplayLink. */
static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink,
                                    const CVTimeStamp* now,
                                    const CVTimeStamp* outputTime,
                                    CVOptionFlags flagsIn,
                                    CVOptionFlags* flagsOut,
                                    void* target) {
    UpdateClosure* updateClosure = ((UpdateClosure*)target);
    updateClosure->func(updateClosure->userDataPtr);
    return kCVReturnSuccess;
}

@implementation DemoViewController {
        CVDisplayLinkRef _displayLink;
        UpdateClosure updateClosure;
    }

    @synthesize userDataPtr;
    @synthesize updateHandler;

    -(void)loadView {
        self.view = [[DemoView alloc] initWithFrame:NSMakeRect(0, 0, 640, 640)];
    }

    -(void) dealloc {
        CVDisplayLinkRelease(_displayLink);
    }

    /** Since this is a single-view app, initialize Vulkan during view loading. */
    -(void) viewDidLoad {
        [super viewDidLoad];

        self.view.wantsLayer = YES;		// Back the view with a layer created by the makeBackingLayer method.

        CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
        updateClosure.func = updateHandler;
        updateClosure.userDataPtr = userDataPtr;
        CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, &updateClosure);
        CVDisplayLinkStart(_displayLink);
    }

    -(void)setUserData: (void*) userDataPtr_ {
        self.userDataPtr = userDataPtr_;
        if (_displayLink != nil) {
            updateClosure.userDataPtr = userDataPtr;
            CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, &updateClosure);
        }
    }
@end

@implementation DemoView

    /** Indicates that the view wants to draw using the backing layer instead of using drawRect:.  */
    -(BOOL) wantsUpdateLayer { return YES; }

    /** Returns a Metal-compatible layer. */
    +(Class) layerClass { return [CAMetalLayer class]; }

    /** If the wantsLayer property is set to YES, this method will be invoked to return a layer instance. */
    -(CALayer*) makeBackingLayer {
        CALayer* layer = [self.class.layerClass layer];
        CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
        layer.contentsScale = MIN(viewScale.width, viewScale.height);
        return layer;
    }

@end

@implementation MyWindowDelegate

- (void)windowWillClose:(NSNotification *)notification {
    run = false;
}

@end
