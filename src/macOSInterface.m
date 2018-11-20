#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "macOSInterface.hpp"

const void* initMetal(const void* pView) {
    NSView* view = [(NSWindow*)pView contentView];
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    CAMetalLayer *metalLayer = [CAMetalLayer layer];
    metalLayer.device = device;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.frame = view.bounds;
    [view.layer addSublayer:metalLayer];
    return metalLayer;
}

void createMacOsApp () {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    id applicationName = [[NSProcessInfo processInfo] processName];
    id window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 120, 120)
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];
    [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
    [window setTitle: applicationName];
    [window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

void runMacOsApp() {
    [NSApp run];
}
