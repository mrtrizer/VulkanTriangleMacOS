cmake_minimum_required (VERSION 3.1.0)
project(TestApp LANGUAGES CXX C)



    # Generation of compilation database
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    #IF( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
    #  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    #    /Users/deniszdorovtsov/Projects/VulkanExperiments/TestApp/compile_commands.json
    #  )
    #ENDIF()

    # Add subdirectories of all nested submodules to reffer them from other places
    # and avoid cross-refference problems.
    

set(CMAKE_BUILD_TYPE Debug)




# C++ standard


# List all sources from all presented sources excluding exclude pathes


    list(APPEND TestApp_SOURCES "/Users/deniszdorovtsov/Projects/VulkanExperiments/TestApp/src/macOSInterface.hpp")

    list(APPEND TestApp_SOURCES "/Users/deniszdorovtsov/Projects/VulkanExperiments/TestApp/src/macOSInterface.m")

    list(APPEND TestApp_SOURCES "/Users/deniszdorovtsov/Projects/VulkanExperiments/TestApp/src/main.cpp")



    
        add_executable(TestApp ${TestApp_SOURCES})
    


# Pathes for header search
target_include_directories(TestApp
PRIVATE

    "/Users/deniszdorovtsov/Projects/VulkanExperiments/TestApp/src"

    "/Users/deniszdorovtsov/.local/sdl2/include"

    "/Users/deniszdorovtsov/.local/vulkansdk/macOS/include"

)



# All preprocessor defenition from module configuration
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(TestApp PRIVATE  $<$<COMPILE_LANGUAGE:CXX>:--std=c++17> -fobjc-arc)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(METAL_LIBRARY Metal)
    find_library(METAL_KIT_LIBRARY MetalKit)
    find_library(QUARTZ_CORE_LIBRARY QuartzCore)
    find_library(MOLTENVK_LIBRARY MoltenVK)
    find_library(IOKIT_LIBRARY IOKit)
    target_link_libraries(TestApp PRIVATE ${IOKIT_LIBRARY} ${COCOA_LIBRARY} ${METAL_LIBRARY} ${METAL_KIT_LIBRARY} ${QUARTZ_CORE_LIBRARY} -L/Users/deniszdorovtsov/.local/sdl2/lib -lsdl2 -L/Users/deniszdorovtsov/.local/vulkansdk/macOS/lib -lvulkan )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(TestApp PRIVATE  --std=c++17)
    target_link_libraries(TestApp PRIVATE -L/Users/deniszdorovtsov/.local/sdl2/lib -lsdl2 -L/Users/deniszdorovtsov/.local/vulkansdk/macOS/lib -lvulkan )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    target_compile_options(TestApp PRIVATE  --std=c++17)
    target_link_libraries(TestApp PRIVATE -L/Users/deniszdorovtsov/.local/sdl2/lib -lsdl2 -L/Users/deniszdorovtsov/.local/vulkansdk/macOS/lib -lvulkan )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(TestApp PRIVATE /std:c++17)
    target_link_libraries(TestApp PRIVATE )
endif()








    if (NOT (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR}))
        if (EXISTS ${CMAKE_SOURCE_DIR}/resources)
            add_custom_command(
                    TARGET TestApp POST_BUILD
                    COMMAND cp -R
                            ${CMAKE_SOURCE_DIR}/resources
                            ${CMAKE_CURRENT_BINARY_DIR}/)
        endif()
    endif()

