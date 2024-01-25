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

|      | Attachment | FrameBuffer | ClearValues                |
| ---- | ---------- | ----------- | -------------------------- |
| 0    | Color      | Color       | { 0.0f, 0.0f, 0.0f, 0.0f } |
| 1    | Albedo     | Albedo      | { 0.0f, 0.0f, 0.0f, 0.0f } |
| 2    | Normal     | Normal      | { 0.0f, 0.0f, 0.0f, 0.0f } |
| 3    | Emissive   | Emissive    | { 0.0f, 0.0f, 0.0f, 0.0f } |
| 4    | Position   | Position    | { 0.0f, 0.0f, 0.0f, 0.0f } |
| 5    | Depth      | Depth       | { 1.0f, 0 }                |

### 

|          | ColorRef    | depthRef | inputRef  |
| -------- | ----------- | -------- | --------- |
| Subpass0 | (0,1,2,3,4) | 5        |           |
| Subpass1 | 0           | 5        | (1,2,3,4) |
| Subpass2 | 0           | 5        | 4         |

