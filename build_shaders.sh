#!/bin/sh
pushd shaders
$VULKAN_SDK/bin/glslangValidator -V shader.vert
$VULKAN_SDK/bin/glslangValidator -V shader.frag
popd
