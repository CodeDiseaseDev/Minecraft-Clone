#include <cstdio>
#include <cstdlib>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Input.h"
#include "Shader.h"
#include "Mesh.h"
#include "Objects/MeshObject.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "arena_alloc.h"
#include "build_config.h"
#include "DiscordRPC.h"
#include "EaseValue.h"
#include "EaseVec3.h"
#include "Player.h"
#include "SceneFramebuffer.h"
#include "Texture.h"
#include "World.h"
#include "WorldRenderer.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Objects/BatchShapeRenderer.h"
#include "Objects/BlockHighlightObject.h"
#include "Objects/ChunkObject.h"
#include "Objects/CrosshairObject.h"
#include "Objects/GaussianBlurObject.h"
#include "Objects/PlayerObject.h"
#include "Objects/ShadowMap.h"


// #include "DiscordGameSDK/types.h"

// std::unique_ptr<MeshObject> mesh_object;
// std::unique_ptr<ChunkObject> chunk_object;


// std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
// std::cout << "GLSL version: "   << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
// std::cout << "Renderer: "       << glGetString(GL_RENDERER) << std::endl;
// std::cout << "Vendor: "         << glGetString(GL_VENDOR) << std::endl;

struct hardware_info {
    const char* OpenGLVersion;
    const char* GLSLVersion;
    const char* GraphicsHardware;
    const char* GraphicsVendor;
};

DiscordRPC discord_rpc;

World* world;
Player player;

float frames_per_second = 0;

Camera camera(16/9);

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

hardware_info hwinfo;

std::optional<RaycastHit> camera_block_raycast_hit;

EaseVec3 player_wasd_velocity {};
// glm::vec3 player_jump_velocity {};
EaseVec3 block_selection_pos {};
EaseValue camera_zoom{camera.fov, camera.fov};

// aspect ratio gets updated every frame

struct ChatMessage {
    std::string content;
    std::string username = "You";

};

struct ChatState {
    std::vector<ChatMessage> messages;
    char inputBuf[1024] = "";
    bool isOpen = false;
};


float normal_fov;
float zoomed_fov;

float camera_sensitivity;
bool mouse_captured = false;

int render_distance_radius;

bool third_person;

bool render_world;
bool render_block_highlight;

bool fly_mode;

int w = 600,
    h = 600;

bool does_imgui_have_keyboard = false;

bool render_self_player_object = false;

float walk_speed;
float sprint_speed;
// float jump_height;
EaseValue wasd_movement_speed {walk_speed, sprint_speed};

std::vector<PlayerObject*> player_characters;

// std::shared_ptr<Shader> shader;
// std::shared_ptr<Shader> block_highlight_shader;
// std::shared_ptr<WorldRenderer> world_renderer;
//
// std::shared_ptr<BlockHighlightObject> block_highlight_object;
//
// std::shared_ptr<Texture> texture_atlas;
//
//
// std::shared_ptr<Shader> crosshair_shader;
//
// std::shared_ptr<CrosshairObject> crosshair_object;
// std::shared_ptr<Texture> crosshair_texture;
//
// std::shared_ptr<GaussianBlurObject> blur_object;
// std::shared_ptr<Shader> horizontal_blur_shader;
// std::shared_ptr<Shader> vertical_blur_shader;
// std::shared_ptr<SceneFramebuffer> sceneFBO;
//
// std::shared_ptr<Shader> depth_shader;
// std::shared_ptr<ShadowMap> shadow_depth_map;
//
// std::shared_ptr<Shader> player_shader;
// std::shared_ptr<PlayerObject> player_object;

arena::Allocator<std::byte> engineArena;

Shader* shader = nullptr;
Shader* block_highlight_shader = nullptr;
WorldRenderer* world_renderer = nullptr;

BlockHighlightObject* block_highlight_object = nullptr;

Texture* texture_atlas = nullptr;


Shader* crosshair_shader = nullptr;
Texture* crosshair_texture = nullptr;

GaussianBlurObject* blur_object = nullptr;
Shader* horizontal_blur_shader = nullptr;
Shader* vertical_blur_shader = nullptr;
SceneFramebuffer* sceneFBO = nullptr;

Shader* depth_shader = nullptr;
ShadowMap* shadow_depth_map = nullptr;

Shader* player_shader = nullptr;
PlayerObject* player_object = nullptr;

Shader* batch_renderer_shader = nullptr;
BatchShapeRenderer* batch_shape_renderer = nullptr;

/* Icons */
Texture* no_keyboard_icon = nullptr;

unsigned int blur_texture;

// std::unique_ptr<Mesh> au_faaaalcon;
// std::shared_ptr<MeshObject> au_falcon_obj;

GLFWwindow* win = nullptr;

std::map<std::string, Shader*> loadedShaders;

ChatState chat_state;

lighting_config preset_no_post_processing = lighting_config{
    {1.0f, 1.0f, 1.0f},
    {0, 0, 0},
    1.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    1.0f,
    {0.6f, 0.7f, 1.0f},
    0.0f,
    150.0f,
    150.0f,
    0,
    1,
    0
};

lighting_config preset_default = preset_no_post_processing;

lighting_config preset_beautiful = {
    {1.0f, 1.0f, 1.0f},
    {70, -35, -65},

    1.0f,

    0.1f,
    2.3f,
    0.0f,
    0.0f,
    1.35f,
    0.95f,

    {0.6f, 0.7f, 1.0f},
    0.03f,
    50.0f,
    150.0f,

    5,
    0,
    1.0f
};

lighting_config preset_realistic = {
    {1.0f, 1.0f, 1.0f},
    {35, -35, -75},

    1.0f,

    0.115, // ambient lighting strength
    1.4f,
    0.0f,
    0.0f,
    0.75f,
    1.0f,

    {0.6f, 0.7f, 1.0f},
    0.03f,
    50.0f,
    150.0f,

    3,
    0,
    1.115f
};

void on_shader_load(Shader* shader, const std::string display_name) {
    loadedShaders[display_name] = shader;
}

glm::vec3 determine_rand_spawn_location() {
    const float rnd_max = (float)RAND_MAX;
    float x = rand() / rnd_max * 2000;
    float z = rand() / rnd_max * 2000;
    // float y = find_world_land_y_at(glm::vec2(x, z));

    return glm::vec3(x, 0, z);
}

float find_world_land_y_at(glm::vec2 pos) {
    float y = 50;

    while (world->getBlockAt(pos.x, y, pos.y).isAir()) {
        y--;
    }

    return y + 1;
}

void use_initial_values() {
    normal_fov = 120;
    zoomed_fov = 40;

    camera_sensitivity = 20.0f;
    // mouse_captured = false;

    render_distance_radius = 1;

    third_person = false;




    render_world = true;
    render_block_highlight = false;

    w = 600;
    h = 600;

    frames_per_second = 0;

    camera = Camera(16/9);
    camera.lighting_shader_config = preset_default;

    player = Player();

    world->gravity = -15;

    walk_speed = 4.0f;
    sprint_speed = walk_speed * 2.5f;
    player.jumpStrength = 9.0f;



    // player.position = determine_rand_spawn_location();
    player.position.y = find_world_land_y_at(
        glm::vec2(player.position.x, player.position.z)) + 1;

    camera.fov = normal_fov;
}

void reload_shaders_only() {
    for (auto& [name, shader] : loadedShaders) {
        if (shader && shader->isValid()) {
            shader->reload();
            printf("[ShaderReload] %s reloaded.\n", name.c_str());
        } else {
            fprintf(stderr, "⚠️  Shader '%s' invalid during reload.\n", name.c_str());
        }
    }
}


void load_blur_shaders() {
    std::ifstream hor, ver, vert;

    std::filesystem::path shaders_dir =
        std::filesystem::current_path() /
        build_config::game_files_directory /
        "shaders";

    hor.open(shaders_dir / "gaussian_blur_horizontal.frag");
    ver.open(shaders_dir / "gaussian_blur_vertical.frag");
    vert.open(shaders_dir / "gaussian_blur.vert");

    std::stringstream horizontal, vertical, vertex;
    horizontal << hor.rdbuf();
    vertical << ver.rdbuf();
    vertex << vert.rdbuf();

    std::string horFragSource = horizontal.str();
    std::string verFragSource = vertical.str();
    std::string vertSource = vertex.str();

    // horizontal_blur_shader = engineArena.allocate()

    horizontal_blur_shader = arena_allocate<Shader>(
        engineArena,
        std::vector<std::pair<const char*, GLenum>>{
            {vertSource.c_str(), GL_VERTEX_SHADER},
            {horFragSource.c_str(), GL_FRAGMENT_SHADER},
        },
        "gaussian_blur_horizontal"
    );
    on_shader_load(horizontal_blur_shader, "H Blur Shader");

    vertical_blur_shader = arena_allocate<Shader>(
        engineArena,
        std::vector<std::pair<const char*, GLenum>>{
            {vertSource.c_str(), GL_VERTEX_SHADER},
            {verFragSource.c_str(), GL_FRAGMENT_SHADER},
        },
        "gaussian_blur_vertical"
    );
    on_shader_load(vertical_blur_shader, "V Blur Shader");

    blur_object = arena_allocate<GaussianBlurObject>(
        engineArena,
        vertical_blur_shader,
        horizontal_blur_shader, w,h);

    sceneFBO = arena_allocate<SceneFramebuffer>(
        engineArena,
        w,h);
}



void initialise() {
    /* AUTO_DEALLOCATE will delete the pointer IF IT EXISTS, to prevent memory leaks */

    AUTO_DEALLOCATE(World, world);
    world = arena_allocate<World>(engineArena, engineArena, 0);

    use_initial_values();

    AUTO_DEALLOCATE(Shader, shader);
    shader = Shader::loadFromName("mesh_object", engineArena);
    on_shader_load(shader, "Primary Mesh Shader");

    AUTO_DEALLOCATE(Shader, block_highlight_shader);
    block_highlight_shader = Shader::loadFromName("block_highlight", engineArena);
    on_shader_load(block_highlight_shader, "Block Highlight Shader");

    AUTO_DEALLOCATE(Shader, depth_shader);
    depth_shader = Shader::loadFromName("depth", engineArena);
    on_shader_load(depth_shader, "Depth Shader");

    AUTO_DEALLOCATE(Shader, player_shader);
    player_shader = Shader::loadFromName("player", engineArena);
    on_shader_load(player_shader, "Player Shader");

    AUTO_DEALLOCATE(Shader, batch_renderer_shader);
    batch_renderer_shader = Shader::loadFromName(
        "batch_draw", engineArena);
    on_shader_load(batch_renderer_shader, "2D Graphics Shader");



    AUTO_DEALLOCATE(Texture, texture_atlas);
    texture_atlas = arena_allocate<Texture>(engineArena);
    std::filesystem::path ta = build_config::textures_dir /
        std::filesystem::path("atlas3.png");
    texture_atlas->load(ta.c_str());

    AUTO_DEALLOCATE(Texture, crosshair_texture);
    crosshair_texture = arena_allocate<Texture>(engineArena);
    std::filesystem::path cr = build_config::textures_dir /
        std::filesystem::path("crosshair.png");
    crosshair_texture->load(cr.c_str());

    AUTO_DEALLOCATE(Texture, no_keyboard_icon);
    no_keyboard_icon = arena_allocate<Texture>(engineArena);
    std::filesystem::path nk = build_config::textures_dir /
        std::filesystem::path("no-keyboard.png");
    no_keyboard_icon->load(nk.c_str());

    // 3. Now that all shaders/textures exist → load blur shaders
    load_blur_shaders();

    AUTO_DEALLOCATE(BatchShapeRenderer, batch_shape_renderer);
    batch_shape_renderer = arena_allocate<BatchShapeRenderer>(
        engineArena, batch_renderer_shader);

    AUTO_DEALLOCATE(PlayerObject, player_object);
    // player_object = arena_allocate<PlayerObject>(
    //     engineArena, engineArena, player_shader, texture_atlas);
    player_object = arena_allocate<PlayerObject>(
        engineArena, engineArena, shader, texture_atlas);

    AUTO_DEALLOCATE(BlockHighlightObject, block_highlight_object);
    block_highlight_object = arena_allocate<BlockHighlightObject>(
        engineArena, engineArena, block_highlight_shader);


    AUTO_DEALLOCATE(ShadowMap, shadow_depth_map);
    shadow_depth_map = arena_allocate<ShadowMap>(
        engineArena, depth_shader, 2048 * 6);

    AUTO_DEALLOCATE(WorldRenderer, world_renderer);
    world_renderer = arena_allocate<WorldRenderer>(
        engineArena, shader, texture_atlas, world, engineArena);

    discord_rpc.setup();
}

void clear_chunks() {
    for (auto& [coord, chunk_column] : world->chunkColumns) {
        chunk_column.deallocate(engineArena);
    }
    for (auto& chunk_object : world_renderer->visibleChunks) {
        chunk_object->~ChunkObject();
        AUTO_DEALLOCATE(ChunkObject, chunk_object);
    }

    world->chunkColumns.clear();
    world_renderer->visibleChunks.clear();
}

void reload_game() {
    initialise();

    world->chunkColumns.clear();
    world_renderer->visibleChunks.clear();

    printf("Reloaded game!\n");
}



void setup_imgui_theme(bool dark = true)
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // === WINDOW / PANELS ===
    colors[ImGuiCol_WindowBg]         = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);   // translucent dark
    colors[ImGuiCol_ChildBg]          = ImVec4(0.05f, 0.05f, 0.05f, 0.0f);
    colors[ImGuiCol_PopupBg]          = ImVec4(0.08f, 0.08f, 0.08f, 0.9f);
    colors[ImGuiCol_Border]           = ImVec4(0.15f, 0.15f, 0.15f, 0.4f);

    // === TITLE BAR (solid black) ===
    colors[ImGuiCol_TitleBg]          = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);   // solid black
    colors[ImGuiCol_TitleBgActive]    = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);   // solid black (active)
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.9f);   // slightly translucent

    // === HEADERS / COLLAPSIBLES ===
    colors[ImGuiCol_Header]           = ImVec4(0.18f, 0.18f, 0.18f, 0.8f);
    colors[ImGuiCol_HeaderHovered]    = ImVec4(0.25f, 0.25f, 0.25f, 0.9f);
    colors[ImGuiCol_HeaderActive]     = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);

    // === BUTTONS ===
    colors[ImGuiCol_Button]           = ImVec4(0.16f, 0.18f, 0.20f, 0.8f);
    colors[ImGuiCol_ButtonHovered]    = ImVec4(0.25f, 0.27f, 0.30f, 1.0f);
    colors[ImGuiCol_ButtonActive]     = ImVec4(0.30f, 0.34f, 0.40f, 1.0f);

    // === FRAMES / INPUTS ===
    colors[ImGuiCol_FrameBg]          = ImVec4(0.0f,  0.0f,  0.0f,  0.3f);
    colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.20f, 0.22f, 0.25f, 1.0f);
    colors[ImGuiCol_FrameBgActive]    = ImVec4(0.3f, 0.3f, 0.30f, 1.0f);

    // === ACCENTS ===
    colors[ImGuiCol_CheckMark]        = ImVec4(0.20f, 0.75f, 0.45f, 1.0f);
    colors[ImGuiCol_SliderGrab]       = ImVec4(0.20f, 0.75f, 0.45f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.90f, 0.55f, 1.0f);

    // === SEPARATORS (black + transparent) ===
    colors[ImGuiCol_Separator]        = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
    colors[ImGuiCol_SeparatorActive]  = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);

    // === TEXT ===
    colors[ImGuiCol_Text]             = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    colors[ImGuiCol_TextDisabled]     = ImVec4(0.55f, 0.55f, 0.55f, 1.0f);

    // === SCROLLBAR ===
    colors[ImGuiCol_ScrollbarBg]      = ImVec4(0.05f, 0.05f, 0.05f, 0.3f);
    colors[ImGuiCol_ScrollbarGrab]    = ImVec4(0.25f, 0.25f, 0.25f, 0.8f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 0.9f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);

    // === SHAPES ===
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 0.0f;
    style.PopupRounding     = 0.0f;
    style.GrabRounding      = 0.0f;
    style.ScrollbarRounding = 0.0f;

    // === SPACING ===
    style.WindowPadding     = ImVec2(12, 8);
    style.FramePadding      = ImVec2(6, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.ScrollbarSize     = 14.0f;

    // === ANTIALIAS ===
    style.AntiAliasedFill  = true;
    style.AntiAliasedLines = true;


    // ImGuiStyle& style = ImGui::GetStyle();
    //
    //
    // if(dark)
    // {
    //     ImGui::StyleColorsDark();
    // }
    // else
    // {
    //     ImGui::StyleColorsLight();
    // }
    //
    // for (int i = 0; i <= ImGuiCol_COUNT; i++)
    // {
    //     ImGuiCol_ ei = (ImGuiCol_)i;
    //     ImVec4& col = style.Colors[i];
    //     if(  (ImGuiCol_ModalWindowDimBg  != ei ) &&
    //         ( ImGuiCol_NavWindowingDimBg != ei ) &&
    //         ( col.w < 1.00f || ( ImGuiCol_FrameBg  == ei )
    //                         || ( ImGuiCol_WindowBg == ei )
    //                         || ( ImGuiCol_ChildBg  == ei ) ) )
    //     {
    //         col.w = 0.5f * col.w;
    //     }
    // }
    //
    // style.ChildBorderSize = 1.0f;
    // style.FrameBorderSize = 0.0f;
    // style.PopupBorderSize = 1.0f;
    // style.WindowBorderSize = 0.0f;
    // style.FrameRounding = 6.0f;
    // style.Alpha = 1.0f;
}

void set_mouse_captured() {
    mouse_captured = !mouse_captured;

    glfwSetInputMode(win, GLFW_CURSOR, mouse_captured ?
        GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

    if (mouse_captured) {
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
    }
    else {
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    }

    Input::CaptureMouse(mouse_captured);
}

void chunk_updater_tick(int radius) {
    std::vector<Chunk*> updated = world->ensureChunkAndNeighbors(
            camera.position.x,
            camera.position.y,
            camera.position.z, radius);

    if (!updated.empty()) {
        world_renderer->rebuildTheseChunks(camera, updated);
    }

    world_renderer->tick(camera.position, radius * 2);
}

// only ever call once per frame
float calculate_fps() {
    static double lastTime = 0.0;
    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime;
    lastTime = currentTime;

    return 1.0f / (float)delta;
}

void init_imgui(GLFWwindow* win) {
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault();
    io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Multi-viewport

    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGui::StyleColorsDark();
    setup_imgui_theme();



}

void teleport(float x, float y, float z) {
    player.position = glm::vec3(x, y, z);
    player.rotation = glm::vec3(0,0,0);
}

void render_blur_at(unsigned int blurTex, int x, int y, int w, int h) {
    batch_shape_renderer->Start();

    int texW = camera.screenSize.x;
    int texH = camera.screenSize.y;

    glm::vec2 pos  = { x, y };
    glm::vec2 size = { w, h };

    // normalized region
    glm::vec4 uvRegion = {
        pos.x / texW,
        pos.y / texH,
        size.x / texW,
        size.y / texH
    };

    batch_shape_renderer->DrawTexture(
        blurTex,
        pos, size,
        {1, 1, 1, 1},
        uvRegion,
        {texW, texH}
    );

    batch_shape_renderer->End(camera);
}

void render_2d() {
    batch_shape_renderer->Start();

    int crosshair_size = std::min(
        crosshair_texture->width,
        crosshair_texture->height);

    batch_shape_renderer->DrawTexture(
        crosshair_texture,
        { camera.screenSize.x / 2 - crosshair_size / 2,
          camera.screenSize.y / 2 + crosshair_size / 2 },
        { crosshair_size, crosshair_size }, {1,1,1,1});

    if (does_imgui_have_keyboard) {
        batch_shape_renderer->DrawTexture(
            no_keyboard_icon,
            { 10, 10 },
            {no_keyboard_icon->width, no_keyboard_icon->height},
            {1,1,1,1},{0,0,1,1},true);
    }

    batch_shape_renderer->End(camera);
}

void render_imgui_window_blur() {
    ImVec2 pos  = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    if (camera.lighting_shader_config.bypassPostProcessing)
        return;

    render_blur_at(
        blur_texture,
        pos.x, pos.y,
        size.x, size.y
    );
}



void draw_chat_overlay(ChatState& chat)
{
    ImGuiIO& io = ImGui::GetIO();

    // invisible fullscreen "canvas"
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##chat_overlay", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings);

    float lineHeight = ImGui::GetTextLineHeightWithSpacing();
    float padding = 8.0f;
    float x = 10.0f;
    float y = io.DisplaySize.y - padding;
    constexpr int field_height = 28;

    auto chat_bg = IM_COL32(0, 0, 0, 180);

    // Draw messages (bottom-up)
    for (int i = (int)chat.messages.size() - 1; i >= 0; --i)
    {
        const std::string msg = "<" + chat.messages[i].username + "> " + chat.messages[i].content;
        y -= lineHeight;

        ImVec2 textSize = ImGui::CalcTextSize(msg.c_str());
        ImDrawList* draw = ImGui::GetWindowDrawList();
        float width = std::max(textSize.x + 8, 300.0f);
        render_blur_at(
            blur_texture,
            x, y - 2 - field_height,
            width, lineHeight
        );
        draw->AddRectFilled(
            ImVec2(x, y - 2 - field_height),
            ImVec2(x + width, y + lineHeight - 2 - field_height),
            chat_bg, 0);
        ImGui::SetCursorScreenPos(ImVec2(x + 4, y - field_height));
        ImGui::TextUnformatted(msg.c_str());
    }
    ImGui::End();

    // If chat is open, draw input field
    if (chat.isOpen)
    {
        render_blur_at(
            blur_texture,
            padding, io.DisplaySize.y - field_height - padding,
            io.DisplaySize.x - padding * 2, field_height
        );

        ImGui::SetNextWindowPos(ImVec2(padding + 1, io.DisplaySize.y - field_height - padding));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - padding * 2, field_height));
        // ImGui::SetNextItemWidth(io.DisplaySize.x - padding * 2);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("##chat_input", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoMove );

        ImGui::PushStyleColor(ImGuiCol_FrameBg, chat_bg);

        if (ImGui::InputTextMultiline("##input", chat.inputBuf, sizeof(chat.inputBuf),
                    ImVec2(io.DisplaySize.x - padding * 2, field_height),
                     ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (strlen(chat.inputBuf) > 0) {
                chat.messages.emplace_back(chat.inputBuf);
                chat.inputBuf[0] = '\0';
            }
        }


        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::End();
    }
}

void DrawArenaMemoryStats(const char* label, const arena::Allocator<std::byte>& allocator)
{
    auto format_size = [](double bytes) -> std::string {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit_index = 0;
        while (bytes >= 1024.0 && unit_index < 4) {
            bytes /= 1024.0;
            ++unit_index;
        }
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%.2f %s", bytes, units[unit_index]);
        return buffer;
    };

    double used = allocator.used_bytes();
    double total = allocator.total_bytes();

    std::string used_str  = format_size(used);
    std::string total_str = format_size(total);

    ImGui::Text("%s: %s / %s", label, used_str.c_str(), total_str.c_str());
}


void render_engine_status_window() {



    if (ImGui::Begin("Engine Status")) {
        render_imgui_window_blur();
        auto show_ptr = [](const char* label, void* ptr) {
            long arena_ptr_int = reinterpret_cast<long>(&engineArena);
            long ptr_int = reinterpret_cast<long>(ptr);

            if (ptr)
                ImGui::Text("%-25s: %p", label, ptr_int - arena_ptr_int);
            else
                ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%-22s: (null)", label);
        };


        // --- Game ---
        ImGui::SeparatorText("Game");
        ImGui::Text("Frame Time: %.0ffps (%.1fms)",
            ImGui::GetIO().Framerate,
            deltaTime * 1000.0f);




        // --- Shaders ---
        ImGui::SeparatorText("Shaders");
        // show_ptr("Main Shader", shader);
        // show_ptr("Block Highlight Shader", block_highlight_shader);
        // show_ptr("Crosshair Shader", crosshair_shader);
        // show_ptr("Horizontal Blur Shader", horizontal_blur_shader);
        // show_ptr("Vertical Blur Shader", vertical_blur_shader);
        // show_ptr("Depth Shader", depth_shader);
        // show_ptr("Player Shader", player_shader);
        for (auto [name, shader] : loadedShaders) {
            show_ptr(name.c_str(), shader);
        }
        ImGui::Spacing();

        // --- Objects ---
        ImGui::SeparatorText("Objects");
        show_ptr("World Renderer", world_renderer);
        show_ptr("Block Highlight Object", block_highlight_object);
        show_ptr("Blur Object", blur_object);
        show_ptr("Player Object", player_object);
        ImGui::Spacing();

        // --- Textures / FBOs ---
        ImGui::SeparatorText("Textures / Framebuffers");
        show_ptr("Texture Atlas", texture_atlas);
        show_ptr("Crosshair Texture", crosshair_texture);
        show_ptr("Scene Framebuffer", sceneFBO);
        show_ptr("Shadow Depth Map", shadow_depth_map);
        ImGui::Spacing();


        ImGui::SeparatorText("Player");
        ImGui::Text("Position: (%.2f, %.2f, %.2f)",
            player.position.x, player.position.y, player.position.z);
        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)",
            player_wasd_velocity.value.x, player_wasd_velocity.value.y, player_wasd_velocity.value.z);
        ImGui::Text("Rotation: (%.2f, %.2f, %.2f)",
            player.rotation.x, player.rotation.y, player.rotation.z);
        ImGui::Text("On Ground: %s", player.is_on_ground ? "true" : "false");
        ImGui::Text("Sprinting: %s", player.is_sprinting ? "true" : "false");
        ImGui::Text("Jump Velocity Y: %.3f", player.jump_velocity.y);
        ImGui::Text("Gravity Velocity Y: %.3f", player.gravity_velocity.y);
        ImGui::Spacing();




        ImGui::SeparatorText("World");
        ImGui::Text("Render Distance: %d chunks", render_distance_radius);
        ImGui::Text("Visible Chunks: %zu", world_renderer ? world_renderer->visibleChunks.size() : 0);
        ImGui::Text("Chunk Columns: %zu", world->chunkColumns.size());
        ImGui::Spacing();




        ImGui::SeparatorText("System");
        ImGui::Text("Resolution: %dx%d", w, h);
        ImGui::Text("Mouse Captured: %s", mouse_captured ? "true" : "false");
        ImGui::Text("Free Cam: %s", fly_mode ? "true" : "false");
        DrawArenaMemoryStats("Arena Memory", engineArena);
        ImGui::Text("GPU: %s", hwinfo.GraphicsHardware);
        ImGui::Text("GPU Vendor: %s", hwinfo.GraphicsVendor);




    }
    ImGui::End();
}

void render_imgui() {
    draw_chat_overlay(chat_state);

    render_engine_status_window();

    ImGui::SetNextWindowSize(ImVec2(320,700), ImGuiCond_Once);
    ImGui::Begin("Debug Menu");
    render_imgui_window_blur();

    // Helper lambda for float sliders
    auto sliderFloat = [&](const char* label, float* value, float min, float max) {
        ImGui::TextUnformatted(label);
        ImGui::PushID(label);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SliderFloat("##slider", value, min, max);
        ImGui::PopID();
        ImGui::Spacing();
    };

    // Helper lambda for int sliders
    auto sliderInt = [&](const char* label, int* value, int min, int max) {
        ImGui::TextUnformatted(label);
        ImGui::PushID(label);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SliderInt("##slider", value, min, max);
        ImGui::PopID();
        ImGui::Spacing();
    };

    {
        ImGui::SeparatorText("** Variables **");

        ImGui::Spacing();
        sliderFloat("FOV", &normal_fov, zoomed_fov, 160);
        sliderFloat("Camera Sensitivity", &camera_sensitivity, 5, 80);
        sliderFloat("Walk Speed", &walk_speed, 1, 30);
        sliderFloat("Sprint Speed", &sprint_speed, 1, 60);
        sliderFloat("Jump Height", &player.jumpStrength, 1, 80);
        sliderInt("Render Distance (Radius)", &render_distance_radius, 0, 20);

        ImGui::Spacing();
        ImGui::Checkbox("Block Highlight", &render_block_highlight);
        ImGui::SameLine();
        ImGui::Checkbox("Free Cam", &fly_mode);

        ImGui::Checkbox("Capture Mouse (Q)", &mouse_captured);
        ImGui::SameLine();
        ImGui::Checkbox("Chunk Renderer", &render_world);

        ImGui::Checkbox("3rd Person", &third_person);

    }

    ImGui::Spacing();
    {
        ImGui::SeparatorText("** Lighting Shader **");
        ImGui::Spacing(); ImGui::Spacing();



        // Lighting shader config sliders
        sliderFloat("Specular Strength",   &camera.lighting_shader_config.specularStrength, 0, 3);
        sliderFloat("Ambient Strength",    &camera.lighting_shader_config.ambientStrength, 0, 3);
        sliderFloat("Diffuse Strength",    &camera.lighting_shader_config.diffuseStrength, 0, 3);
        ImGui::Spacing();

        sliderFloat("Shininess",           &camera.lighting_shader_config.shininess, 0, 250);
        sliderFloat("Contrast",            &camera.lighting_shader_config.contrast, 0, 3);
        sliderFloat("Vibrancy",            &camera.lighting_shader_config.vibrancy, 0, 3);
        sliderFloat("Ambient Occlusion",   &camera.lighting_shader_config.ambientOcclusion, 0, 3);
        sliderFloat("Day / Night Value",   &camera.lighting_shader_config.sky_night_day_light_modifier, 0, 1);
        ImGui::Spacing();

        sliderFloat("Fog Start",           &camera.lighting_shader_config.fogStart, 0, 400);
        sliderFloat("Fog End",             &camera.lighting_shader_config.fogEnd, 0, 400);

        sliderInt("Shadow Quality (PCF Radius)", &camera.lighting_shader_config.pcfRadius, 0, 8);
    }

    ImGui::Spacing();
    {
        ImGui::SeparatorText("** Lighting Shader PRESETS **");

        ImGui::Spacing();
        ImGui::Spacing();
        if (ImGui::Button("Default")) {
            camera.lighting_shader_config = preset_default;
        }
        ImGui::SameLine();
        if (ImGui::Button("Realistic")) {
            camera.lighting_shader_config = preset_realistic;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stunning")) {
            camera.lighting_shader_config = preset_beautiful;
        }



        if (ImGui::Button("No Post Processing")) {
            camera.lighting_shader_config = preset_no_post_processing;
        }

    }

    ImGui::Spacing();
    {

        // ImGui::BeginChild("Idk");
        ImGui::SeparatorText("** Misc **");

        ImGui::Spacing();

        static int x = 0, y = 0, z = 0;
        static bool correct_y = true;
        ImGui::Text("Teleport");
        ImGui::Checkbox("Correct Y Level Automatically", &correct_y);
        ImGui::PushItemWidth(80);
        ImGui::InputInt("X", &x);
        ImGui::SameLine();
        if (correct_y) ImGui::BeginDisabled();
        ImGui::InputInt("Y", &y);
        if (correct_y) ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::InputInt("Z", &z);
        ImGui::PopItemWidth();
        // ImGui::SameLine();
        if (correct_y) y = find_world_land_y_at(glm::vec2(x,z)) + 1;
        if (ImGui::Button("Teleport")) {
            teleport(x, y, z);
        }
        ImGui::SameLine();
        if (ImGui::Button("Copy Current")) {
            x = player.position.x;
            y = player.position.y;
            z = player.position.z;
        }
        ImGui::Spacing();

        // ImGui::Text("** Chunks **");
        ImGui::Text("Shaders, chunks & textures.");
        if (ImGui::Button("Clear Chunks")) {
            clear_chunks();
        }
        ImGui::SameLine();


        if (ImGui::Button("Reload entire game")) {
            reload_game();
        }

        if (ImGui::Button("Reload shaders only")) {
            reload_shaders_only();
        }
    }


    ImGui::End();
}


void apply_mouse_delta_to_rotation(glm::vec2 delta, glm::vec3& rotation, float sensitivity, float deltaTime, bool clamp = true, bool use_z = false) {
    float y_delta = delta.y * sensitivity * deltaTime;

    if (use_z) rotation.z += y_delta;
    else rotation.y -= y_delta;

    rotation.x -= delta.x * sensitivity * deltaTime;

    if (clamp) {
        if (use_z) {
            if (rotation.z > 89.0f) rotation.z = 89.0f;
            if (rotation.z < -89.0f) rotation.z = -89.0f;
        }
        else {
            if (rotation.y > 89.0f) rotation.y = 89.0f;
            if (rotation.y < -89.0f) rotation.y = -89.0f;
        }

        if (rotation.x > 360.0f) rotation.x = 0;
        if (rotation.x < 0) rotation.x = 360.0f;
    }

}

void set_block(glm::ivec3 pos, BlockID new_block) {
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;

    auto block = Block{
        new_block,
        pos
    };

    world->setBlockAtAndUpdate(
        x, y, z,
        block
    );
}

void break_block(BlockID block_id = BlockID::Air) {
    // doesnt work for some reason? lol
    set_block(
        camera_block_raycast_hit->voxel,
        block_id
    );
}

void place_block(BlockID block_id) {
    set_block(
        camera_block_raycast_hit->adjacent(),
        block_id
    );
}

void build_tower(int height, int radius) {
    glm::ivec3 base = glm::ivec3(player.position);

    for (int y = 0; y < height; y++) {
        for (int x = -radius; x <= radius; x++) {
            for (int z = -radius; z <= radius; z++) {
                float dist = std::sqrt(float(x*x + z*z));
                if (dist >= radius - 0.5f && dist <= radius + 0.5f) {
                    set_block(base + glm::ivec3(x, y, z), BlockID::Stone);
                }
            }
        }
    }

    const float turnsPerHeight = 0.6f;
    for (float r = 1; r <= 3; r += 0.5f) {
        for (float y = 0; y < height; y += 0.2f) {
            float angle = y * turnsPerHeight;
            float fx = std::cos(angle) * (radius - r);
            float fz = std::sin(angle) * (radius - r);

            glm::ivec3 stairPos = base + glm::ivec3(
                (int)std::round(fx), (int)std::floor(y), (int)std::round(fz));

            set_block(stairPos, BlockID::Stone);
        }
    }

}

void clear_area(int width = 50, int length = 50, int height = 50) {

    int bottomY = player.position.y;
    int centerX = player.position.x;
    int centerZ = player.position.z;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            for (int z = 0; z < length; z++) {
                glm::ivec3 blockPos = {
                    x + centerX - (width / 2),
                    y + bottomY,
                    z + centerZ - (length / 2)
                };
                set_block(blockPos, BlockID::Air);
            }
        }
    }
}

void create_house() {
    int centerX = floor(player.position.x);
    int centerZ = floor(player.position.z);
    int bottomY = floor(player.position.y);

    int width = 30;
    int length = 30;
    int height = 8;

    clear_area(width, length, height);

    int offsetX = -width / 2;
    int offsetZ = -length / 2;



    // --- Floor ---
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < length; z++) {
            set_block({centerX + offsetX + x, bottomY - 1, centerZ + offsetZ + z}, BlockID::Stone);
        }
    }

    // --- Front wall ---
    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width; x++) {
            set_block({centerX + offsetX + x, bottomY + y, centerZ + offsetZ}, BlockID::Stone);
        }
    }

    // --- Back wall ---
    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width; x++) {
            set_block({centerX + offsetX + x, bottomY + y, centerZ + offsetZ + length - 1}, BlockID::Stone);
        }
    }

    // --- Left wall ---
    for (int y = 0; y < height - 1; y++) {
        for (int z = 0; z < length; z++) {
            set_block({centerX + offsetX, bottomY + y, centerZ + offsetZ + z}, BlockID::Stone);
        }
    }

    // --- Right wall ---
    for (int y = 0; y < height - 1; y++) {
        for (int z = 0; z < length; z++) {
            set_block({centerX + offsetX + width - 1, bottomY + y, centerZ + offsetZ + z}, BlockID::Stone);
        }
    }

    // --- Roof ---
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < length; z++) {
            set_block({centerX + offsetX + x, bottomY + height - 1, centerZ + offsetZ + z}, BlockID::Stone);
        }
    }

    // --- Doorway (front wall, centered) ---
    int doorX = centerX + offsetX + width / 2;
    for (int y = 0; y < 2; y++) {
        set_block({doorX, bottomY + y, centerZ + offsetZ}, BlockID::Air);
    }
}

void spawn_player() {
    PlayerObject* pPlayer = arena_allocate<PlayerObject>(
        engineArena, engineArena, shader, texture_atlas);

    pPlayer->position = player.position;
    pPlayer->rotation = player.rotation;

    player_characters.emplace_back(pPlayer);
}

void game_logic() {
    ImGuiIO& io = ImGui::GetIO();
    player_wasd_velocity.target = glm::vec3(0.0f);

    // --- INPUT LOCK HANDLING ---
    const bool block_mouse     = io.WantCaptureMouse  && !mouse_captured;
    const bool block_keyboard  = io.WantCaptureKeyboard && !mouse_captured;

    does_imgui_have_keyboard = block_keyboard;

    // if your engine uses mouse_captured to toggle camera control:
    io.WantCaptureMouse    = !mouse_captured;
    io.WantCaptureKeyboard = !mouse_captured;

    // --- INPUT WRAPPERS ---
    auto KeyHeld = [&](int key) -> bool {
        return (!block_keyboard) && Input::IsKeyHeld(key);
    };
    auto KeyPressed = [&](int key) -> bool {
        return (!block_keyboard) && Input::IsKeyPressed(key);
    };
    auto MouseHeld = [&](int button) -> bool {
        return (!block_mouse) && Input::IsMouseHeld(button);
    };
    auto MousePressed = [&](int button) -> bool {
        return (!block_mouse) && Input::IsMousePressed(button);
    };

    // --- BASIC INPUT STATES ---
    bool left_click  = MouseHeld(GLFW_MOUSE_BUTTON_1);
    bool right_click = MouseHeld(GLFW_MOUSE_BUTTON_2);

    const bool sprint_key = KeyHeld(GLFW_KEY_LEFT_CONTROL);
    const bool jump_key   = KeyHeld(GLFW_KEY_SPACE);
    const bool down_key   = KeyHeld(GLFW_KEY_LEFT_SHIFT);


    // --- TOGGLES ---
    if (KeyPressed(GLFW_KEY_Q)) {
        set_mouse_captured();
    }
    if (KeyPressed(GLFW_KEY_F)) {
        fly_mode = !fly_mode;
        player.gravity_velocity = glm::vec3(0.0f);
        player.jump_velocity = glm::vec3(0.0f);
    }

    if (KeyPressed(GLFW_KEY_H))  render_block_highlight = !render_block_highlight;
    if (KeyPressed(GLFW_KEY_F5)) { reload_game(); return; }
    if (KeyHeld(GLFW_KEY_Y)) teleport(0, 50, 0);
    if (KeyHeld(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(win, true);

    if (KeyPressed(GLFW_KEY_G)) create_house();
    if (KeyPressed(GLFW_KEY_B)) clear_area(400, 400, 50);
    if (KeyPressed(GLFW_KEY_J)) build_tower(150, 10);
    if (KeyPressed(GLFW_KEY_V)) camera.lighting_shader_config.vsync = !camera.lighting_shader_config.vsync;

    if (KeyPressed(GLFW_KEY_T)) chat_state.isOpen = !chat_state.isOpen;

    if (KeyPressed(GLFW_KEY_P)) {
        spawn_player();
    }

    if (KeyPressed(GLFW_KEY_J)) {
        render_self_player_object = !render_self_player_object;
    }

    // --- CAMERA FOV ---
    camera_zoom.target = KeyHeld(GLFW_KEY_C) ? zoomed_fov : normal_fov;

    // --- MOVEMENT SPEED ---
    wasd_movement_speed.target = sprint_key ? sprint_speed : walk_speed;

    // --- RAYCASTING / BLOCK ACTIONS ---
    {
        static bool last_raycast_had_value = false;
        bool raycast_has_value = camera_block_raycast_hit.has_value();

        if (raycast_has_value && !last_raycast_had_value)
            block_selection_pos.value = block_selection_pos.target; // instant snap

        if (raycast_has_value && !io.WantCaptureMouse) {
            // use IsMousePressed here instead of IsMouseHeld, this will
            // prevent a block being placed every frame while the mouse is
            // held.

            left_click = MousePressed(GLFW_MOUSE_BUTTON_1);
            right_click = MousePressed(GLFW_MOUSE_BUTTON_2);





            if (right_click) place_block(BlockID::Stone);
            else if (left_click) break_block(BlockID::Air);
        }

        last_raycast_had_value = raycast_has_value;
    }

    // --- CAMERA ROTATION ---
    glm::vec2 delta = Input::GetMouseDelta();
    if (KeyHeld(GLFW_KEY_R)) {
        // Adjust sun direction with mouse
        apply_mouse_delta_to_rotation(
            delta, camera.lighting_shader_config.sunDir,
            camera_sensitivity, deltaTime, false, true);

    } else if ((mouse_captured || right_click) && (delta.x != 0 || delta.y != 0)) {
        // if (fly_mode)
        //     apply_mouse_delta_to_rotation(delta, camera.rotation, camera_sensitivity, deltaTime);
        // else
        apply_mouse_delta_to_rotation(delta, player.rotation, camera_sensitivity, deltaTime);
    }

    // --- MOVEMENT (WASD + JUMP + GRAVITY) ---
    {
        glm::vec3 move_dir(0.0f);
        const bool KEY_W = KeyHeld(GLFW_KEY_W);
        const bool KEY_S = KeyHeld(GLFW_KEY_S);
        const bool KEY_A = KeyHeld(GLFW_KEY_A);
        const bool KEY_D = KeyHeld(GLFW_KEY_D);

        if (KEY_W) move_dir += player.getFront();
        if (KEY_S) move_dir -= player.getFront();
        if (KEY_D) move_dir += player.getRight();
        if (KEY_A) move_dir -= player.getRight();

        move_dir.y = 0;
        if (glm::length(move_dir) > 0.001f)
            move_dir = glm::normalize(move_dir);

        // Handle vertical flight (free-cam) or jump (normal)
        if (fly_mode) {
            if (jump_key) move_dir.y += 1.0f;
            if (down_key) move_dir.y -= 1.0f;
        }

        player_wasd_velocity.target += move_dir;
        player.is_sprinting = glm::length(move_dir) > 0 && sprint_key;
    }

    // --- PLAYER / CAMERA MOVEMENT ---
    {
        glm::vec3 offset = player_wasd_velocity.value * wasd_movement_speed.value * deltaTime;

        if (!fly_mode) {
            player.jumpTick(world, jump_key, deltaTime);
            player.gravityTick(world, deltaTime);

            offset += player.gravity_velocity * deltaTime;
            offset += glm::vec3(0,player.jump_velocity.y,0) * deltaTime;

            // Jumping (grounded)

            if (jump_key && !player.is_on_ground) {

            }

            glm::vec3 desiredPos = player.position + offset;
            // player.position = desiredPos;
            player.position = player.tryMoveWithSlide(world, desiredPos, offset, 1.0f);

            glm::vec3 delta = desiredPos - player.position;
            if (glm::length(delta) > 0.001f)
                player_wasd_velocity.target -= delta * 50.0f;
        } else {
            // Free camera flies smoothly
            player.position += offset;
        }
    }

    // --- CHUNK MANAGEMENT ---
    chunk_updater_tick(render_distance_radius);

    // --- SYNC CAMERA ---
    // if (!fly_mode)
    player.useCamera(camera, !KeyHeld(GLFW_KEY_U), third_person);

    // camera.lookAt(player.get_player_eye_pos());


}


void ease_tick() {
    wasd_movement_speed.tick(deltaTime, 5.0f);
    player_wasd_velocity.tick(deltaTime, 5.0f);
    camera_zoom.tick(deltaTime, 20.0f);
    block_selection_pos.tick(deltaTime, 20.0f);



    camera.fov = camera_zoom.value;
}

int init_glfw() {
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);



    win = glfwCreateWindow(w, h, "Codecraft", nullptr, nullptr);
    if (!win) return 1;



    glfwMakeContextCurrent(win);
    glfwSwapInterval(camera.lighting_shader_config.vsync);

    if (!gladLoadGL(glfwGetProcAddress)) return 1;

    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);


    hwinfo.GLSLVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    hwinfo.OpenGLVersion = (const char*)glGetString(GL_VERSION);
    hwinfo.GraphicsHardware = (const char*)glGetString(GL_RENDERER);
    hwinfo.GraphicsVendor = (const char*)glGetString(GL_VENDOR);

    std::cout << "OpenGL version: " << hwinfo.OpenGLVersion << std::endl;
    std::cout << "GLSL version: "   << hwinfo.GLSLVersion << std::endl;
    std::cout << "Renderer: "       << hwinfo.GraphicsHardware << std::endl;
    std::cout << "Vendor: "         << hwinfo.GraphicsVendor << std::endl;

    int samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    std::cout << "MSAA samples: " << samples << std::endl;

    return 0;
}

bool verify_pointers_before_frame() {
    auto check = [](auto ptr, const char* name) {
        if (!ptr) {
            printf("[Engine Error] ❌ %s is nullptr\n", name);
            return false;
        }
        return true;
    };

    bool ok = true;
    ok &= check(shader, "shader");
    ok &= check(world_renderer, "world_renderer");
    ok &= check(texture_atlas, "texture_atlas");
    ok &= check(sceneFBO, "sceneFBO");
    ok &= check(blur_object, "blur_object");
    ok &= check(shadow_depth_map, "shadow_depth_map");
    ok &= check(block_highlight_shader, "block_highlight_shader");
    ok &= check(block_highlight_object, "block_highlight_object");
    ok &= check(player_object, "player_object");
    ok &= check(player_shader, "player_shader");
    ok &= check(crosshair_texture, "crosshair_texture");
    ok &= check(depth_shader, "depth_shader");
    return ok;
}



int main() {

    BlockRegistry::Init();

    std::cout << "Running from " << std::filesystem::current_path() << std::endl;

    if (int code = init_glfw()) {
        return code;
    }



    init_imgui(win);
    Input::Init(win);

    initialise();

    // update mouse capture with initial bool value
    set_mouse_captured();

    std::cout << "atlas id = " << texture_atlas->id << std::endl;

    while (!glfwWindowShouldClose(win)) {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // --- FRAME START ---
        glfwPollEvents();
        Input::Update();

        glfwSwapInterval(camera.lighting_shader_config.vsync);

        if (!verify_pointers_before_frame()) {
            return 1;
        }

        // window + camera
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);
        camera.aspect = float(w) / float(h);

        if (!camera.lighting_shader_config.bypassPostProcessing) {
            // resize offscreen buffers
            blur_object->updateSize(w, h);
            sceneFBO->resize(w, h);
            // shadow_depth_map->Resize(w, h);
        }


        // ---- game logic / input / physics
        game_logic();

        // -------------------
        // PASS 1: SHADOW MAP (depth only)
        // -------------------
        if (!camera.lighting_shader_config.bypassPostProcessing) {
            glm::vec3 sunDirVec = camera.lighting_shader_config.sunDir;
            glm::mat4 lightSpace = shadow_depth_map->GetLightSpaceMatrix(sunDirVec, camera);


            shadow_depth_map->BeginDepthPass(lightSpace);
            world_renderer->draw_depth_only(lightSpace, depth_shader);
            shadow_depth_map->EndDepthPass();


            // -------------------
            // PASS 2: SCENE (into sceneFBO)  ← this is the ONLY world draw to color
            // -------------------
            sceneFBO->bind();
        }

        // sky color
        glm::vec3 daylight  = {0.53f, 0.81f, 0.92f};
        glm::vec3 nightlight= {0, 0, 0};
        glm::vec3 sky_light = daylight * camera.lighting_shader_config.sky_night_day_light_modifier
                            + nightlight * (1.0f - camera.lighting_shader_config.sky_night_day_light_modifier);

        camera.lighting_shader_config.fogColor = sky_light;

        glClearColor(sky_light.x, sky_light.y, sky_light.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);


        // draw world ONCE
        camera.setScreenSize(w,h);
        if (render_world) world_renderer->draw(camera, shadow_depth_map);

        // au_falcon_obj->rotation = glm::vec3(-90, 0, 0);
        //
        // au_falcon_obj->scale = glm::vec3(0.1f);
        // au_falcon_obj->draw(camera, shadow_depth_map);

        player_object->position = player.position;
        player_object->rotation = player.rotation;
        if (render_self_player_object) {
            player_object->draw(camera, shadow_depth_map);
        }

        for (auto& pl : player_characters) {
            pl->draw(camera, shadow_depth_map);
        }

        if (!camera.lighting_shader_config.bypassPostProcessing) {
            sceneFBO->unbind();
        }

        // -------------------
        // PASS 3: BLUR the scene texture (optional, for your highlight)
        // -------------------
        if (!camera.lighting_shader_config.bypassPostProcessing) {
            unsigned int blurredTex = blur_object->Apply(
                sceneFBO->getTexture(), 20);

            blur_texture = blurredTex;



            // -------------------
            // PASS 4: PRESENT sceneFBO to the screen
            // -------------------
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, w, h);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // EITHER: blit (fast)

            sceneFBO->blitToDefault(w, h);   // implement via glBlitFramebuffer
            // OR draw a fullscreen quad with a pass-through shader sampling sceneFBO->getTexture()
        }

        // -------------------
        // PASS 5: OVERLAYS (highlight uses blurredTex), then UI
        // -------------------
        auto blockOpt = world->raycastBlock(camera.position, glm::normalize(camera.getFront()), 5);
        camera_block_raycast_hit = blockOpt;

        if (blockOpt && render_block_highlight) {
            block_highlight_shader->use();
            block_highlight_shader->setInt("blurredScene", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, blur_texture);
            block_selection_pos.target = blockOpt->voxel;

            block_highlight_object->Draw(blockOpt.value(), camera, {1,1,1,1});
        }

        // printf("%u\n", blurredTex);

        render_2d();

        discord_rpc.update(
            world_renderer->visibleChunks.size(),
            static_cast<int>(ImGui::GetIO().Framerate),
            camera.lighting_shader_config.vsync
        );

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        render_imgui();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // --- FRAME END ---
        glfwSwapBuffers(win);

        ease_tick();
    }


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(win);
    glfwTerminate();

    return 0;
}

