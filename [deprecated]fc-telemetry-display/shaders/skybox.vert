#version 460
layout(binding = 0, std140) uniform GlobalMatrixBlock
{
	mat4 matProjection;
	mat4 matView;
};
//layout(location = 0) uniform mat4 matModel;
layout(location = 0) in vec3 vertexPosition;
layout(location = 0) out vec3 fragTexCoord3D;
void main()
{
    fragTexCoord3D = vertexPosition;
    mat4 translationlessView = mat4(mat3(matView));
    vec4 positionVP = matProjection * translationlessView *//matView * matModel * 
        vec4(vertexPosition, 1.0);
    gl_Position = positionVP.xyww;
}