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
}

void GLTFLoader::loadFile(std::string filePath) {

  mVertices.clear();
  mIndices.clear();

  tinygltf::Model model;
  tinygltf::TinyGLTF loader;

  std::string err;
  std::string warn;

  std::filesystem::path p = std::filesystem::current_path();

  bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, (p.generic_string() + filePath));

  if (!warn.empty()) {
    printf("Warn: %s\n", warn.c_str());
  }

  if (!err.empty()) {
    printf("Err: %s\n", err.c_str());
  }

  if (!ret) {
    printf("Failed to parse glTF\n");
  } else {
    const tinygltf::Scene& scene = model.scenes[0];
    
    for (size_t i = 0; i < scene.nodes.size(); i++) {
      const tinygltf::Node node = model.nodes[scene.nodes[i]];
      loadNode(node, model);
    }
  }
}

void GLTFLoader::loadNode(tinygltf::Node node, tinygltf::Model model){
  std::cout<< "looking at: " << node.name << "\n";

  // Load node's children
  if (node.children.size() > 0) {
      for (size_t i = 0; i < node.children.size(); i++) {
          loadNode(model.nodes[node.children[i]], model);
      }
  }

  if(node.mesh > -1) {
    std::cout<< "mesh index: " << node.mesh << "\n";
    const tinygltf::Mesh mesh = model.meshes[node.mesh];
    std::cout<< "mesh name: " << mesh.name << "\n";

    //Iterate over mesh primitives for each vertex info
    for (size_t i = 0; i < mesh.primitives.size(); i++) {
	    const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
      //=============
      //Vertices
      //=============
      // Get buffer data for vertex positions
      uint32_t vertexStart = static_cast<uint32_t>(mVertices.size());
      const float* positionBuffer = nullptr;
      size_t vertexCount = 0;
      if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
				const tinygltf::Accessor& position_accessor = model.accessors[glTFPrimitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& position_bufferView = model.bufferViews[position_accessor.bufferView];
				positionBuffer = reinterpret_cast<const float*>(
                            &(model.buffers[position_bufferView.buffer].data[position_accessor.byteOffset + position_bufferView.byteOffset])
                            );
				vertexCount = position_accessor.count;
      }

      //=============
      //Normals
      //=============
      const float* normalBuffer = nullptr;
      if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
        const tinygltf::Accessor& normal_accessor = model.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
				const tinygltf::BufferView& normal_bufferView = model.bufferViews[normal_accessor.bufferView];
				normalBuffer = reinterpret_cast<const float*>(
                            &(model.buffers[normal_bufferView.buffer].data[normal_accessor.byteOffset + normal_bufferView.byteOffset])
                            );
				
      }

      // Push relevant vertex and normal data into the relevant objects
      for(size_t v = 0; v < vertexCount; v++) {
        Utils::Vertex vertex{};
        glm::vec3 position = glm::make_vec3(&positionBuffer[v * 3]);
        //glm::vec3 position2 =glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
        //std::cout << "index:" << v << glm::to_string(position) << "\n";
        vertex.pos = position;
        vertex.color =  glm::vec3(1.0f, 0.0f, 0.0f);
        vertex.texCoord = glm::vec2(0.0f, 0.0f);

        //std::cout << glm::to_string(vertex.color) << "\n";
        if(normalBuffer) {
          glm::vec3 normal = glm::make_vec3(&normalBuffer[v * 3]);
          vertex.normal = normal;
          //std::cout << "Normal:" << glm::to_string(normal) << "\n";
        }

        mVertices.push_back(vertex);
      }

      //===========
      //Indices
      //===========
      const tinygltf::Accessor& indices_accessor = model.accessors[glTFPrimitive.indices];
      const tinygltf::BufferView& indices_bufferView = model.bufferViews[indices_accessor.bufferView];
      const tinygltf::Buffer& indices_buffer = model.buffers[indices_bufferView.buffer];

      switch(indices_accessor.componentType) {
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
          const uint32_t* buf = reinterpret_cast<const uint32_t*>(&indices_buffer.data[indices_accessor.byteOffset + indices_bufferView.byteOffset]);
          for (size_t index = 0; index < indices_accessor.count; index++) {
            mIndices.push_back(buf[index] + vertexStart);
          }
          break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
          const uint16_t* buf = reinterpret_cast<const uint16_t*>(&indices_buffer.data[indices_accessor.byteOffset + indices_bufferView.byteOffset]);
          for (size_t index = 0; index < indices_accessor.count; index++) {
            mIndices.push_back(buf[index] + vertexStart);
          }
          break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
          const uint8_t* buf = reinterpret_cast<const uint8_t*>(&indices_buffer.data[indices_accessor.byteOffset + indices_bufferView.byteOffset]);
          for (size_t index = 0; index < indices_accessor.count; index++) {
            mIndices.push_back(buf[index] + vertexStart);
          }
          break;
        }
        default:
          std::cerr << "Index component type " << indices_accessor.componentType << " not supported!" << std::endl;
          return;
      }

      
      for (auto i: mIndices){
        std::cout << i << ' ';
      }
      std::cout << " indices size: " << mIndices.size() << "\n";
      
    }
  }

}
}
