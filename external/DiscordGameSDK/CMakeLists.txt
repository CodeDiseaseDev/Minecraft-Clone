cmake_minimum_required(VERSION 3.16)
project(DiscordGameSDK LANGUAGES CXX)

# Header-only library for includes + linking
add_library(DiscordGameSDK INTERFACE)
target_include_directories(DiscordGameSDK INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# --- Platform / architecture detection ---
if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(DISCORD_SDK_LIB "${CMAKE_CURRENT_SOURCE_DIR}/x86_64/discord_game_sdk.dll.lib")
        set(DISCORD_SDK_BIN "${CMAKE_CURRENT_SOURCE_DIR}/x86_64/discord_game_sdk.dll")
    else()
        set(DISCORD_SDK_LIB "${CMAKE_CURRENT_SOURCE_DIR}/x86/discord_game_sdk.dll.lib")
        set(DISCORD_SDK_BIN "${CMAKE_CURRENT_SOURCE_DIR}/x86/discord_game_sdk.dll")
    endif()
elseif(APPLE)
    set(DISCORD_SDK_LIB "${CMAKE_CURRENT_SOURCE_DIR}/x86_64/discord_game_sdk.dylib")
    set(DISCORD_SDK_BIN "${CMAKE_CURRENT_SOURCE_DIR}/x86_64/discord_game_sdk.dylib")
elseif(UNIX)
    set(DISCORD_SDK_LIB "${CMAKE_CURRENT_SOURCE_DIR}/x86_64/discord_game_sdk.so")
    set(DISCORD_SDK_BIN "${CMAKE_CURRENT_SOURCE_DIR}/x86_64/discord_game_sdk.so")
endif()

# Link the precompiled SDK binary
target_link_libraries(DiscordGameSDK INTERFACE "${DISCORD_SDK_LIB}")
set_property(TARGET DiscordGameSDK PROPERTY LINKER_LANGUAGE CXX)


# --- Function to copy the runtime lib for any executable ---
function(copy_discord_sdk_runtime TARGET_NAME)
    add_custom_command(
            TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DISCORD_SDK_BIN}"
            $<TARGET_FILE_DIR:${TARGET_NAME}>/
            COMMENT "Copying Discord Game SDK runtime binary for ${TARGET_NAME}"
    )
endfunction()
