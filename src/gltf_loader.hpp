#pragma once 
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <tiny_gltf.h>
#include <utils.hpp>
#include <iostream>
namespace GLTF{
struct GLTFLoader {
  std::vector<uint32_t> mIndices;
  std::vector<Utils::Vertex> mVertices;

  GLTFLoader();
  ~GLTFLoader();

  void loadNode(tinygltf::Node node, tinygltf::Model model);

};
}
