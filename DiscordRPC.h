//
// Created by code on 10/12/25.
//

#ifndef DISCORDRPC_H
#define DISCORDRPC_H
#include <cstdint>

#include "discord-rpc.hpp"
#include "cmake-build-debug-local/_deps/fmt-src/include/fmt/args.h"


class DiscordRPC {
private:
  const char* APPLICATION_ID = "1424666581950009394";
  bool SendPresence = true;

  int64_t last_update = 0;

  int64_t start_time = time(nullptr);

public:
  void setup() {
    discord::RPCManager::get()
      .setClientID(APPLICATION_ID)
      .onReady([](discord::User const& user) {
        printf("rpc connected to user %s\n",
          user.username.c_str());
      })
      .initialize();
  }

  void update(size_t chunksLoaded, int fps, bool vsync) {
    // do not update until 5 seconds elapsed
    if (time(nullptr) - last_update < 2.5f) {
      return;
    }
    last_update = time(nullptr);

    auto& rpc = discord::RPCManager::get();

    std::string state = std::to_string(chunksLoaded) + " chunks loaded!";
    std::string details = std::to_string(fps) + " FPS (" + (vsync ? "vsync" : "no vsync") + ")";

    rpc.getPresence()
      .setActivityType(discord::ActivityType::Game)
      .setState(state)
      .setDetails(details)
      .setStartTimestamp(start_time)
      .refresh();

    printf("updated\n");
  }

  ~DiscordRPC() {
    discord::RPCManager::get().shutdown();
  }
};



#endif //DISCORDRPC_H
