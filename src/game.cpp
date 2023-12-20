#include "game.hpp"

namespace GameEngine {
Game::Game() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
  }

  window = SDL_CreateWindow("SDL Vulkan Sample", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, GameEngine::WIDTH,
                            GameEngine::HEIGHT,
                            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  vulkanRenderer = new VulkanEngine::VulkanRenderer(window);

  isRunning = true;
}

Game::~Game() {
  // delete vulkanRenderer;
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void Game::run() {

  while (isRunning) {
    std::string event = getEvent();
    // std::cout << "Eventer: " << event << "\n";
    vulkanRenderer->drawFrame();

    // SDL_Delay(1000);
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
      case SDLK_LEFT:
        eventName = "MOVE_LEFT";
        vulkanRenderer->mCameraLookX += 0.1f;
        break;
      case SDLK_RIGHT:
        eventName = "MOVE_RIGHT";
        vulkanRenderer->mCameraLookX -= 0.1f;
        break;
      case SDLK_UP:
        eventName = "MOVE_UP";
        vulkanRenderer->mCameraLookY += 0.1f;
        break;
      case SDLK_DOWN:
        eventName = "MOVE_DOWN";
        vulkanRenderer->mCameraLookY -= 0.1f;
        break;
      case SDLK_e: {
        eventName = "KEY_E";
        std::cout << "Event: " << eventName << "\n";
        vulkanRenderer->mCameraLookZ += 0.1f;
        break;
      }
      case SDLK_q: {
        eventName = "KEY_Q";
        std::cout << "Event: " << eventName << "\n";
        vulkanRenderer->mCameraLookZ -= 0.1f;
        break;
      }

      case SDLK_w: {
        eventName = "KEY_W";
        std::cout << "Event: " << eventName << "\n";
        vulkanRenderer->mCameraPosZ += 0.1f;
        break;
      }
      case SDLK_s: {
        eventName = "KEY_S";
        std::cout << "Event: " << eventName << "\n";
        vulkanRenderer->mCameraPosZ -= 0.1f;
        break;
      }
      case SDLK_a: {
        eventName = "KEY_A";
        std::cout << "Event: " << eventName << "\n";
        vulkanRenderer->mCameraPosX -= 0.1f;
        break;
      }
      case SDLK_d: {
        eventName = "KEY_D";
        std::cout << "Event: " << eventName << "\n";
        vulkanRenderer->mCameraPosX += 0.1f;
        break;
      }
      case SDLK_z: {
        eventName = "KEY_z";
        std::cout << "Event: " << eventName << "\n";
        vulkanRenderer->mCameraPosY -= 0.1f;
        break;
      }
      case SDLK_x: {
        eventName = "KEY_x";
        std::cout << "Event: " << eventName << "\n";
        vulkanRenderer->mCameraPosY += 0.1f;
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
      std::cout << "MouseLookX: " << vulkanRenderer->mCameraLookX << " MouseY:" << vulkanRenderer->mCameraLookY<< "\n";
      VulkanHelper::convertPixelToNormalizedDeviceCoord(vulkanRenderer->mSwapChainExtent, posX, posY);
      if(event.button.button == SDL_BUTTON_LEFT) {
        std::cout << "Left mouse pressed\n";
        isCameraMoving = true;
        mouseXStart = posX; 
        mouseYStart = posY; 
      }
      break;

    case SDL_MOUSEBUTTONUP:
      eventName = "MOUSE_UP";
      if(event.button.button == SDL_BUTTON_LEFT) {
        std::cout << "Left mouse pressed\n";
        isCameraMoving = false;
      }
      break;

    case SDL_MOUSEMOTION:
      std::cout << "moving da mouse\n";
      std::cout << "MouseX: " << event.motion.x << " MouseY:" << event.motion.y << "\n";
      if(isCameraMoving) {
        vulkanRenderer->mCameraLookX += (mouseXStart - event.motion.x) * 0.00001f;
        vulkanRenderer->mCameraLookY -= (mouseYStart - event.motion.y) * 0.00001f;
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
