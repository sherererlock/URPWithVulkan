
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Assets\Shaders\pbr_vert.hlsl -Fo Assets\Shaders\spv\pbr_vert.hlsl.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Assets\Shaders\pbr_frag.hlsl -Fo Assets\Shaders\spv\pbr_frag.hlsl.spv

@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Assets\Shaders\shadow_vert.hlsl -Fo Assets\Shaders\spv\shadow_vert.hlsl.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Assets\Shaders\shadow_frag.hlsl -Fo Assets\Shaders\spv\shadow_frag.hlsl.spv

@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Assets\Shaders\quad_vert.hlsl -Fo Assets\Shaders\spv\quad_vert.hlsl.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Assets\Shaders\blit_frag.hlsl -Fo Assets\Shaders\spv\blit_frag.hlsl.spv

@REM glslangValidator.exe -V Assets\Shaders\gbuffer_skin.vert -o Assets\Shaders\spv\gbuffer_skin.vert.spv

@REM glslangValidator.exe -V Assets\Shaders\gbuffer_anim.vert -o Assets\Shaders\spv\gbuffer_anim.vert.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Assets\Shaders\gbuffer_vert.hlsl -Fo Assets\Shaders\spv\gbuffer_vert.hlsl.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Assets\Shaders\gbuffer_frag.hlsl -Fo Assets\Shaders\spv\gbuffer_frag.hlsl.spv

@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Assets\Shaders\Lighting_frag.hlsl -Fo Assets\Shaders\spv\Lighting_frag.hlsl.spv

@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Assets\Shaders\uioverlay.vert -Fo Assets\Shaders\spv\uioverlay.vert.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Assets\Shaders\uioverlay.frag -Fo Assets\Shaders\spv\uioverlay.frag.spv