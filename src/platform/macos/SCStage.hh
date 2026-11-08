#pragma once

#import <MetalKit/MetalKit.h>

#include "platform/macos/metal_bridge.hh"
#include "simulation/world.hh"

@interface SCStage : MTKView <MTKViewDelegate>

- (instancetype)initWithFrame:(CGRect)frameRect
                       device:(id<MTLDevice>)device
                        world:(sc::world*)world
                       bridge:(sc::render::metal_bridge*)bridge;
@end
