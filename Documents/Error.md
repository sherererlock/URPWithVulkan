# URPWithVulkanError

1. 在gltfmodel.cpp中Mesh的构造函数里在构造mesh过程中为uniformBuffer创建vkBuffer时因为size和memProperty的参数反了而导致的报错

2. 模型画出来是乱的

   由于VertexInputState的stride和vertexbuffer中每个vertex的实际的stride不同

3. 在shswapchain类的第二个构造函数初始化列表中由于devie成员变量的初始化值是自身而不是传入的值（变量名字拼写错误），故在更改窗口大小视会调用此构造函数，导致device为空，之后在使用device时会导致空引用崩溃

4. vertexAttributeBinding中所有的属性都必须要在vertex中用到，不然会报warning

5. descriptorset在draw中使用了，但是自始至终没有更新过

6. 东西没渲染出来，忘记乘以projection矩阵了

7. 渲染的颜色不对

   - 在dx中mat[0]表示矩阵的第一列，mat[1]表示矩阵的第二列
   - viewpos = mat[3]取的是矩阵的第四列，而真正的viewpos在第四行

8. 编译shader时编译参数和类型要对应

9. 在创建subpassdes时传入的attachmentRef数组忘记写引用符号导致其按值传递离开作用域后析构导致ref失效，从而导致创建renderpass失败

10. descriptorpool没有空间分配set和descriptor

11. VkDescriptorImageInfo采样的layout要与VkAttachmentDescription的final layout保持一致

12. 所有部分都在阴影里

    在GlobalUBO中的多加了一个float4x4，导致数据错位

13. 光照的方向在模型上表现的是反的

14. 移動视角平面的颜色变黑

    - 模型加载时将y被取反，但是camera的y没有被取反, 最终采取的方式，camera取反，模型加载时y不动
    - tbn的计算在hlsl下需要transpose

15. basePass创建renderPass时由于attachment的finalLayout与attachmentRef的layout混淆导致创建失败

    - attachmentRef中的layout是在subpass执行写入时使用的layout，不能说shader read相关的layout
    - final layout则是subpass执行完毕之后转换的layout

16. pipeline中colorAttachmentBlendState数量应该与render pass中的color attachment数量一致

17. pointLightSystem没有渲染到gbuffer上

    shader没有重写

18. basePass写出的colorAttachment背景是红色

    clearvalue中的color和depth共用同一内存空间，不能同时赋值

19. basePass没有渲染出任何东西

    - GltfRenderSystem.createPipelineLayout传的是layouts的引用，添加了之后，使用到了base GltfRenderSystem中了
    - depth的clear value被设置为了0，所以所有东西东渲染不上去

20. lightingpass渲染颜色不对

    - 物体的颜色不对

      - inverseView的的第三行表示相机的位置，而不是第三列

      - 没有乘以inverseProj
      - 

    - 背景也被上了颜色

      clearvalue = (0.01, 0.01, 0.01, 1),各个位置的值都有意义，只能用0来clear

21. 重建世界坐标出现问题

    - 误区1：view的第三行也就是表示位置的行需要被置为0
    - 误区2：千万不要忘记乘以inverseProj
    - 误区3：left2right和top2bottom不需要normalize，因为uv已经是normalize过的值了
    - 误区4：uv值的y值是反的，所以需要用1-uv.y

22. shadow问题

    - position的计算问题

23. 在延迟渲染下，光照亮了物体背对光的表面

    - position的计算问题

24. 光没画出来

    depth被写为0了
    
25. 阴影没考虑到动画

------

### 低级失误

1. F0的lerp居然写成了0.4f

------

# c++

1. 在基类的构造函数中调用虚函数，链接时报错找不到实现

   当在构造基类部分时，派生类还没被完全创建。即当A::A()执行时，B类对象还没被完全创建，此时它被当成一个A对象，而不是B对象，因此Function()绑定的是A的Function()
   
2. 创建对象是，记得对象名后面跟{},表示初始化值，如果明确初始化，则值是未知的ww