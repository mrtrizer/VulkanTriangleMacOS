Before launching configure enviroment (via schemes in case of Xcode) as:
VK_ICD_FILENAMES = <path>/vulkansdk/macOS/etc/vulkan/icd.d/MoltenVK_icd.json
VK_LAYER_PATH = <path>/vulkansdk/macOS/etc/vulkan/explicit_layer.d

Also set a working dir as root of repository. Shaders are searched relative to the project root.
