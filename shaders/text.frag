#version 450 core

layout (location = 0) in vec2 inUV;

layout (binding = 0) uniform sampler2D samplerFont;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
	//Using the red channel value as transparency
	//Only red channel has value
	float color = texture(samplerFont, inUV).r;
	outFragColor = vec4(color);
}
