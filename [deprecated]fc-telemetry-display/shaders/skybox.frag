#version 460
layout(binding = 0) uniform samplerCube skybox;
layout(location = 0) in vec3 fragTexCoord3D;
layout(location = 0) out vec4 outputFragment;
void main()
{
    outputFragment = texture(skybox, fragTexCoord3D);
}