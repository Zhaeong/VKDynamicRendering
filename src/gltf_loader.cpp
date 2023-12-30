#include "gltf_loader.hpp"

//Note it's important to put this in only one cpp file, else there will be include issues
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <tiny_gltf.h>
namespace GLTF {

GLTFLoader::GLTFLoader(){
  std::cout << "Started GLTFLoader\n";
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;

  std::string err;
  std::string warn;

  bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "");
  if (!warn.empty()) {
    printf("Warn: %s\n", warn.c_str());
  }

  if (!err.empty()) {
    printf("Err: %s\n", err.c_str());
  }

  if (!ret) {
    printf("Failed to parse glTF\n");
  }
}
}
