#pragma once

#include <SDL2/SDL.h>
#include <iostream>
#include <string>

#include <vulkan_renderer.hpp>

namespace GameEngine {

static const int WIDTH = 800;
static const int HEIGHT = 600;

class Game {
public:
  bool isRunning;

  SDL_Window *window;
  VulkanEngine::VulkanRenderer *vulkanRenderer;

  Game();
  ~Game();

  void run();

  std::string getEvent();
};
} // namespace GameEngine