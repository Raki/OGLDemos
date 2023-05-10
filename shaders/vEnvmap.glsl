#version 460

layout (location = 0) in vec3 position;

uniform mat4 proj;
uniform mat4 view;

out vec3 localPos;

void main()
{
    localPos = position;

    mat4 rotView = mat4(mat3(view)); // remove translation from the view matrix
    vec4 clipPos = proj* rotView * vec4(localPos, 1.0);

    gl_Position = clipPos.xyww;
}