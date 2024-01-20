#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "macros.hlsl"

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 UV;
layout(location = 3) in vec3 Tangent;

layout(location = 0) out vec3 oNormal;
layout(location = 1) out vec2 oUV;
layout(location = 2) out vec3 oTangent;
layout(location = 3) out vec3 oPosition;

struct PointLight
{
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) 
uniform UniformBufferObject
{
    mat4 projection;
    mat4 view;
    vec4 ambientLightColor;
    vec4 viewPos;
    vec4 camereInfo;
    vec4 size;
    PointLight pointLights[10];
    int numLights;
} ubo;

#ifdef CPU_ANIM
layout(set = 2, binding = 0)
uniform nodeMatrices {
	mat4 nodeMatrice;
}nodebo;
#endif

layout(push_constant) uniform PushConsts{
    mat4 modelMatrix;
    mat4 normalMatrix;
}primitive;

void main()
{
#ifdef CPU_ANIM
    mat4 model = primitive.modelMatrix * nodebo.nodeMatrice;
#else
    mat4 model = primitive.modelMatrix;
#endif
    vec4 position = (model * vec4(Pos, 1.0));
    oPosition = position.xyz;
    mat3 mat = mat3(model);
    mat = transpose(inverse(mat3(mat)));
    oUV = UV;
    oNormal = normalize(mat * Normal);
    oTangent = normalize(mat * Tangent);
    gl_Position = ubo.projection * ubo.view * position;
}