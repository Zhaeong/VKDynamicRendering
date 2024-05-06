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

  mVulkanRenderer->mViewMatrix = calculateViewMatrixQuat(mVulkanRenderer->mCameraPos);

  // instantiate to identiy matrix
  mVulkanRenderer->mCameraRotation = glm::mat4(1.0);

  for(int i = 0; i < 2; i++){
  Utils::Model cubeModel;

  cubeModel.mVertices = mGLTFLoader->mVertices;
  cubeModel.mIndices = mGLTFLoader->mIndices;

  mVulkanRenderer->createVertexBuffer(cubeModel.mVertices, &cubeModel.mVertexBuffer, &cubeModel.mVertexBufferMemory);
  mVulkanRenderer->createIndexBuffer(cubeModel.mIndices, &cubeModel.mIndexBuffer, &cubeModel.mIndexBufferMemory);
  mVulkanRenderer->createUniformBufferForModel(&cubeModel.mUniformBuffer, &cubeModel.mUniformBuffersMemory);

  // push back create copy
  mVulkanRenderer->mModels.push_back(cubeModel);
  }
  
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
    //updateRotation();

    //mVulkanRenderer->mViewMatrix = calculateViewMatrix(mVulkanRenderer->mCameraPos,
    //                                                 mVulkanRenderer->mCameraPos + mVulkanRenderer->mCameraFront,
    //                                                 mVulkanRenderer->mCameraUp);

    mVulkanRenderer->mViewMatrix = calculateViewMatrixQuat(mVulkanRenderer->mCameraPos);


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
        mVulkanRenderer->mLightRot+=1;

        //mRoll += mLookSpeed * mDeltaTime;
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
        std::cout << "Event: " << eventName << "\n";
        glm::vec3 cameraRight = glm::vec3(mVulkanRenderer->mViewMatrix[0][0],
                                          mVulkanRenderer->mViewMatrix[1][0],
                                          mVulkanRenderer->mViewMatrix[2][0]);
        mVulkanRenderer->mCameraPos += (cameraRight * mDeltaTime * mMoveSpeed);
        break;
      }
      case SDLK_z: {
        eventName = "KEY_z";
        std::cout << "Event: " << eventName << "\n";
        break;
      }
      case SDLK_x: {
        eventName = "KEY_x";
        std::cout << "Event: " << eventName << "\n";
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

void Game::updateRotation(){
  float cosYaw = cos(glm::radians(mYaw));
  float sinYaw = sin(glm::radians(mYaw));

  float cosPitch = cos(glm::radians(mPitch));
  float sinPitch = sin(glm::radians(mPitch));

  glm::vec3 rotation = glm::vec3(0.0f);

  rotation.x = cosYaw * cosPitch;
  rotation.y = sinPitch;
  rotation.z = sinYaw * cosPitch;

  rotation = glm::normalize(rotation);
  //mVulkanRenderer->mCameraFront = rotation;

}

glm::mat4 Game::calculateViewMatrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp){
// 1. Position = known
    // 2. Calculate cameraDirection
    glm::vec3 zaxis = glm::normalize(position - target);
    // 3. Get positive right axis vector
    glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
    // 4. Calculate camera up vector
    glm::vec3 yaxis = glm::cross(zaxis, xaxis);

    // Create translation and rotation matrix
    // In glm we access elements as mat[col][row] due to column-major layout
    glm::mat4 translation = glm::mat4(1.0f); // Identity matrix by default
    translation[3][0] = -position.x; // Third column, first row
    translation[3][1] = -position.y;
    translation[3][2] = -position.z;
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0][0] = xaxis.x; // First column, first row
    rotation[1][0] = xaxis.y;
    rotation[2][0] = xaxis.z;
    rotation[0][1] = yaxis.x; // First column, second row
    rotation[1][1] = yaxis.y;
    rotation[2][1] = yaxis.z;
    rotation[0][2] = zaxis.x; // First column, third row
    rotation[1][2] = zaxis.y;
    rotation[2][2] = zaxis.z; 

    // Return lookAt matrix as combination of translation and rotation matrix
    return rotation * translation; // Remember to read from right to left (first translation then rotation)
}

glm::mat4 Game::calculateViewMatrixQuat(glm::vec3 position) {
  //FPS camera:  RotationX(pitch) * RotationY(yaw)
  glm::quat qPitch = glm::angleAxis(glm::radians(mPitch), glm::vec3(1, 0, 0));
  glm::quat qYaw = glm::angleAxis(glm::radians(mYaw), glm::vec3(0, 1, 0));
  //glm::quat qRoll = glm::angleAxis(mRoll ,glm::vec3(0,0,1));  

  //For a FPS camera we can omit roll
  glm::quat orientation = qPitch * qYaw;
  orientation = glm::normalize(orientation);
  //glm::mat4 rotate = glm::mat4_cast(orientation);
  mCameraRotation = glm::mat4_cast(orientation);

  glm::mat4 translation = glm::mat4(1.0f); // Identity matrix by default
  translation[3][0] = -position.x; // Third column, first row
  translation[3][1] = -position.y;
  translation[3][2] = -position.z;

  return mCameraRotation * translation;
}
} // namespace GameEngine
