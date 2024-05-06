#version 450
#extension GL_EXT_debug_printf : enable


layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 light;
    vec3 camerPos;
} ubo;

layout(binding = 1) uniform UniformBufferObjectModel {
    mat4 modelPos;
} uboModel;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outFragPos;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec3 outLightPos;
layout(location = 5) out vec3 outCameraView;


void main() {
    gl_Position = ubo.proj * ubo.view * uboModel.modelPos *  vec4(inPosition, 1.0);
    // fragColor = inColor;
    fragTexCoord = inTexCoord;

    // gl_Position = vec4(inPosition, 1.0);
    fragColor = inColor;

    // Output the position in world coord to feed into frag shader
    outFragPos = vec3(uboModel.modelPos * vec4(inPosition, 1.0));

    outNormal = inNormal;

    outLightPos = ubo.light.xyz;

    outCameraView = outFragPos - ubo.camerPos;
    
    //debugPrintfEXT("Light pos is %v4f", ubo.light);
    //debugPrintfEXT("pos is %v4f", gl_Position);

}
