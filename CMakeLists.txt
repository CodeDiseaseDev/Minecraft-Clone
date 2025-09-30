cmake_minimum_required(VERSION 3.16)
project(LearningOpenGL C CXX)
set(CMAKE_CXX_STANDARD 20)

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
)

target_include_directories(LearningOpenGL PRIVATE external)

find_package(glm CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
target_link_libraries(LearningOpenGL PRIVATE glfw ${CMAKE_DL_LIBS} glm::glm)
