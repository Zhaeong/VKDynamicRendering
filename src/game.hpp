#pragma once

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <string>

#include <vulkan_renderer.hpp>

namespace GameEngine {

static const int WIDTH = 1280;
static const int HEIGHT = 720;

class Game {
public:
  bool isRunning;

  std::chrono::time_point<std::chrono::high_resolution_clock> mLastTimestamp;
  float mDeltaTime;
  float mMoveSpeed = 0.1f;
  SDL_Window *mWindow;
  VulkanEngine::VulkanRenderer *mVulkanRenderer;

  bool mIsCameraMoving = false;
  int32_t mMouseXStart;
  int32_t mMouseYStart;
  Game();
  ~Game();

  void run();

  std::string getEvent();
};
} // namespace GameEngine
