// This is your build script. You only need to "bootstrap" it once with `cc -o nob nob.c`
// (you can call the executable whatever actually) or `cl nob.c` on MSVC. After that every
// time you run the `nob` executable if it detects that you modifed nob.c it will rebuild
// itself automatically thanks to NOB_GO_REBUILD_URSELF (see below)

#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"
#define TOOLS_FOLDER "tools/"

int build_assets(Nob_Cmd *cmd)
{
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER))
        return 1;

    nob_cmd_append(cmd,
        "cc", "-Wall", "-Wextra", "-std=c99",
        "-o", TOOLS_FOLDER "asset2pak",
        TOOLS_FOLDER "asset2pak.c");
    if (!nob_cmd_run(cmd))
        return 1;

    nob_cmd_append(cmd,
        "./tools/asset2pak",
        BUILD_FOLDER "assets.pak",
        "assets/objs/teapot.obj",
        "assets/objs/cat.obj",
        "assets/objs/monkey.obj");
    if (!nob_cmd_run(cmd))
        return 1;

    return 0;
}

int build_game(Nob_Cmd *cmd)
{
    if (!nob_mkdir_if_not_exists(BUILD_FOLDER))
        return 1;

    nob_cmd_append(cmd,
        "cc",
        "-Wall", "-Wextra", "-std=c99",
        "-lSDL2", "-lm",
        "-O3", "-march=native", "-flto",
        "-o", BUILD_FOLDER "engine",

        SRC_FOLDER "main.c",
        SRC_FOLDER "app/app.c",
        SRC_FOLDER "core/arena.c",
        SRC_FOLDER "core/mat.c",
        SRC_FOLDER "platform/window.c",
        SRC_FOLDER "platform/input.c",
        SRC_FOLDER "platform/time.c",
        SRC_FOLDER "renderer/renderer.c",
        SRC_FOLDER "core/geom.c",
        SRC_FOLDER "culling.c",
        SRC_FOLDER "scene/teapot_renderer.c",
        SRC_FOLDER "core/camera.c",
        SRC_FOLDER "assets/pakloader.c",
        SRC_FOLDER "assets/objloader.c",
        SRC_FOLDER "ui/overlay.c",
        SRC_FOLDER "assets/model.c",
        SRC_FOLDER "scene/teapot_scene.c",
        SRC_FOLDER "scene/scene.c",
        SRC_FOLDER "scene/scene_factory.c",
        SRC_FOLDER "assets/loader.c",
        SRC_FOLDER "debug/profiler.c",
        SRC_FOLDER "core/camera_input.c",
        SRC_FOLDER "ui/overlay_helpers.c"
    );

    return !nob_cmd_run(cmd);
}

int build_obj2c(Nob_Cmd *cmd)
{
    nob_cmd_append(cmd,
        "cc", "-Wall", "-Wextra", "-std=c99",
        "-o", TOOLS_FOLDER "obj2c",
        TOOLS_FOLDER "obj2c.c");

    return !nob_cmd_run(cmd);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = {0};

    if (argc > 1) {
        if (strcmp(argv[1], "assets") == 0)
            return build_assets(&cmd);

        if (strcmp(argv[1], "game") == 0)
            return build_game(&cmd);

        if (strcmp(argv[1], "obj2c") == 0)
            return build_obj2c(&cmd);
    }

    if (build_assets(&cmd))
        return 1;

    return build_game(&cmd);
}

