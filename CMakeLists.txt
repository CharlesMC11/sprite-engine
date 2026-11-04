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
  set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT dwarf-with-dysm)
  set(GET_TASK_ALLOW true)
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
  set(GET_TASK_ALLOW false)
endif ()

# Shared Library

add_library(shared INTERFACE
        src/assets/atlas.hh
        src/assets/sprite.hh
        src/core/core.hh
        src/graphics/graphics_types.hh
        src/math/bbox.hh
        src/math/utils.hh
        src/simulation/physics_types.hh
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
        src/
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
        src/assets/asset_constants.hh
        src/assets/asset_ids.hh
        src/core/input.hh
        src/core/memory.cc
        src/core/memory.hh
        src/graphics/display_constants.hh
        src/graphics/graphics_types.cc
        src/graphics/metal_bridge.hh
        src/graphics/metal_bridge.cc
        src/graphics/render_constants.hh
        src/graphics/shader.metal
        src/math/bbox.hh
        src/platform/SCAppDelegate.hh
        src/platform/SCAppDelegate.mm
        src/platform/SCStage.hh
        src/platform/SCStage.mm
        src/platform/main.mm
        src/registry/entity_registry.hh
        src/registry/entity_registry.cc
        src/simulation/physics.hh
        src/simulation/physics.cc
        src/simulation/spatial_grid.hh
        src/simulation/spatial_grid.cc
        src/simulation/world.hh
        src/simulation/world.cc
)

target_include_directories(sprite-engine PUBLIC
        src/
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
        "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/shader.metal" -o
        "${CMAKE_CURRENT_BINARY_DIR}/shader.air"
        COMMAND xcrun -sdk macosx metallib
        "${CMAKE_CURRENT_BINARY_DIR}/shader.air" -o
        "${CMAKE_CURRENT_BINARY_DIR}/shader.metallib"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/shader.metal"
)

add_custom_target(shaders DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/shader.metallib")
add_dependencies(sprite-engine shaders)
