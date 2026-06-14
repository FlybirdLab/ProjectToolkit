-- RapidJSON - Fast JSON parser/generator for C++
require("rapidjson", {
    source = "https://github.com/Tencent/rapidjson.git",
    mirrors = {
        "https://gitee.com/mirrors/rapidjson.git"
    },
    ref = "v1.1.0",
    build = "header_only",
    export_defines = false,
    artifacts = {
        include_dirs = { "include" }
    }
})
