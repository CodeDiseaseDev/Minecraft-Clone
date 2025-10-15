//
// Created by code on 10/8/25.
//

#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

#include <string>

// #ifdef DEV_GAME_FILES_DIR
// constexpr bool ___DEV = true;
// #else
// constexpr bool ___DEV = false;
// #endif

#pragma once
#include <string>
#include <filesystem>

namespace build_config {

  // Returns the base directory for game data depending on OS
  inline std::filesystem::path getBaseDirectory() {
#ifdef _WIN32
    // Example: C:\Users\<User>\AppData\Roaming\CodeCraft
    const char* appdata = std::getenv("APPDATA");
    if (appdata)
      return std::filesystem::path(appdata) / "CodeCraft";
    else
      return std::filesystem::path(".") / "CodeCraft"; // fallback
#elif defined(__APPLE__)
    // Example: /Users/<User>/Library/Application Support/CodeCraft
    const char* home = std::getenv("HOME");
    if (home)
      return std::filesystem::path(home) / "Library" / "Application Support" / "CodeCraft";
    else
      return std::filesystem::path(".") / "CodeCraft"; // fallback
#else
    // Linux: ~/.codecraft
    const char* home = std::getenv("HOME");
    if (home)
      return std::filesystem::path(home) / ".codecraft";
    else
      return std::filesystem::path(".") / "CodeCraft"; // fallback
#endif
  }

  inline const std::filesystem::path game_files_directory = getBaseDirectory() / "game_files";

  inline const std::filesystem::path shaders_dir =
      game_files_directory / "shaders";

  inline const std::filesystem::path textures_dir =
      game_files_directory / "textures";

} // namespace build_config


#endif //BUILD_CONFIG_H

