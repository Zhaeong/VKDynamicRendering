#version 450
#extension GL_EXT_debug_printf : enable
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 inFragPos;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inLightPos;


layout (location = 0) out vec4 outColor;

void main() {
    //rgb, alpha
    //outColor = texture(texSampler, fragTexCoord);
    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(inLightPos - inFragPos);
    //debugPrintfEXT("Light dir is %v3f", lightDir);
    //debugPrintfEXT("pos is %v3f", inFragPos);
    debugPrintfEXT("norm is %v3f", norm);

    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    float diffuseStrength = max(dot(norm, lightDir), 0.15);

    //debugPrintfEXT("diffuse strenght: %f", diffuseStrength);

    vec3 diffuse =  diffuseStrength * lightColor;




    vec3 result = diffuse * inColor;
    // outColor = vec4(inColor, 1.0);
    outColor = vec4(result, 1.0);

    
}
