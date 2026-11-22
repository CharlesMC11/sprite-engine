#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION

#include "platform/macos/Renderer.hh"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <cassert>

#include "assets/atlas.hh"
#include "graphics/display_constants.hh"
#include "registry/entity_registry.hh"
#include "render_constants.hh"

namespace sc {

    // Constructors

    Renderer::Renderer(MTL::Device* device)
        : device_{NS::TransferPtr(device)},
          cmdQueue_{NS::TransferPtr(device_->newMTL4CommandQueue())},
          cmdBuffer_{NS::TransferPtr(device_->newCommandBuffer())}
    {
        NS::Error* error{nullptr};

        const auto* const libPath{
                NS::String::string(SHADER_LIB_PATH, NS::UTF8StringEncoding)};
        const auto library{
                NS::TransferPtr(device_->newLibrary(libPath, &error))};
        assert(library && !error &&
                error->localizedDescription()->utf8String());

        const auto* fnName{
                NS::String::string("k_clear_screen", NS::UTF8StringEncoding)};
        auto fn{NS::TransferPtr(library->newFunction(fnName))};
        assert(fn && "Could not load `k_clear_screen`.");

        clearPso_ = NS::TransferPtr(
                device_->newComputePipelineState(fn.get(), &error));
        assert(clearPso_ && !error &&
                error->localizedDescription()->utf8String());

        fnName = NS::String::string("k_draw_sprites", NS::UTF8StringEncoding);
        fn = NS::TransferPtr(library->newFunction(fnName));
        assert(fn && "Could not load `k_draw_sprites`.");

        spritePso_ = NS::TransferPtr(
                device_->newComputePipelineState(fn.get(), &error));
        assert(spritePso_ && !error &&
                error->localizedDescription()->utf8String());

        const auto argTable{NS::TransferPtr(
                MTL4::ArgumentTableDescriptor::alloc()->init())};
        argTable->setMaxBufferBindCount(2);

        argTable_ = NS::TransferPtr(
                device_->newArgumentTable(argTable.get(), &error));
        assert(argTable_ && !error &&
                error->localizedDescription()->utf8String());
    }

    // Public methods

    void Renderer::beginFrame(const MTL::Drawable* const buffer)
    {
        computeCmdEncoder_ = cmdBuffer_->computeCommandEncoder();

        const auto* const texture{
                static_cast<const CA::MetalDrawable* const>(buffer)
                        ->texture()};
        computeCmdEncoder_->setTexture(texture, 0UZ);
    }

    void Renderer::endFrame(const MTL::Drawable* buffer)
    {
        computeCmdEncoder_->endEncoding();
        cmdBuffer_->presentDrawable(buffer);
        cmdBuffer_->commit();

        computeCmdEncoder_ = nullptr;
        cmdBuffer_ = nullptr;
    }

    void Renderer::clear() const
    {
        computeCmdEncoder_->setComputePipelineState(clearPso_.get());

        const MTL::Size gridSize{display::WIDTH, display::HEIGHT, 1UZ};
        const MTL::Size threadGroupSize{16UZ, 16UZ, 1UZ};

        computeCmdEncoder_->dispatchThreads(gridSize, threadGroupSize);
    }

    void Renderer::draw(const entity_registry& registry) const
    {
        computeCmdEncoder_->setComputePipelineState(spritePso_.get());

        computeCmdEncoder_->setBuffer(atlasBuffer_.get(), paletteSpanOffset_,
                render::BUFFER_INDEX_PALETTES);

        computeCmdEncoder_->setBuffer(atlasBuffer_.get(), sprite32SpanOffset_,
                render::BUFFER_INDEX_SPRITES);

        using xform = xform_channel;
        computeCmdEncoder_->setBuffer(registryBuffer_.get(),
                registry.offset(xform::x_pos),
                render::BUFFER_INDEX_X_POSITIONS);
        computeCmdEncoder_->setBuffer(registryBuffer_.get(),
                registry.offset(xform::y_pos),
                render::BUFFER_INDEX_Y_POSITIONS);
        computeCmdEncoder_->setBuffer(registryBuffer_.get(),
                registry.offset(xform::z_pos),
                render::BUFFER_INDEX_Z_POSITIONS);

        using index = index_channel;
        computeCmdEncoder_->setBuffer(registryBuffer_.get(),
                registry.offset(index::sprite_index),
                render::BUFFER_INDEX_ATLAS_INDICES);
        computeCmdEncoder_->setBuffer(registryBuffer_.get(),
                registry.offset(index::draw_order),
                render::BUFFER_INDEX_DRAW_ORDER);

        const auto count{static_cast<std::uint32_t>(registry.count())};
        computeCmdEncoder_->setBytes(
                &count, sizeof count, render::BUFFER_INDEX_ENTITY_COUNT);

        const MTL::Size gridSize{display::WIDTH, display::HEIGHT, 1UZ};
        const MTL::Size threadGroupSize{16UZ, 16UZ, 1UZ};

        computeCmdEncoder_->dispatchThreads(gridSize, threadGroupSize);
    }

    // Mutators

    void Renderer::setRegistryBuffer(const entity_registry& registry)
    {
        constexpr std::size_t totalSizeBytes{
                entity_registry::MAX_ENTITY_COUNT *
                entity_registry::TOTAL_CHANNEL_COUNT *
                sizeof(math::fixed_point_t)};

        registryBuffer_ = NS::TransferPtr(device_->newBuffer(registry.buffer(),
                totalSizeBytes, MTL::ResourceStorageModeShared, nullptr));
        assert(registryBuffer_ &&
                "Could not allocate a MTL buffer for the registry.");
    }

    void Renderer::setAtlasBuffer(const assets::atlas& atlas)
    {
        const std::size_t totalSizeBytes{sizeof atlas.meta +
                atlas.meta.palette_block_size +
                atlas.meta.sprite16_block_size +
                atlas.meta.sprite32_block_size};

        atlasBuffer_ = NS::TransferPtr(device_->newBuffer(atlas.data(),
                totalSizeBytes, MTL::ResourceStorageModeShared, nullptr));
        assert(atlasBuffer_ && "Could not allocate a Metal buffer.");

        paletteSpanOffset_ = atlas.meta.palette_block_offset;
        sprite32SpanOffset_ = atlas.meta.sprite32_block_offset;
    }

} // namespace sc
