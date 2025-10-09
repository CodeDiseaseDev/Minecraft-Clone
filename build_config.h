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

namespace build_config {
  constexpr std::string game_files_directory =
    DEV_GAME_FILES_DIR;

  const std::filesystem::path shaders_dir =
        std::filesystem::current_path() /
        game_files_directory /
        "shaders";

  const std::filesystem::path textures_dir =
        std::filesystem::current_path() /
        game_files_directory /
        "textures";
}

#endif //BUILD_CONFIG_H
