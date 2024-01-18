
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Shaders\pbr_vert.hlsl -Fo Shaders\spv\pbr_vert.hlsl.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Shaders\pbr_frag.hlsl -Fo Shaders\spv\pbr_frag.hlsl.spv

@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Shaders\shadow_vert.hlsl -Fo Shaders\spv\shadow_vert.hlsl.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Shaders\shadow_frag.hlsl -Fo Shaders\spv\shadow_frag.hlsl.spv

@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Shaders\quad_vert.hlsl -Fo Shaders\spv\quad_vert.hlsl.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Shaders\blit_frag.hlsl -Fo Shaders\spv\blit_frag.hlsl.spv

@REM glslangValidator.exe -V Shaders\gbuffer_skin.vert -o Shaders\spv\gbuffer_skin.vert.spv
glslangValidator.exe -V Shaders\gbuffer_anim.vert -o Shaders\spv\gbuffer_anim.vert.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Shaders\gbuffer_vert.hlsl -Fo Shaders\spv\gbuffer_vert.hlsl.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Shaders\gbuffer_frag.hlsl -Fo Shaders\spv\gbuffer_frag.hlsl.spv

D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Shaders\Lighting_frag.hlsl -Fo Shaders\spv\Lighting_frag.hlsl.spv