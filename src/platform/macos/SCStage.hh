#import <MetalKit/MetalKit.h>

#include "platform/macos/Renderer.hh"
#include "simulation/world.hh"

@interface SCStage : MTKView <MTKViewDelegate>

- (instancetype)initWithFrame:(CGRect)frameRect
                       device:(id<MTLDevice>)device
                        world:(sc::world*)world
                       bridge:(sc::Renderer*)bridge;
@end
