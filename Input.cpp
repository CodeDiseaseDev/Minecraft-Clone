#include "Input.h"

#include <unordered_map>

void Input::Init(GLFWwindow* window) {
  s_Window = window;
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  s_MousePos = {x, y};
  s_MousePrevPos = s_MousePos;
  s_MouseDelta = {0.0f, 0.0f};
}

void Input::Update() {
  s_MousePrevPos = s_MousePos;

  double x, y;
  glfwGetCursorPos(s_Window, &x, &y);
  s_MousePos = {x, y};
  s_MouseDelta = s_MousePos - s_MousePrevPos;
}

// --- Keyboard ---
bool Input::IsKeyHeld(int key) {
  return glfwGetKey(s_Window, key) == GLFW_PRESS;
}

bool Input::IsKeyPressed(int key) {
  static std::unordered_map<int, bool> keyStates;

  int state = glfwGetKey(s_Window, key);
  bool wasDown = keyStates[key];
  keyStates[key] = (state == GLFW_PRESS);

  // return true only on transition: released → pressed
  return (state == GLFW_PRESS && !wasDown);
}

// --- Mouse ---
bool Input::IsMouseHeld(int button) {
  return glfwGetMouseButton(s_Window, button) == GLFW_PRESS;
}

bool Input::IsMousePressed(int button) {
  static bool prev[8] = {false}; // supports up to 8 mouse buttons
  bool held = IsMouseHeld(button);
  bool pressed = held && !prev[button];
  prev[button] = held;
  return pressed;
}

glm::vec2 Input::GetMousePosition() {
  return s_MousePos;
}

glm::vec2 Input::GetMouseDelta() {
  return s_MouseDelta;
}

void Input::CaptureMouse(bool should_capture) {
  glfwSetInputMode(
    s_Window,
    GLFW_CURSOR,
    should_capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}
