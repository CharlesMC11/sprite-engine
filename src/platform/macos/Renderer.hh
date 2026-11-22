#pragma once

#include <Foundation/Foundation.hpp>

#include <cstddef>

namespace MTL {
    class Buffer;
    class ComputePipelineState;
    class Device;
    class Drawable;
} // namespace MTL

namespace MTL4 {
    class ArgumentTable;
    class CommandBuffer;
    class CommandQueue;
    class ComputeCommandEncoder;
} // namespace MTL4

namespace sc {

    class entity_registry;
    namespace assets {
        struct atlas;
    } // namespace assets
    namespace core {
        class mapped_view;
    } // namespace core

    /**
     * @class Renderer
     * High-level interface for the Metal Compute pipeline.
     *
     * Orchestrates the dispatch of kernels to the GPU using
     * `sc::assets::sprite` data as primary input.
     */
    class Renderer final {
    public:
        // Static constants

        static constexpr auto SHADER_LIB_PATH{"assets/shader.metallib"};

        // Constructors

        explicit Renderer(MTL::Device* device);
        ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;

        auto operator=(const Renderer&) -> Renderer& = delete;
        auto operator=(Renderer&&) -> Renderer& = delete;

        // Public methods

        /**
         *
         *
         * @param buffer
         * The destination drawable/texture.
         */
        void beginFrame(const MTL::Drawable* buffer);

        /**
         *
         *
         * @param buffer
         * The destination drawable/texture.
         */
        void endFrame(const MTL::Drawable* buffer);

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

        void setRegistryBuffer(const entity_registry& registry);

        /**
         * Create a shared buffer for the atlas.
         *
         * @param atlas
         * The mapped view of atlas to store in the buffer.
         */
        void setAtlasBuffer(const assets::atlas& atlas);

    private:
        // Attributes

        NS::SharedPtr<MTL::Device> device_{nullptr};
        NS::SharedPtr<MTL4::CommandQueue> cmdQueue_{nullptr};
        NS::SharedPtr<MTL4::CommandBuffer> cmdBuffer_{nullptr};
        MTL4::ComputeCommandEncoder* computeCmdEncoder_{nullptr};

        NS::SharedPtr<MTL4::ArgumentTable> argTable_{nullptr};

        NS::SharedPtr<MTL::ComputePipelineState> clearPso_{nullptr};
        NS::SharedPtr<MTL::ComputePipelineState> spritePso_{nullptr};

        NS::SharedPtr<MTL::Buffer> registryBuffer_{nullptr};
        NS::SharedPtr<MTL::Buffer> atlasBuffer_{nullptr};
        std::size_t paletteSpanOffset_{0UZ};
        std::size_t sprite32SpanOffset_{0UZ};
    };

} // namespace sc
