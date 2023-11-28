cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(VKGame VERSION 1.0)

# if(NOT DEFINED ENV{VULKAN_SDK})
#     message("Environment variable of VULKAN_SDK is not defined: $ENV{VULKAN_SDK}")
# else()
#     set(VULKAN_SDK  $ENV{VULKAN_SDK})
#     message("Environment variable of VULKAN_SDK is defined: $ENV{VULKAN_SDK}")
# endif()

# if (VULKAN_SDK)
#     include_directories ("${VULKAN_SDK}/include")
# endif()

# find_library(Vulkan_LIBRARY
#              NAMES vulkan vulkan-1
#              PATHS ${VULKAN_SDK}/lib )
# message("Vulkan_LIBRARY:${Vulkan_LIBRARY}")

find_package(Vulkan REQUIRED)

if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "C:/VulkanSDK/SDL2-2.28.1/x86_64-w64-mingw32/lib/cmake/SDL2")
    message("CMAKE_PREFIX_PATH:${CMAKE_PREFIX_PATH}")
ENDIF(WIN32)

include_directories(${Vulkan_INCLUDE_DIR})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

message("Project Build Dir: ${PROJECT_BINARY_DIR}")
file(COPY "${PROJECT_SOURCE_DIR}/textures" DESTINATION "${PROJECT_BINARY_DIR}")
file(COPY "${PROJECT_SOURCE_DIR}/shaders" DESTINATION "${PROJECT_BINARY_DIR}")
file(COPY "${PROJECT_SOURCE_DIR}/fonts" DESTINATION "${PROJECT_BINARY_DIR}")
if(WIN32)
    file(COPY "${PROJECT_SOURCE_DIR}/bin/SDL2.dll" DESTINATION "${PROJECT_BINARY_DIR}")
    set_property(DIRECTORY ${PROJECT_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
ENDIF(WIN32)


include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/external")

if(WIN32)
    add_executable (VKGame WIN32
        "src/game.cpp"
        "src/vulkan_renderer.cpp"
        "src/text_overlay.cpp"
        "src/main.cpp")
ELSEIF(UNIX)
    add_executable (VKGame
        "src/game.cpp"
        "src/vulkan_renderer.cpp"
        "src/text_overlay.cpp"
        "src/main.cpp")
ENDIF(WIN32)

target_link_libraries(VKGame PUBLIC "${SDL2_LIBRARIES}")
target_link_libraries(VKGame PUBLIC "${Vulkan_LIBRARY}")
