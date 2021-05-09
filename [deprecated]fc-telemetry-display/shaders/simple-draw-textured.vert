#version 460
layout(binding = 0, std140) uniform GlobalMatrixBlock
{
	mat4 matProjection;
	mat4 matView;
};
layout(location = 0) uniform mat4 matModel;
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTextureCoordinate;
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in vec3 vertexNormal;
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
void main()
{
	fragTexCoord = vertexTextureCoordinate;
	fragColor    = vertexColor;
	gl_Position  = matProjection * matView * matModel *
		vec4(vertexPosition, 1);
}