
D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Shaders\pbr_vert.hlsl -Fo Shaders\spv\pbr_vert.hlsl.spv
D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Shaders\pbr_frag.hlsl -Fo Shaders\spv\pbr_frag.hlsl.spv

@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T vs_6_1 -E main Shaders\shadow_vert.hlsl -Fo Shaders\spv\shadow_vert.hlsl.spv
@REM D:\VulkanSDK\1.3.268.0\Bin\dxc.exe -spirv -T ps_6_1 -E main Shaders\shadow_frag.hlsl -Fo Shaders\spv\shadow_frag.hlsl.spv