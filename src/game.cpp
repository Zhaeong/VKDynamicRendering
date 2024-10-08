#include "game.hpp"

namespace GameEngine {
Game::Game() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
  }

  mGLTFLoader = new GLTF::GLTFLoader();


  mWindow = SDL_CreateWindow("SDL Vulkan Sample", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, GameEngine::WIDTH,
                            GameEngine::HEIGHT,
                            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  mVulkanRenderer = new VulkanEngine::VulkanRenderer(mWindow);

  mVulkanRenderer->mCameraPos = glm::vec3(0.0f, 0.0f, 2.0f);

  // instantiate to identiy matrix
  mVulkanRenderer->mCameraRotation = glm::mat4(1.0);

  mGLTFLoader->loadFile("/models/sphere/sphere.gltf");
  Utils::Model lightModel = loadModel(glm::vec3(3.0f, 3.0f, 3.0f),
                             mGLTFLoader->mVertices, 
                             mGLTFLoader->mIndices); 

  mVulkanRenderer->mModels.push_back(lightModel);
  
  mGLTFLoader->loadFile("/models/cube/cube.gltf");
  Utils::Model cubeModel = loadModel(glm::vec3(0.0f, 0.0f, 0.0f),
                             mGLTFLoader->mVertices, 
                             mGLTFLoader->mIndices); 
  mVulkanRenderer->mModels.push_back(cubeModel);

  Utils::Model secondCube = loadModel(glm::vec3(1.0f, 0.0f, 0.0f),
                             mGLTFLoader->mVertices, 
                             mGLTFLoader->mIndices);
  mVulkanRenderer->mModels.push_back(secondCube);
  
  mVulkanRenderer->beginVulkanObjectCreation();

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

    // Put into uniform buffer and view matrix to transform model coordinates in world space into camera space
    mVulkanRenderer->mViewMatrix = Utils::calculateViewMatrixQuat(mVulkanRenderer->mCameraPos, mPitch, mYaw);


    std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
    mDeltaTime = (float)std::chrono::duration<double, std::milli>(currentTime - mLastTimestamp).count();
    //std::cout << "timer: " << mDeltaTime << "\n";
    mLastTimestamp = std::chrono::high_resolution_clock::now();
    float fps = 1000.0f/mDeltaTime;
    //std::cout << "fps: " << fps << "\n";


    mVulkanRenderer->mTextOverlay->beginTextUpdate();
    mVulkanRenderer->mTextOverlay->addText("FPS: " + std::to_string(fps) + " FrameTime:" + std::to_string(mDeltaTime), 0.0f, 0.0f, TextOverlay::alignLeft);
    mVulkanRenderer->mTextOverlay->addText("Camera Pos- X:" + std::to_string(mVulkanRenderer->mCameraPos.x) + 
                        " Y:"  + std::to_string(mVulkanRenderer->mCameraPos.y) + 
                        " Z:"  + std::to_string(mVulkanRenderer->mCameraPos.z), 0.0f, 30.0f, TextOverlay::alignLeft);
                        
    mVulkanRenderer->mTextOverlay->endTextUpdate();
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
        mPitch += mLookSpeed * mDeltaTime;
        std::cout << "yaw: " << mYaw << "\n";
        std::cout << "pitch: " << mPitch << "\n";

        break;
      }
      case SDLK_DOWN: {
        eventName = "MOVE_DOWN";
        mPitch -= mLookSpeed * mDeltaTime;
 
        break;
      }
      case SDLK_e: {
        eventName = "KEY_E";
        std::cout << "Event: " << eventName << "\n";

        // rotate the light model about the y axis
        glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), glm::radians(10.0f),  glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 originalPos = mVulkanRenderer->mModels[0].mPosition;
        mVulkanRenderer->mModels[0].mPosition = glm::vec4(originalPos.x, originalPos.y, originalPos.z, 1.0f) * rot_mat;

        break;
      }
      case SDLK_q: {
        eventName = "KEY_Q";
        //mRoll -= mLookSpeed * mDeltaTime;
        std::cout << "Event: " << eventName << "\n";
        break;
      }

      case SDLK_w: {
        eventName = "KEY_W";
        std::cout << "Event: " << eventName << "\n";
        float moveSpeed = mDeltaTime * mMoveSpeed;
        std::cout << "deltaTime: " << mDeltaTime << "\n";
        std::cout << "MoveSpeed: " << moveSpeed << "\n";
        //glm is has column major matrices, so third column represents the forward vector
        //view matrix:
        //[ right_x, up_x, forward_x, position_x ]
        //[ right_y, up_y, forward_y, position_y ]
        //[ right_z, up_z, forward_z, position_z ]
        //[ 0, 0, 0, 1]
        glm::vec3 cameraFront = glm::vec3(mVulkanRenderer->mViewMatrix[0][2],
                                          mVulkanRenderer->mViewMatrix[1][2],
                                          mVulkanRenderer->mViewMatrix[2][2]); 

        //std::cout << glm::to_string(cameraFront) << "\n";
        mVulkanRenderer->mCameraPos -= (cameraFront * mDeltaTime * mMoveSpeed);

        break;
      }
      case SDLK_s: {
        eventName = "KEY_S";
        std::cout << "Event: " << eventName << "\n";
        glm::vec3 cameraFront = glm::vec3(mVulkanRenderer->mViewMatrix[0][2],
                                          mVulkanRenderer->mViewMatrix[1][2],
                                          mVulkanRenderer->mViewMatrix[2][2]);
        mVulkanRenderer->mCameraPos += (cameraFront * mDeltaTime * mMoveSpeed);
        break;
      }
      case SDLK_a: {
        eventName = "KEY_A";
        std::cout << "Event: " << eventName << "\n";
        // Previous method was to get cross product of the front, and cross product the with the up to get the perpendicular direction
        // This is kept here for posterity
        // mVulkanRenderer->mCameraPos -= glm::normalize(glm::cross(mVulkanRenderer->mCameraFront, mVulkanRenderer->mCameraUp)) * mDeltaTime * mMoveSpeed;
        glm::vec3 cameraRight = glm::vec3(mVulkanRenderer->mViewMatrix[0][0],
                                          mVulkanRenderer->mViewMatrix[1][0],
                                          mVulkanRenderer->mViewMatrix[2][0]);
        mVulkanRenderer->mCameraPos -= (cameraRight * mDeltaTime * mMoveSpeed);
 
        break;
      }
      case SDLK_d: {
        eventName = "KEY_D";
        glm::vec3 cameraRight = glm::vec3(mVulkanRenderer->mViewMatrix[0][0],
                                          mVulkanRenderer->mViewMatrix[1][0],
                                          mVulkanRenderer->mViewMatrix[2][0]);
        mVulkanRenderer->mCameraPos += (cameraRight * mDeltaTime * mMoveSpeed);
        break;
      }
      case SDLK_z: {
        eventName = "KEY_z";
        std::cout << "Event: " << eventName << "\n";
        mVulkanRenderer->mModels[0].mPosition.y +=0.1f;
        break;
      }
      case SDLK_c: {
        eventName = "KEY_c";
        std::cout << "Event: " << eventName << "\n";
        mVulkanRenderer->mModels[2].mPosition.y +=0.1f;
        break;
      }
      case SDLK_x: {
        eventName = "KEY_x";
        std::cout << "Event: " << eventName << "\n";
        mVulkanRenderer->mModels[0].mPosition.y -=0.1f;
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
        mPitch += pitchDiff;
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

Utils::Model Game::loadModel(glm::vec3                  position,
                             std::vector<Utils::Vertex> vertices, 
                             std::vector<uint32_t>      indices) {

    Utils::Model cubeModel;

    cubeModel.mPosition = position;

    cubeModel.mVertices = vertices;
    cubeModel.mIndices = indices;

    mVulkanRenderer->createVertexBuffer(cubeModel.mVertices, &cubeModel.mVertexBuffer, &cubeModel.mVertexBufferMemory);
    mVulkanRenderer->createIndexBuffer(cubeModel.mIndices, &cubeModel.mIndexBuffer, &cubeModel.mIndexBufferMemory);
    mVulkanRenderer->createUniformBufferForModel(&cubeModel.mUniformBuffer, &cubeModel.mUniformBuffersMemory);

    return cubeModel;
 
}

} // namespace GameEngine
