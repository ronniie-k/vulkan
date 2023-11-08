#version 450
#extension GL_OES_standard_derivatives : enable

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 2) in vec2 iTexCoord;
layout(location = 3) in vec4 iTangent;

layout(location = 0) out vec3 oWorldPos;
layout(location = 1) out vec3 oNormal;
layout(location = 2) out vec2 oTexCoord;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normal;
    vec4 lightPos;
    vec4 cameraPos;
} ubo;

void main() {
    oWorldPos = vec3(ubo.model * vec4(iPosition, 1.0));
    oNormal = iNormal;
    oTexCoord = iTexCoord;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(iPosition, 1.0);
}