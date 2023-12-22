#pragma once

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <string>

#include <vulkan_renderer.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

namespace GameEngine {

static const int WIDTH = 1280;
static const int HEIGHT = 720;

class Game {
public:
  bool isRunning;

  std::chrono::time_point<std::chrono::high_resolution_clock> mLastTimestamp;
  float mDeltaTime;
  float mMoveSpeed = 0.1f;
  float mLookSpeed = 5.0f;
  // These are chosen to match the default rotation == (0, 0, -1)
  float mYaw = 0.0f; // because cos(0) == 1, we don't want positive z direction, we want sin(-90) == -1 so rotation.z == -1
  float mPitch = 0.0f;

  glm::mat4 mCameraRotation;

  SDL_Window *mWindow;
  VulkanEngine::VulkanRenderer *mVulkanRenderer;

  bool mIsCameraMoving = false;
  int32_t mMouseXStart;
  int32_t mMouseYStart;
  Game();
  ~Game();

  void run();

  std::string getEvent();
  void updateRotation();
  glm::mat4 calculateViewMatrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp);
  glm::mat4 calculateViewMatrixQuat(glm::vec3 position); 
};
} // namespace GameEngine
