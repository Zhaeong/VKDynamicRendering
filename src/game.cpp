#include "game.hpp"

namespace GameEngine {
Game::Game() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
  }

  mWindow = SDL_CreateWindow("SDL Vulkan Sample", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, GameEngine::WIDTH,
                            GameEngine::HEIGHT,
                            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  mVulkanRenderer = new VulkanEngine::VulkanRenderer(mWindow);

  mVulkanRenderer->mCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  mVulkanRenderer->mCameraPos = glm::vec3(0.0f, 0.0f, 2.0f);
  mVulkanRenderer->mCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

  isRunning = true;
  mLastTimestamp = std::chrono::high_resolution_clock::now();
}

Game::~Game() {
  // delete vulkanRenderer;
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
}

void Game::run() {

  while (isRunning) {

    std::string event = getEvent();
    // std::cout << "Eventer: " << event << "\n";
    float cosYaw = cos(glm::radians(mYaw));
    float sinYaw = sin(glm::radians(mYaw));

    float cosPitch = cos(glm::radians(mPitch));
    float sinPitch = sin(glm::radians(mPitch));

    glm::vec3 rotation = glm::vec3(0.0f);

    rotation.x = cosYaw * cosPitch;
    rotation.y = sinPitch;
    rotation.z = sinYaw * cosPitch;

    rotation = glm::normalize(rotation);
    mVulkanRenderer->mCameraFront = rotation;

    std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
    mDeltaTime = (float)std::chrono::duration<double, std::milli>(currentTime - mLastTimestamp).count();
    //std::cout << "timer: " << mDeltaTime << "\n";
    mLastTimestamp = std::chrono::high_resolution_clock::now();
    //float fps = 1000.0f/mDeltaTime;
    //std::cout << "fps: " << fps << "\n";

    mVulkanRenderer->drawFrame();
    //SDL_Delay(10);
    //   isRunning = false;
  }
}

std::string Game::getEvent() {
  std::string eventName = "NONE";
  // Poll for events. SDL_PollEvent() returns 0 when there are no
  // more events on the event queue, our while loop will exit when
  // that occurs.

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_KEYDOWN:

      // Check the SDLKey values and move change the coords
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        eventName = "EXIT";
        isRunning = false;
        break;
      case SDLK_LEFT: {
        eventName = "MOVE_LEFT";
        mYaw -= mLookSpeed * mDeltaTime;
        break;
      }
      case SDLK_RIGHT: {
        eventName = "MOVE_RIGHT";
        mYaw += mLookSpeed * mDeltaTime;
        break;
      }
      case SDLK_UP: {
        eventName = "MOVE_UP";
        mPitch -= mLookSpeed * mDeltaTime;
        break;
      }
      case SDLK_DOWN: {
        eventName = "MOVE_DOWN";
        mPitch += mLookSpeed * mDeltaTime;
 
        break;
      }
      case SDLK_e: {
        eventName = "KEY_E";
        std::cout << "Event: " << eventName << "\n";
        mVulkanRenderer->mCameraLookZ += 0.1f;
        break;
      }
      case SDLK_q: {
        eventName = "KEY_Q";
        std::cout << "Event: " << eventName << "\n";
        mVulkanRenderer->mCameraLookZ -= 0.1f;
        break;
      }

      case SDLK_w: {
        eventName = "KEY_W";
        std::cout << "Event: " << eventName << "\n";
        float moveSpeed = mDeltaTime * mMoveSpeed;
        std::cout << "deltaTime: " << mDeltaTime << "\n";
        std::cout << "MoveSpeed: " << moveSpeed << "\n";
        mVulkanRenderer->mCameraPos += (mVulkanRenderer->mCameraFront * mDeltaTime * mMoveSpeed);
        //vulkanRenderer->mCameraPosZ += 0.1f;

        break;
      }
      case SDLK_s: {
        eventName = "KEY_S";
        std::cout << "Event: " << eventName << "\n";
        //vulkanRenderer->mCameraPosZ -= 0.1f;
        mVulkanRenderer->mCameraPos -= mVulkanRenderer->mCameraFront * mDeltaTime * mMoveSpeed;
        break;
      }
      case SDLK_a: {
        eventName = "KEY_A";
        std::cout << "Event: " << eventName << "\n";
        // Get cross product of the front x up to get the perpendicular direction
        mVulkanRenderer->mCameraPos -= glm::normalize(glm::cross(mVulkanRenderer->mCameraFront, mVulkanRenderer->mCameraUp)) * mDeltaTime * mMoveSpeed;
        break;
      }
      case SDLK_d: {
        eventName = "KEY_D";
        std::cout << "Event: " << eventName << "\n";
        mVulkanRenderer->mCameraPos += glm::normalize(glm::cross(mVulkanRenderer->mCameraFront, mVulkanRenderer->mCameraUp)) * mDeltaTime * mMoveSpeed;
        break;
      }
      case SDLK_z: {
        eventName = "KEY_z";
        std::cout << "Event: " << eventName << "\n";
        //mVulkanRenderer->mCameraPosY -= 0.1f;
        break;
      }
      case SDLK_x: {
        eventName = "KEY_x";
        std::cout << "Event: " << eventName << "\n";
        //mVulkanRenderer->mCameraPosY += 0.1f;
        break;
      }
      default:
        eventName = "KEY_DOWN";
        break;
      }
      break;

    case SDL_KEYUP:
      eventName = "KEY_UP";
      break;

    case SDL_MOUSEBUTTONDOWN:
      eventName = "MOUSE_DOWN";
      int posX, posY;
      SDL_GetMouseState(&posX, &posY);
      std::cout << "MouseX: " << posX << " MouseY:" << posY << "\n";
      std::cout << "MouseLookX: " << mVulkanRenderer->mCameraLookX << " MouseY:" << mVulkanRenderer->mCameraLookY<< "\n";
      VulkanHelper::convertPixelToNormalizedDeviceCoord(mVulkanRenderer->mSwapChainExtent, posX, posY);
      if(event.button.button == SDL_BUTTON_LEFT) {
        std::cout << "Left mouse pressed\n";
        mIsCameraMoving = true;
        mMouseXStart = posX; 
        mMouseYStart = posY; 
      }
      break;

    case SDL_MOUSEBUTTONUP:
      eventName = "MOUSE_UP";
      if(event.button.button == SDL_BUTTON_LEFT) {
        std::cout << "Left mouse pressed\n";
        mIsCameraMoving = false;
      }
      break;

    case SDL_MOUSEMOTION:
      // std::cout << "moving da mouse\n";
      // std::cout << "MouseX: " << event.motion.x << " MouseY:" << event.motion.y << "\n";
      if(mIsCameraMoving) {
        float yawDiff = (mMouseXStart - event.motion.x) * 0.01f; 
        float pitchDiff = (mMouseYStart - event.motion.y) * 0.01f; 
        mYaw -= yawDiff;
        mPitch -= pitchDiff;
      }
      break;

    case SDL_QUIT:
      eventName = "EXIT";
      isRunning = false;
      break;
    default:
      break;
    }
  }

  // std::cout << "Event: " << eventName << "\n";

  return eventName;
}
} // namespace GameEngine
