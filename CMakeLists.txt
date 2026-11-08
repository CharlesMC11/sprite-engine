cmake_minimum_required(VERSION 4.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(test-sprite-engine CXX OBJCXX)

include(CheckIPOSupported)
option(ENABLE_LTO "Enable ThinLTO in Release builds" ON)

# Compiler & Linker Options

set(COMMON_COMPILE_OPTIONS
        -D_FORTIFY_SOURCE=2
        -fcolor-diagnostics
        -fstrict-aliasing
        -Wall
        -Wconversion
        -Wextra
        -Wpedantic
)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Generic")
  list(APPEND COMMON_COMPILE_OPTIONS
          -falign-functions=16
          -mbranch-protection=standard
  )
endif ()

set(COMMON_LINK_OPTIONS
        -Wl,-no_warn_duplicate_libraries
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DEBUG)
  list(APPEND COMMON_COMPILE_OPTIONS
          -O0
          -g
          -fno-omit-frame-pointer
          -fno-optimize-sibling-calls
  )
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-4")
  set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT dwarf-with-dsym)
else ()
  add_compile_definitions(NDEBUG)
  list(APPEND COMMON_COMPILE_OPTIONS
          -O2
          -fomit-frame-pointer
  )
  list(APPEND COMMON_LINK_OPTIONS
          -Wl,-dead_strip
  )
  if (ENABLE_LTO)
    check_ipo_supported(RESULT ipo_supported)
    if (ipo_supported)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif ()
  endif ()
endif ()

# Shared Library

add_library(shared STATIC
        src/core/core.hh
        src/core/memory.hh
        src/math/bbox.hh
        src/math/fixed_point.hh
        src/math/fixed_point.cc
        src/math/utils.hh
        src/graphics/graphics_types.hh
        src/assets/atlas.hh
        src/assets/sprite.hh
        src/simulation/physics_types.hh
)

target_include_directories(shared PUBLIC
        src/
)

# Sprite Compiler / Atlas Linker

find_package(Python 3.14 COMPONENTS Interpreter Development.Module REQUIRED)
execute_process(
        COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
        OUTPUT_VARIABLE nanobind_ROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(_pipeline
        pipeline/_pipeline.cc
)

target_include_directories(_pipeline PRIVATE
        .venv/
)

target_compile_options(_pipeline PRIVATE
        ${COMMON_COMPILE_OPTIONS}
)

target_link_libraries(_pipeline PRIVATE shared)

target_link_options(_pipeline PRIVATE
        ${COMMON_LINK_OPTIONS}
)

# Frameworks

find_library(APPKIT_LIB AppKit REQUIRED)
find_library(METAL_LIB Metal REQUIRED)
find_library(METALKIT_LIB MetalKit REQUIRED)
find_library(QUARTZ_LIB QuartzCore REQUIRED)

# Executable

add_executable(sprite-engine
        src/core/bitwise_enum.hh
        src/core/input.hh
        src/core/memory.cc
        src/assets/asset_ids.hh
        src/graphics/display_constants.hh
        src/platform/macos/asset_constants.hh
        src/platform/macos/metal_bridge.hh
        src/platform/macos/metal_bridge.cc
        src/platform/macos/render_constants.hh
        src/platform/macos/SCAppDelegate.hh
        src/platform/macos/SCAppDelegate.mm
        src/platform/macos/SCStage.hh
        src/platform/macos/SCStage.mm
        src/platform/macos/main.mm
        src/registry/entity_registry.hh
        src/registry/entity_registry.cc
        src/simulation/physics.hh
        src/simulation/physics.cc
        src/simulation/spatial_grid.hh
        src/simulation/world.hh
        src/simulation/world.cc
#        src/main.cc
)

target_include_directories(sprite-engine PUBLIC
        vendor/metal-cpp/
)

target_compile_options(sprite-engine PRIVATE
        ${COMMON_COMPILE_OPTIONS}
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_link_libraries(sprite-engine PRIVATE
        shared
        ${APPKIT_LIB}
        ${METAL_LIB}
        ${METALKIT_LIB}
        ${QUARTZ_LIB}
)

target_link_options(sprite-engine PRIVATE
        ${COMMON_LINK_OPTIONS}
)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shader.metallib"
        COMMAND xcrun -sdk macosx metal -c -I "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/macos/shader.metal" -o
        "${CMAKE_CURRENT_BINARY_DIR}/shader.air"
        COMMAND xcrun -sdk macosx metallib
        "${CMAKE_CURRENT_BINARY_DIR}/shader.air" -o
        "${CMAKE_CURRENT_BINARY_DIR}/shader.metallib"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/macos/shader.metal"
)

add_custom_target(shaders DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shader.metallib")
#add_dependencies(sprite-engine shaders)
