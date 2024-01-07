#define PI 3.141592653589793
#define PI2 6.283185307179586
#define EPS 1e-3

float D_GGX_TR(float ndoth, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float nom = a2;
    float denom = ndoth * ndoth * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return nom / max(denom, 0.0001f);
}

float GeometrySchlickGGX(float dotp, float k)
{
    float nom = dotp;
    float denom = dotp * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(float ndotv, float ndotl, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0f;

    return GeometrySchlickGGX(ndotv, k) * GeometrySchlickGGX(ndotl, k);
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 F_SchlickR(float cosTheta, float3 F0, float roughness)
{
    float smothness = 1.0 - roughness;
    return F0 + (max(float3(smothness, smothness, smothness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 calculateNormal(VSOutput input)
{
    float3 tangentNormal = textureNormal.Sample(samplerNormal, input.UV).rgb * 2.0 - 1.0;
    
    tangentNormal = normalize(tangentNormal);
    
    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Tangent);
    float3 B = normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);
    return normalize( mul(TBN, tangentNormal) );
}

float3 DirectLighting(float3 n, float3 v, float3 albedo, float3 F0, float roughness, float metallic, VSOutput input)
{
    float ndotv = clamp(dot(n, v), 0.0, 1.0);
    float3 f = fresnelSchlick(ndotv, F0);

    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < ubo.numLights; i++)
    {
        float3 l = normalize(ubo.pointLights[i].position.xyz - input.WorldPos);
        float3 lightColor = ubo.pointLights[i].color.rgb;
        float ndotl = dot(n, l);
        if (ndotl > 0.0)
        {
            ndotl = clamp(ndotl, 0.0, 1.0);
            float3 h = normalize(v + l);

            float ndoth = clamp(dot(n, h), 0.0, 1.0);

            float ndf = D_GGX_TR(ndoth, roughness);
            float g = GeometrySmith(ndotv, ndotl, roughness);

            float3 nom = ndf * g * f;
            float denom = 4 * ndotv * ndotl + 0.001;
            float3 specular = nom / denom;

            float3 ks = f;
            float3 kd = (float3(1.0f, 1.0f, 1.0f) - f);
            kd *= (1 - metallic);

            Lo += lightColor * (kd * albedo / PI + specular) * ndotl;
        }
    }

    return Lo;
}