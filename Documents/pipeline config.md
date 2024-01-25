# ShaderConfig

## gbuffer_anim.vert

## gbuffer_frag.vert

|              | descriptorSet0 | descriptorSet1             | descriptorSet2 |
| ------------ | -------------- | -------------------------- | -------------- |
| **binding0** | **GlobalUBO**  | **Albedo Texture**         | **nodeMatrix** |
| **binding1** |                | **Normal Texture**         |                |
| **binding2** |                | **RoughnessMetal Texture** |                |
| **binding3** |                | **Emissive Texture**       |                |
|              |                |                            |                |

## Transparent.vert

## Transparent.frag

|              | descriptorSet0                    |
| ------------ | --------------------------------- |
| **binding0** | **GlobalUBO**                     |
| **binding1** | **Transparent Texture**           |
| **binding2** | **PositionDepth InputAttachment** |

## Lighting_frag

|              | **descriptorSet0**  | **descriptorSet1**           |
| ------------ | ------------------- | ---------------------------- |
| **binding0** | **GlobalUBO**       | **Albedo InputAttachment**   |
| **binding1** | **CameraExtentUBO** | **Normal InputAttachment**   |
| **binding2** | **ShadowUBO**       | **Emissive InputAttachment** |
| **binding3** |                     | **Poistion InputAttachment** |
| **binding4** |                     | **Shadow Texture**           |

## DeferRenderingPass
