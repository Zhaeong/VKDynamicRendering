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

  SDL_Window *window;
  VulkanEngine::VulkanRenderer *vulkanRenderer;

  bool isCameraMoving = false;
  int32_t mouseXStart;
  int32_t mouseYStart;

  Game();
  ~Game();

  void run();

  std::string getEvent();
};
} // namespace GameEngine
