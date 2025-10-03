#include <cstdio>
#include <cstdlib>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Input.h"
#include "Shader.h"
#include "Mesh.h"
#include "Objects/MeshObject.h"

#include <filesystem>
#include <fstream>
#include <iostream>

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
#include "Objects/BlockHighlightObject.h"
#include "Objects/ChunkObject.h"
#include "Objects/CrosshairObject.h"
#include "Objects/GaussianBlurObject.h"
#include "Objects/PlayerObject.h"
#include "Objects/ShadowMap.h"

// std::unique_ptr<MeshObject> mesh_object;
// std::unique_ptr<ChunkObject> chunk_object;

World world = World(0);
Player player;

float frames_per_second = 0;

Camera camera(16/9);

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;



std::optional<RaycastHit> camera_block_raycast_hit;

EaseVec3 player_wasd_velocity {};
glm::vec3 player_jump_velocity {};
EaseVec3 block_selection_pos {};
EaseValue camera_zoom{camera.fov, camera.fov};

// aspect ratio gets updated every frame


float normal_fov = 120;
float zoomed_fov = 40;

float camera_sensitivity = 25.0f;
bool mouse_captured = false;

int render_distance_radius = 1;

bool third_person = false;

bool render_world = true;
bool render_block_highlight = true;

bool free_cam = false;

int w = 600,
    h = 600;


float walk_speed = 4.0f;
float sprint_speed = walk_speed * 2.0f;
float jump_height = 10.0f;
EaseValue wasd_movement_speed {walk_speed, sprint_speed};

std::shared_ptr<Shader> shader;
std::shared_ptr<Shader> block_highlight_shader;
std::shared_ptr<WorldRenderer> world_renderer;

std::shared_ptr<BlockHighlightObject> block_highlight_object;

std::shared_ptr<Texture> texture_atlas;


std::shared_ptr<Shader> crosshair_shader;

std::shared_ptr<CrosshairObject> crosshair_object;
std::shared_ptr<Texture> crosshair_texture;

std::shared_ptr<GaussianBlurObject> blur_object;
std::shared_ptr<Shader> horizontal_blur_shader;
std::shared_ptr<Shader> vertical_blur_shader;
std::shared_ptr<SceneFramebuffer> sceneFBO;

std::shared_ptr<Shader> depth_shader;
std::shared_ptr<ShadowMap> shadow_depth_map;

std::shared_ptr<Shader> player_shader;
std::shared_ptr<PlayerObject> player_object;

GLFWwindow* win = nullptr;

glm::vec3 determine_rand_spawn_location() {
    const float rnd_max = (float)RAND_MAX;
    float x = rand() / rnd_max * 2000;
    float z = rand() / rnd_max * 2000;
    // float y = find_world_land_y_at(glm::vec2(x, z));

    return glm::vec3(x, 0, z);
}

float find_world_land_y_at(glm::vec2 pos) {
    float y = 50;

    while (world.getBlockAt(pos.x, y, pos.y).isAir()) {
        y--;
    }

    return y;
}

void use_initial_values() {
    normal_fov = 120;
    zoomed_fov = 40;

    camera_sensitivity = 40.0f;
    // mouse_captured = false;

    render_distance_radius = 1;

    third_person = false;

    mouse_captured = false;

    render_world = true;
    render_block_highlight = true;

    w = 600;
    h = 600;

    frames_per_second = 0;

    camera = Camera(16/9);

    world = World(0);
    player = Player();

    player.position = determine_rand_spawn_location();
    player.position.y = find_world_land_y_at(
        glm::vec2(player.position.x, player.position.z)) + 1;

    camera.fov = normal_fov;
}

void load_blur_shaders() {
    std::ifstream hor, ver, vert;

    std::filesystem::path shaders_dir = std::filesystem::current_path() / "shaders";

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

    horizontal_blur_shader = std::make_shared<Shader>(
        std::vector<std::pair<const char*, GLenum>>{
            {vertSource.c_str(), GL_VERTEX_SHADER},
            {horFragSource.c_str(), GL_FRAGMENT_SHADER},
        },
        "gaussian_blur_horizontal"
    );

    vertical_blur_shader = std::make_shared<Shader>(
        std::vector<std::pair<const char*, GLenum>>{
            {vertSource.c_str(), GL_VERTEX_SHADER},
            {verFragSource.c_str(), GL_FRAGMENT_SHADER},
        },
        "gaussian_blur_vertical"
    );

    blur_object = std::make_shared<GaussianBlurObject>(
        vertical_blur_shader,
        horizontal_blur_shader, w,h);

    sceneFBO = std::make_shared<SceneFramebuffer>(
        w,h);


    depth_shader = Shader::loadFromName("depth");


    player_shader = Shader::loadFromName("player");
    player_object = std::make_shared<PlayerObject>(
            player_shader, texture_atlas);
}

void initialise() {
    // auto meshOpt = Mesh::loadOBJ("/home/code/Documents/models/RTX 3090.obj");
    // if (!meshOpt) {
    //     std::cerr << "Failed to load OBJ\n";
    //     return;
    // }

    // auto mesh = std::make_unique<Mesh>(std::move(*meshOpt));

    // automatically loads mesh_object.vert & mesh_object.frag as a unique_ptr
    shader = Shader::loadFromName("mesh_object");
    block_highlight_shader = Shader::loadFromName("block_highlight");

    shadow_depth_map = std::make_shared<ShadowMap>(
        depth_shader, 2048 * 6);

    texture_atlas = std::make_shared<Texture>();
    texture_atlas->load("./textures/atlas3.png");




    load_blur_shaders();

    {
        crosshair_shader = Shader::loadFromName("crosshair");

        crosshair_texture = std::make_shared<Texture>();
        crosshair_texture->load("./textures/crosshair.png");

        crosshair_object = std::make_shared<CrosshairObject>(
            crosshair_shader, crosshair_texture);
    }

    // mesh_object = std::make_unique<MeshObject>(
    //     std::move(mesh), shader);



    block_highlight_object = std::make_unique<BlockHighlightObject>(
        block_highlight_shader);

    world_renderer = std::make_unique<WorldRenderer>(
        shader, texture_atlas, world);


    use_initial_values();
}





void setup_imgui_theme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Base colors
    colors[ImGuiCol_WindowBg]         = ImVec4(0.12f, 0.13f, 0.15f, 0.85f);
    colors[ImGuiCol_Header]           = ImVec4(0.20f, 0.22f, 0.27f, 1.0f);
    colors[ImGuiCol_HeaderHovered]    = ImVec4(0.26f, 0.29f, 0.35f, 1.0f);
    colors[ImGuiCol_HeaderActive]     = ImVec4(0.30f, 0.34f, 0.41f, 1.0f);

    colors[ImGuiCol_Button]           = ImVec4(0.17f, 0.19f, 0.23f, 1.0f);
    colors[ImGuiCol_ButtonHovered]    = ImVec4(0.26f, 0.29f, 0.35f, 1.0f);
    colors[ImGuiCol_ButtonActive]     = ImVec4(0.30f, 0.34f, 0.41f, 1.0f);

    colors[ImGuiCol_FrameBg]          = ImVec4(0.16f, 0.17f, 0.20f, 1.0f);
    colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.20f, 0.22f, 0.27f, 1.0f);
    colors[ImGuiCol_FrameBgActive]    = ImVec4(0.25f, 0.27f, 0.33f, 1.0f);

    colors[ImGuiCol_TitleBg]          = ImVec4(0.09f, 0.09f, 0.10f, 1.0f);
    colors[ImGuiCol_TitleBgActive]    = ImVec4(0.16f, 0.18f, 0.21f, 1.0f);

    colors[ImGuiCol_CheckMark]        = ImVec4(0.20f, 0.70f, 0.40f, 1.0f); // green accent
    colors[ImGuiCol_SliderGrab]       = ImVec4(0.20f, 0.70f, 0.40f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.18f, 0.85f, 0.48f, 1.0f);

    colors[ImGuiCol_Separator]        = ImVec4(0.20f, 0.22f, 0.27f, 1.0f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.70f, 0.45f, 1.0f);
    colors[ImGuiCol_SeparatorActive]  = ImVec4(0.20f, 0.85f, 0.55f, 1.0f);

    // Rounding
    style.WindowRounding    = 6.0f;
    style.FrameRounding     = 4.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding      = 4.0f;

    // Padding & spacing
    style.WindowPadding     = ImVec2(12, 8);
    style.FramePadding      = ImVec2(6, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.ScrollbarSize     = 14.0f;

    style.AntiAliasedFill  = true;
    style.AntiAliasedLines = true;
}

void chunk_updater_tick(int radius) {
    std::vector<std::shared_ptr<Chunk>> updated = world.ensureChunkAndNeighbors(
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

void render_imgui() {
    ImGui::SetNextWindowSize(ImVec2(500,350), ImGuiCond_Once);
    ImGui::Begin("Debug Menu");

    if (mouse_captured) {
        ImGui::Text("[ Mouse Captured ]");
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
    }

    {
        ImGui::Text("XYZ: %.2f, %.2f, %.2f",
            player.position.x,
            player.position.y,
            player.position.z);
        ImGui::Text("Player Rotation: %.2f, %.2f, %.2f",
            player.rotation.x,
            player.rotation.y,
            player.rotation.z);
        ImGui::Text("Sun Rotation: %.2f, %.2f, %.2f",
            camera.lighting_shader_config.sunDir.x,
            camera.lighting_shader_config.sunDir.y,
            camera.lighting_shader_config.sunDir.z);
    }

    static bool show_camera = false;
    if (!show_camera) {
        if (ImGui::Button("Show camera relative XYZ")) {
            show_camera = true;
        }
    }
    else {
        ImGui::Text("Camera Relative XYZ: %.2f, %.2f, %.2f",
            camera.position.x - player.position.x,
            camera.position.y - player.position.y,
            camera.position.z - player.position.z);
    }

    {
        int chunks = 0;
        for (auto& chunk : world.chunkColumns)
            chunks += 1;

        ImGui::Text("Chunks Loaded: %i", chunks);
        ImGui::SameLine();
        ImGui::Text("FPS: %.2f", frames_per_second);
        ImGui::Spacing();
    }

    {
        ImGui::Text("Window Size %i,%i", w, h);
        ImGui::Spacing();
    }

    ImGui::Spacing();
    {
        ImGui::Text("** Controls **");
        ImGui::SameLine();
        static bool enabled = false;
        ImGui::Checkbox("Show Controls", &enabled);

        if (enabled) {
            // ImGui::BeginChild("ControlsBox");

            ImGui::Spacing();
            ImGui::Text("Toggle Mouse Capture: Q");
            ImGui::Text("Movement: W,A,S,D");
            ImGui::Text("Jump (Hold): Space");
            ImGui::Text("Zoom: C");
            // ImGui::EndChild();
        }
    }

    ImGui::Spacing();
    {

        // ImGui::BeginChild("Variables");
        ImGui::Text("** Variables **");
        ImGui::SameLine();
        static bool enabled = false;
        ImGui::Checkbox("Enable Variables Config", &enabled);

        if (enabled) {
            ImGui::Spacing();
            ImGui::SliderFloat("FOV", &normal_fov, zoomed_fov, 160);
            ImGui::SliderFloat("Camera Sensitivity", &camera_sensitivity, 5, 80);
            ImGui::SliderFloat("Walk Speed", &walk_speed, 1, 30);
            ImGui::SliderFloat("Sprint Speed", &sprint_speed, 1, 60);
            ImGui::SliderFloat("Jump Height", &jump_height, 1, 80);
            ImGui::SliderInt("Render Distance (Radius)", &render_distance_radius, 0, 20);

            ImGui::Spacing();
            ImGui::Checkbox("Capture Mouse? (Q)", &mouse_captured);
            ImGui::SameLine();
            ImGui::Checkbox("3rd-person? (NOT IMPLEMENTED)", &third_person);

            ImGui::Checkbox("Chunk Renderer Enabled?", &render_world);
            ImGui::SameLine();
            ImGui::Checkbox("Block Highlight", &render_block_highlight);

            ImGui::Spacing();
            ImGui::Checkbox("Free Cam", &free_cam);

            // ImGui::EndChild();
        }
    }

    ImGui::Spacing();
    {
        ImGui::Text("** Lighting Shader **");
        ImGui::SameLine();
        static bool enabled = false;
        ImGui::Checkbox("Enable Shader Config", &enabled);

        if (enabled) {
            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::SliderFloat("specularStrength", &camera.lighting_shader_config.specularStrength, 0, 2);
            ImGui::SliderFloat("ambientStrength", &camera.lighting_shader_config.ambientStrength, 0, 2);
            ImGui::SliderFloat("diffuseStrength", &camera.lighting_shader_config.diffuseStrength, 0, 2);
            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::SliderFloat("shininess", &camera.lighting_shader_config.shininess, 0, 200);
            ImGui::SliderFloat("contrast", &camera.lighting_shader_config.contrast, 0, 2);
            ImGui::SliderFloat("vibrancy", &camera.lighting_shader_config.vibrancy, 0, 2);


            ImGui::SliderFloat("sky_night_day_light_modifier", &camera.lighting_shader_config.sky_night_day_light_modifier, 0, 1);
            ImGui::Spacing();
            ImGui::Spacing();

            // ImGui::ColorPicker3("fogColor", (float*)&camera.lighting_shader_config.fogColor);

            // ImGui::SliderFloat("lighting_config::fogColor", &camera.lighting_shader_config.fogColor, 0, 200);
            ImGui::SliderFloat("fogStart", &camera.lighting_shader_config.fogStart, 0, 150);
            ImGui::SliderFloat("fogEnd", &camera.lighting_shader_config.fogEnd, 0, 150);
            ImGui::SliderFloat("fogDensity", &camera.lighting_shader_config.fogDensity, 0, 0.5f);


            ImGui::SliderInt("shadowQuality (pcfRadius)", &camera.lighting_shader_config.pcfRadius, 0, 8);
        }

    }

    ImGui::Spacing();
    {

        // ImGui::BeginChild("Idk");
        ImGui::Text("** Misc **");
        ImGui::SameLine();
        static bool enabled = false;
        ImGui::Checkbox("Show Misc Config", &enabled);

        if (enabled) {
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
                player.position = glm::vec3(x, y, z);
                player.rotation = glm::vec3(0,0,0);
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
                world.chunkColumns.clear();
                world_renderer->visibleChunks.clear();
            }
            ImGui::SameLine();


            if (ImGui::Button("Reload entire game")) {
                initialise();

                world.chunkColumns.clear();
                world_renderer->visibleChunks.clear();
            }
        }
    }


    ImGui::End();
}


void apply_mouse_delta_to_rotation(glm::vec2 delta, glm::vec3& rotation, float sensitivity, float deltaTime, bool clamp = true) {
    rotation.y -= delta.y * sensitivity * deltaTime;
    rotation.x += delta.x * sensitivity * deltaTime;

    if (clamp) {
        if (rotation.y > 89.0f) rotation.y = 89.0f;
        if (rotation.y < -89.0f) rotation.y = -89.0f;
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

    world.setBlockAt(
        x, y, z,
        block
    );

    if (auto chunkPtr = world.getChunkPtrAt(x, y, z)) {
        // rebuild chunk next frame
        // printf("0x%x\n", chunkPtr.get());
        world_renderer->chunksToRebuild
            .emplace_back(chunkPtr);
    }
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

void game_logic() {
    ImGuiIO& io = ImGui::GetIO();

    bool left_click = false,
         right_click = false;

    if (!io.WantCaptureMouse) {
        left_click = Input::IsMousePressed(GLFW_MOUSE_BUTTON_1);
        right_click = Input::IsMousePressed(GLFW_MOUSE_BUTTON_2);
    }

    wasd_movement_speed.target = walk_speed;
    if (Input::IsKeyHeld(GLFW_KEY_LEFT_CONTROL)) {
      wasd_movement_speed.target = sprint_speed;
    }
    if (Input::IsKeyHeld(GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(win, true);
    }
    camera_zoom.target = normal_fov;
    if (Input::IsKeyHeld(GLFW_KEY_C)) {
      camera_zoom.target = zoomed_fov;
    }
    if (Input::IsKeyPressed(GLFW_KEY_Q)) {
      mouse_captured = !mouse_captured;
      Input::CaptureMouse(mouse_captured);
    }

    if (camera_block_raycast_hit.has_value()) {
        if (left_click) {
            place_block(BlockID::DiamondOre);
        }
        else if (right_click) {
            break_block(BlockID::Stone);
        }
    }



    glm::vec2 delta = Input::GetMouseDelta();

    if (Input::IsKeyHeld(GLFW_KEY_R)) {
        // hold R to change sun direction
        apply_mouse_delta_to_rotation(
            delta, camera.lighting_shader_config.sunDir,
            camera_sensitivity, deltaTime, false);
    }
    else if ((mouse_captured || right_click) && (delta.x != 0 || delta.y != 0)) {
        // normal camera movement
        if (free_cam) {
            apply_mouse_delta_to_rotation(
                delta, camera.rotation,
                camera_sensitivity, deltaTime);
        }
        else {
            apply_mouse_delta_to_rotation(
                delta, player.rotation,
                camera_sensitivity, deltaTime);
        }
    }

    {
        glm::vec3 velocity = glm::vec3(0, 0, 0);
        glm::vec3 jump_velocity = glm::vec3(0, 0, 0);

        const bool KEY_W = Input::IsKeyHeld(GLFW_KEY_W),
                   KEY_S = Input::IsKeyHeld(GLFW_KEY_S),
                   KEY_A = Input::IsKeyHeld(GLFW_KEY_A),
                   KEY_D = Input::IsKeyHeld(GLFW_KEY_D);


        const bool KEY_SPACE = Input::IsKeyHeld(GLFW_KEY_SPACE),
                   KEY_SHIFT = Input::IsKeyHeld(GLFW_KEY_LEFT_SHIFT);

        int directions = 0;
        if (KEY_W || KEY_S) {
            directions++;
            if (KEY_W) velocity += camera.getFront();
            else velocity -= camera.getFront();
        }
        if (KEY_D || KEY_A) {
            directions++;
            if (KEY_D) velocity += camera.getRight();
            else velocity -= camera.getRight();
        }

        velocity.y = 0;
        if (directions > 0) {
            velocity = glm::normalize(velocity);
        }

        if (KEY_SPACE || KEY_SHIFT) { // UP & DOWN
            directions++;
            if (KEY_SPACE) jump_velocity.y += jump_height;
            else velocity.y -= 1;
        }
        player_wasd_velocity.target = velocity;
        player_jump_velocity = jump_velocity;
    }

    {
        glm::vec3 offset = glm::vec3(0, 0, 0);
        offset += player_wasd_velocity.value * wasd_movement_speed.value * deltaTime; // space / jump velocity (dont apply sprint multiplier)
        offset += player_jump_velocity * deltaTime; // Gravity velocity
        offset += player.gravity_velocity * deltaTime;

        if (free_cam) {
            camera.position += offset;
        }
        else {
            player.position += offset;
        }
    }
    chunk_updater_tick(render_distance_radius);

    if (!free_cam) {
        player.useCamera(camera, third_person);
    }


}

void ease_tick() {
    wasd_movement_speed.tick(deltaTime, 5.0f);
    player_wasd_velocity.tick(deltaTime, 10.0f);
    camera_zoom.tick(deltaTime, 20.0f);
    block_selection_pos.tick(deltaTime, 20.0f);

    player.gravityTick(world, deltaTime);

    camera.fov = camera_zoom.value;
}

int init_glfw() {
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);



    win = glfwCreateWindow(w, h, "OpenGL Rectangle", nullptr, nullptr);
    if (!win) return 1;



    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    if (!gladLoadGL(glfwGetProcAddress)) return 1;

    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_CULL_FACE);

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: "   << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Renderer: "       << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Vendor: "         << glGetString(GL_VENDOR) << std::endl;

    int samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    std::cout << "MSAA samples: " << samples << std::endl;

    return 0;
}

int main() {

    BlockRegistry::Init();

    std::cout << "Running from " << std::filesystem::current_path() << std::endl;

    if (int code = init_glfw()) {
        return code;
    }

    initialise();

    init_imgui(win);
    Input::Init(win);

    std::cout << "atlas id = " << texture_atlas->id << std::endl;

    while (!glfwWindowShouldClose(win)) {
        frames_per_second = calculate_fps();
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // --- FRAME START ---
        glfwPollEvents();
        Input::Update();



        // window + camera
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);
        camera.aspect = float(w) / float(h);

        // resize offscreen buffers
        blur_object->updateSize(w, h);
        sceneFBO->resize(w, h);
        // shadow_depth_map->Resize(w, h);

        // ---- game logic / input / physics
        game_logic();

        // -------------------
        // PASS 1: SHADOW MAP (depth only)
        // -------------------
        glm::vec3 sunDirVec = camera.lighting_shader_config.sunDir;
        glm::mat4 lightSpace = shadow_depth_map->GetLightSpaceMatrix(sunDirVec, camera.position);


        shadow_depth_map->BeginDepthPass(lightSpace);
        world_renderer->draw_depth_only(lightSpace, depth_shader);
        shadow_depth_map->EndDepthPass();


        // -------------------
        // PASS 2: SCENE (into sceneFBO)  ← this is the ONLY world draw to color
        // -------------------
        sceneFBO->bind();

        // sky color
        glm::vec3 daylight  = {0.53f, 0.81f, 0.92f};
        glm::vec3 nightlight= {0.05f, 0.07f, 0.0975f};
        glm::vec3 sky_light = daylight * camera.lighting_shader_config.sky_night_day_light_modifier
                            + nightlight * (1.0f - camera.lighting_shader_config.sky_night_day_light_modifier);

        camera.lighting_shader_config.fogColor = sky_light;

        glClearColor(sky_light.x, sky_light.y, sky_light.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // draw world ONCE
        if (render_world) world_renderer->draw(camera, shadow_depth_map);

        player_object->position = player.position;
        player_object->rotation = player.rotation;
        player_object->draw(camera, shadow_depth_map);

        sceneFBO->unbind();

        // -------------------
        // PASS 3: BLUR the scene texture (optional, for your highlight)
        // -------------------
        unsigned int blurredTex = blur_object->Apply(
            sceneFBO->getTexture(), 1);

        // -------------------
        // PASS 4: PRESENT sceneFBO to the screen
        // -------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // EITHER: blit (fast)
        sceneFBO->blitToDefault(w, h);   // implement via glBlitFramebuffer
        // OR draw a fullscreen quad with a pass-through shader sampling sceneFBO->getTexture()

        // -------------------
        // PASS 5: OVERLAYS (highlight uses blurredTex), then UI
        // -------------------
        auto blockOpt = world.raycastBlock(camera.position + 0.5f, glm::normalize(camera.getFront()), 5);
        camera_block_raycast_hit = blockOpt;

        if (blockOpt && render_block_highlight) {
            block_highlight_shader->use();
            block_highlight_shader->setInt("blurredScene", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, blurredTex);
            block_selection_pos.target = blockOpt->voxel;

            block_highlight_object->Draw(block_selection_pos.value, camera, {0.0, 0.0, 0.0, 0.2});
        }

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
}

