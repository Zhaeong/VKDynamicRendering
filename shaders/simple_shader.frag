#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

void main() {
    //rgb, alpha
    outColor = texture(texSampler, fragTexCoord);
    // outColor = vec4(inColor, 1.0);
}