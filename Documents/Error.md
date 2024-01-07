# URPWithVulkanError

1. 在gltfmodel.cpp中Mesh的构造函数里在构造mesh过程中为uniformBuffer创建vkBuffer时因为size和memProperty的参数反了而导致的报错

2. 模型画出来是乱的

   由于VertexInputState的stride和vertexbuffer中每个vertex的实际的stride不同

3. 在shswapchain类的第二个构造函数初始化列表中由于devie成员变量的初始化值是自身而不是传入的值（变量名字拼写错误），故在更改窗口大小视会调用此构造函数，导致device为空，之后在使用device时会导致空引用崩溃

4. vertexAttributeBinding中所有的属性都必须要在vertex中用到，不然会报warning

5. descriptorset在draw中使用了，但是自始至终没有更新过

6. 

   ------

   # c++

   1. 在基类的构造函数中调用虚函数，链接时报错找不到实现

      当在构造基类部分时，派生类还没被完全创建。即当A::A()执行时，B类对象还没被完全创建，此时它被当成一个A对象，而不是B对象，因此Function()绑定的是A的Function()