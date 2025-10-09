cmake_minimum_required(VERSION 3.16)
project(LearningOpenGL C CXX)
set(CMAKE_CXX_STANDARD 20)

#add_subdirectory(external/DiscordGameSDK)

option(DEV_GAME_FILES_DIR "Path to the dev game files" "../game_files")
message(STATUS "Using game files dir: ${DEV_GAME_FILES_DIR}")

add_executable(LearningOpenGL
        main.cpp
        external/glad/gl.c
        Shader.cpp
        Shader.h
        Mesh.cpp
        Mesh.h
        Camera.cpp
        Camera.h
        Input.cpp
        Objects/GameObject.cpp
        Objects/GameObject.h
        Objects/MeshObject.cpp
        Objects/MeshObject.h
        Objects/ChunkObject.cpp
        Objects/ChunkObject.h
        Chunk.cpp
        Chunk.h
        World.cpp
        World.h
        WorldRenderer.cpp
        WorldRenderer.h
        EaseValue.cpp
        EaseValue.h
        EaseVec3.cpp
        EaseVec3.h
        Objects/BlockHighlightObject.cpp
        Objects/BlockHighlightObject.h
        stb_image.h
        Texture.cpp
        Texture.h
        Objects/CrosshairObject.cpp
        Objects/CrosshairObject.h
        Player.cpp
        Player.h
        stb_easy_font.h
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_impl_glfw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_impl_opengl3.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui.cpp
        Objects/GaussianBlurObject.cpp
        Objects/GaussianBlurObject.h
        SceneFramebuffer.cpp
        SceneFramebuffer.h
        lighting_config.h
        Objects/ShadowMap.cpp
        Objects/ShadowMap.h
        FastNoiseLite.h
        Objects/PlayerObject.cpp
        Objects/PlayerObject.h
        BlockRegistry.cpp
        BlockRegistry.h
        arena_alloc.cpp
        arena_alloc.h
        Objects/BatchShapeRenderer.cpp
        Objects/BatchShapeRenderer.h
        build_config.h
)

#target_link_libraries(LearningOpenGL PRIVATE DiscordGameSDK)

target_include_directories(LearningOpenGL PRIVATE external)

find_package(glm CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
target_link_libraries(LearningOpenGL PRIVATE glfw ${CMAKE_DL_LIBS} glm::glm)
target_compile_definitions(LearningOpenGL PRIVATE DEV_GAME_FILES_DIR="${DEV_GAME_FILES_DIR}")

#copy_discord_sdk_runtime(LearningOpenGL)
