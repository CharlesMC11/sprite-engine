#import "SCAppDelegate.hh"

#import <Cocoa/Cocoa.h>
#include <Metal/Metal.hpp>
#import <MetalKit/MetalKit.h>

#include <memory>

#import "SCStage.hh"
#include "assets/atlas.hh"
#include "core/memory.hh"
#include "graphics/display_constants.hh"
#include "platform/macos/Renderer.hh"
#include "platform/macos/asset_constants.hh"
#include "simulation/world.hh"

@implementation SCAppDelegate {
    std::unique_ptr<sc::world> _world;
    std::unique_ptr<sc::Renderer> _bridge;
}

- (void)applicationDidFinishLaunching:(nonnull NSNotification*)notification
{
    constexpr double scale{4.0};
    const auto frame{::NSMakeRect(0.0, 0.0, sc::display::WIDTH * scale,
            sc::display::HEIGHT * scale)};
    self.window = [[NSWindow alloc]
            initWithContentRect:frame
                      styleMask:(::NSWindowStyleMaskTitled |
                                        ::NSWindowStyleMaskClosable |
                                        ::NSWindowStyleMaskResizable)
                        backing:NSBackingStoreBuffered
                          defer:NO

    ];
    [self.window setTitle:@"Sprite Engine"];
    [self.window center];

    _world = std::make_unique<sc::world>(sc::memory{});

    id<MTLDevice> device{MTLCreateSystemDefaultDevice()};
    _bridge = std::make_unique<sc::Renderer>((__bridge MTL::Device*) device);

    self.view = [[SCStage alloc] initWithFrame:frame
                                        device:device
                                         world:_world.get()
                                        bridge:_bridge.get()];

    self.window.contentView = self.view;
    [self.window makeKeyAndOrderFront:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
        (nonnull NSApplication*)sender
{
    return YES;
}

@end
