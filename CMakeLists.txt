cmake_minimum_required (VERSION 3.1.0)
project(TestApp LANGUAGES CXX C)

set(CMAKE_BUILD_TYPE Debug)

file(GLOB TestApp_SOURCES
    "./src/*.hpp"
    "./src/*.cpp"
    "./src/*.m"
)

message(status "list: ${TestApp_SOURCES}")

add_executable(TestApp ${TestApp_SOURCES})
    
set (VULKAN_SDK "/Users/deniszdorovtsov/.local/vulkansdk")

# Pathes for header search
target_include_directories(TestApp
    PRIVATE
    "./src"
    "${VULKAN_SDK}/macOS/include"
    "./3dparty/glm"
)

# All preprocessor defenition from module configuration
target_compile_options(TestApp PRIVATE  $<$<COMPILE_LANGUAGE:CXX>:--std=c++17> -fobjc-arc)
find_library(COCOA_LIBRARY Cocoa)
find_library(METAL_LIBRARY Metal)
find_library(METAL_KIT_LIBRARY MetalKit)
find_library(QUARTZ_CORE_LIBRARY QuartzCore)
find_library(MOLTENVK_LIBRARY MoltenVK)
find_library(IOKIT_LIBRARY IOKit)
target_link_libraries(TestApp PRIVATE ${IOKIT_LIBRARY} ${COCOA_LIBRARY} ${METAL_LIBRARY} ${METAL_KIT_LIBRARY} ${QUARTZ_CORE_LIBRARY} -L/Users/deniszdorovtsov/.local/vulkansdk/macOS/lib -lvulkan )

