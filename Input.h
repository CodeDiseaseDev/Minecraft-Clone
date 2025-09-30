#pragma once
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

class Input {
public:
  // Call this once after creating the GLFWwindow
  static void Init(GLFWwindow* window);

  // Call this once per frame (after glfwPollEvents)
  static void Update();

  // Keyboard
  static bool IsKeyHeld(int key);
  static bool IsKeyPressed(int key);

  // Mouse buttons
  static bool IsMouseHeld(int button);
  static bool IsMousePressed(int button);

  // Mouse movement
  static glm::vec2 GetMousePosition();
  static glm::vec2 GetMouseDelta();

  static void CaptureMouse(bool should_capture = true);

private:
  static inline GLFWwindow* s_Window = nullptr;

  static inline glm::vec2 s_MousePos {0.0f, 0.0f};
  static inline glm::vec2 s_MousePrevPos {0.0f, 0.0f};
  static inline glm::vec2 s_MouseDelta {0.0f, 0.0f};
};
