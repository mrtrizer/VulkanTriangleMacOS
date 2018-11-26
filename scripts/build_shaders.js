
function call(command, cwd) {
    const childProcess = require("child_process");
    childProcess.execSync(command, {"cwd": cwd, stdio: "inherit"});
}

module.exports.run = function (context) {
    const path = require("path");
    const cmd = path.join(context.config.vulkan.sdkPath, "macOS/bin/glslangValidator");
    const cwd = path.join(context.moduleRoot, "shaders");
    call(cmd + " -V shader.vert", cwd);
    call(cmd + " -V shader.frag", cwd);
};

module.exports.before = ["gen"];
