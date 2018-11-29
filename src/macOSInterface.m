#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include "macOSInterface.hpp"

/** The main view controller for the demo storyboard. */
@interface DemoViewController : NSViewController
    @property void *userDataPtr;
    @property void (*updateHandler)();
    @property void (*initHandler)(void* controller, void* caMetalLayer);
@end

/** The Metal-compatibile view for the demo Storyboard. */
@interface DemoView : NSView
@end

@interface MyWindowDelegate : NSObject <NSWindowDelegate>
@end

void createMacOsApp (void (*initHandler)(void*, void*), void (*updateHandler)(void*)) {
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
    controller.initHandler = initHandler;
    controller.updateHandler = updateHandler;
    window.contentViewController = controller;
    [NSApp run];
}

void setUserData(void* controller, void* userDataPtr) {
    ((__bridge DemoViewController*)controller).userDataPtr = userDataPtr;
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
    @synthesize initHandler;

    -(void)loadView {
        self.view = [[DemoView alloc] initWithFrame:NSMakeRect(0, 0, 640, 640)];
    }

    -(void) dealloc {
        //demo_cleanup(&demo);
        CVDisplayLinkRelease(_displayLink);
    }

    /** Since this is a single-view app, initialize Vulkan during view loading. */
    -(void) viewDidLoad {
        [super viewDidLoad];

        self.view.wantsLayer = YES;		// Back the view with a layer created by the makeBackingLayer method.
        const char* arg = "cube";
        if (initHandler != nil)
            initHandler((__bridge void*)self, (__bridge void*)self.view);

        CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
        updateClosure.func = updateHandler;
        updateClosure.userDataPtr = userDataPtr;
        CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, &updateClosure);
        CVDisplayLinkStart(_displayLink);
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
    [NSApp terminate:self];
}

@end
