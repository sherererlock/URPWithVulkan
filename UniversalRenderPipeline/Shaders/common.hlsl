
float linearDepth(float depth, float znear, float zfar)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * znear * zfar) / (zfar + znear - z * (zfar - znear));
}

float3 ReconstructWorldPos(float2 uv, float linearDepth, float3 lt, float3 l2r, float3 t2b, float znear)
{
    float zscale = linearDepth + 1.0f / znear;
    float3 viewpos = lt + l2r * uv.x + t2b * uv.y;
    
    return viewpos * zscale;
}