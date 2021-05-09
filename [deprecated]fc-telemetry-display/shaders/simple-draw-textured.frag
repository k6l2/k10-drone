#version 460
layout(binding = 0) uniform sampler2D textureSampler2d;
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 0) out vec4 outputFragment;
void main()
{
	outputFragment = fragColor * texture(textureSampler2d , fragTexCoord);
}