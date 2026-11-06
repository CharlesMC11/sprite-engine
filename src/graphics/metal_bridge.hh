#pragma once

#include <Foundation/Foundation.hpp>

#include <cstddef>

namespace MTL {
    class Buffer;
    class CommandBuffer;
    class CommandQueue;
    class ComputeCommandEncoder;
    class ComputePipelineState;
    class Device;
    class Drawable;
} // namespace MTL

namespace sc {

    class entity_registry;
    namespace assets {
        struct atlas;
    } // namespace assets
    namespace core {
        class mapped_view;
    } // namespace core

    namespace render {

        /**
         * High-level interface for the Metal Compute pipeline.
         *
         * Orchestrates the dispatch of kernels to the GPU using
         * `sc::assets::sprite` data as primary input.
         */
        class metal_bridge final {
        public:
            // Constructors

            explicit metal_bridge(MTL::Device* device);
            ~metal_bridge() = default;

            metal_bridge(const metal_bridge&) = delete;
            metal_bridge(metal_bridge&&) = delete;

            auto operator=(const metal_bridge&) -> metal_bridge& = delete;
            auto operator=(metal_bridge&&) -> metal_bridge& = delete;

            // Public methods

            /**
             *
             *
             * @param buffer
             * The destination drawable/texture.
             */
            void begin_frame(const MTL::Drawable* buffer);

            /**
             *
             *
             * @param buffer
             * The destination drawable/texture.
             */
            void end_frame(const MTL::Drawable* buffer);

            /**
             * Encode a command to clear the screen.
             */
            void clear() const;

            /**
             * Encode a draw command for entities on screen.
             *
             * @param registry
             * The registry containing the entities to draw.
             */
            void draw(const entity_registry& registry) const;

            // Mutators

            void set_registry_buffer(const entity_registry& registry);

            /**
             * Create a shared buffer for the atlas.
             *
             * @param atlas
             * The mapped view of atlas to store in the buffer.
             */
            void set_atlas_buffer(const assets::atlas& atlas);

        private:
            // Attributes

            NS::SharedPtr<MTL::Device> device_{nullptr};
            NS::SharedPtr<MTL::CommandQueue> queue_{nullptr};

            NS::SharedPtr<MTL::ComputePipelineState> clear_pso_{nullptr};
            NS::SharedPtr<MTL::ComputePipelineState> sprite_pso_{nullptr};

            NS::SharedPtr<MTL::Buffer> registry_buffer_{nullptr};
            NS::SharedPtr<MTL::Buffer> atlas_buffer_{nullptr};
            std::size_t palette_span_offset_{0UZ};
            std::size_t sprite32_span_offset_{0UZ};

            MTL::CommandBuffer* command_buffer_{nullptr};
            MTL::ComputeCommandEncoder* encoder_{nullptr};
        };

    } // namespace render

} // namespace sc
