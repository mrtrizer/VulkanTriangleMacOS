"strict"

module.exports.run = function (context) {
    config = context.config;
    if (typeof(config.sdl2) !== "object")
        return;
    if (typeof(config.cxx.link_flags) !== "array")
        config.cxx.link_flags = [];
    if (typeof(config.cxx.flags) !== "array")
        config.cxx.flags = [];
    if (typeof(config.sdl2.lib) === "string") {
        config.cxx.link_flags.push("-L" + config.sdl2.lib);
        config.cxx.link_flags.push("-lsdl2");
    }
    if (typeof(config.sdl2.include) === "string") {
        config.cxx.header_dirs.push(config.sdl2.include);
    }
    if (typeof(config.vulkan.sdkPath) === "string") {
        config.cxx.header_dirs.push(config.vulkan.sdkPath + "/macOS/include");
        config.cxx.link_flags.push("-L" + config.vulkan.sdkPath + "/macOS/lib");
        config.cxx.link_flags.push("-lvulkan");
    }
};

module.exports.before = ["gen"];
