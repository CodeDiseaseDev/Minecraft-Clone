//
// Created by code on 10/12/25.
//

#include "DiscordRPC.h"

void DiscordRPC::setup() const {
  discord::RPCManager::get()
      .setClientID(APPLICATION_ID)
      .onReady([](discord::User const& user) {
        printf("rpc connected to user '%s'\n",
               user.username.c_str());
      })
      .initialize();
}

void DiscordRPC::update(size_t chunksLoaded, int fps, bool vsync) {
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
}

DiscordRPC::~DiscordRPC() {
  discord::RPCManager::get().shutdown();
}
