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
  void setup() const;

  void update(size_t chunksLoaded, int fps, bool vsync);

  ~DiscordRPC();
};



#endif //DISCORDRPC_H
