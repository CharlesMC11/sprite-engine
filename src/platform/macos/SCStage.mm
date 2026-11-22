#import "SCStage.hh"

#include <Metal/Metal.hpp>
#import <MetalKit/MetalKit.h>

#include "core/input.hh"
#include "graphics/display_constants.hh"
#include "platform/macos/Renderer.hh"
#include "simulation/world.hh"

@implementation SCStage {
    sc::world* _world;
    sc::Renderer* _bridge;
}

- (instancetype)initWithFrame:(CGRect)frameRect
                       device:(id<MTLDevice>)device
                        world:(sc::world*)world
                       bridge:(sc::Renderer*)bridge
{
    self = [super initWithFrame:frameRect device:device];
    if (self) {
        self.delegate = self;
        self.drawableSize =
                ::CGSizeMake(sc::display::WIDTH, sc::display::HEIGHT);
        self.preferredFramesPerSecond =
                static_cast<NSInteger>(sc::display::TARGET_FPS);

        self.layer.magnificationFilter = kCAFilterNearest;

        self.framebufferOnly = false;

        _world = world;
        _bridge = bridge;
    }

    return self;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)mouseDown:(nonnull NSEvent*)event
{
    [self.window makeFirstResponder:self];
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

- (void)drawInMTKView:(nonnull MTKView*)view
{
    const auto* drawable = (__bridge MTL::Drawable*) view.currentDrawable;
    _world->tick(_keysPressed);
}

@end
