
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 Pos;

layout(set = 0, binding = 0) 
uniform UniformBufferObject
{
    float4x4 lightVP;
} ubo;

layout(push_constant) uniform PushConsts{
    mat4 modelMatrix;
    mat4 normalMatrix;
}primitive;


void main()
{
    mat4 model = primitive.modelMatrix * nodebo.nodeMatrice;
    vec4 position = (model * vec4(Pos, 1.0));

    oNormal = normalize(mat * Normal);
    oTangent = normalize(mat * Tangent);
    gl_Position = ubo.projection * ubo.view * position;
}