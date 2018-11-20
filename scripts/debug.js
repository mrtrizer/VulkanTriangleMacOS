"strict"

module.exports.run = function (context) {
    console.log("config: " + JSON.stringify(context, null, 2));
};

module.exports.after = ["gen"];
