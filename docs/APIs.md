# API引用  

本文主要介绍Protocol Buffer类在C++如何工作，当然也包含Protocol Buffer本身应用文档。本文包含：  

- 通过Protocol Buffer编译器，如何从`.proto`文件中生成C++代码。
- 为提供的源码生成API文档。  

当然也有其它语言对应的API引用文档，详见[这里](https://developers.google.com/protocol-buffers/docs/reference/overview)。  

## 代码生成指南  

[编译器调用](###编译器调用)  
[包](###包)  
[消息](###消息)  
[字段](###字段)  
[Any](###Any)  
[Oneof](###Oneof)  
[枚举](###枚举)  
[扩展](###扩展)  
[Arena调用](###Arena调用)  
[服务](###服务)  
[插件](###插件)  

本章节实际上是介绍Protocol Buffer编译器从给定的protocol定义中生成的C++代码。所有proto2和proto3生成的代码不同之处都会高亮标出 --- 需要注意的是这些不同之处只是生成的代码中的不同，而不是消息类/接口（同一版本的编译器生成的是一样的）的不同。开始之前，你应该先了解[proto2 language guide](https://developers.google.com/protocol-buffers/docs/proto)或[proto3 language guide](https://developers.google.com/protocol-buffers/docs/proto3)。

### 编译器调用  

使用`--cpp_out=`命令行参数，Protocol Buffer编译器会生成C++输出。`--cpp_out=`选项的参数是你要存放C++输出的目录。编译器会为每个`.proto`文件生成一个头文件和实现文件。输出文件的名称与给定的`.proto`文件名称有关：  

- 后缀（`.proto`）被替换成`.pb.h`（头文件）或`pb.cc`（实现文件）。
- proto路径（通过`--proto_path`或`-I`指定）被输出路径（通过`--cpp_out`指定）替换。  

例如，调用如下命令：  

> protoc --proto_path=src --cpp_out=build/gen src/foo.proto src/bar/baz.proto

编译器读取文件`src/foo.proto`和`src/bar/baz.proto`并产生4个输出文件：`build/gen/foo.pb.h`、`build/gen/foo.pb.cc`、`build/gen/bar/baz.pb.h`和`build/gen/bar/baz.pb.cc`。需要的话，编译器会自动生成`build/gen/bar`目录，但是并不会创建`build`或`build/gen`，因此，它们必须已存在。  

### 包  

如果`.proto`文件包含`package`声明，那么文件中所有的内容都会被放在对应的C++命名空间中。例如，给定`package`声明：  

> pakcage foo.bar

文件中的所有声明都会放在`foo::bar`命名空间中。  

### 消息  

如下，是一个简单的消息声明：  

> message Foo {}

编译器会生成一个名为`Foo`的类，派生自`google::protobuf::Message`。这个类是一个具体的类，不存在为实现的纯虚函数。取决与优化模式，`Message`中的虚函数会/不会被`Foo`重写。默认情况下，`Foo`实现所有方法的特定版本以获得最快速度。但是，如果`.proto`文件中包含：  

> option optimize_for = CODE_SIZE;

之后`Foo`只重写功能所需的最小方法集，剩下的靠基本的反射实现。这会显著减小生成代码的大小，但会降低性能。或者，如果`.proto`文件中包含：  

> option optimize_for = LITE_RUNTIME;

之后`Foo`会包含所有方法的快速实现，但实现的是`google::protobuf::MessageLite`的接口，它只是`Message`方法的一个子集。需要特意说明的是，它不支持描述符或反射。但是，这种模式下，生成的代码只需链接`libprotobuf-lite.so`（Windows下`libprotobuf-lite.lib`）即可，而不是`libprotobuf.so`（`libprotobuf.lib`）。“lite”版本的库比完整的库要小的多，特别适合像手机这样的资源有限的系统。  

你*不*应该创建自己的`Foo`子类。如果你创建了子类且重写了虚函数，重写的函数可能会被忽略，因为许多生成的方法调用被去虚拟胡以提高性能。  

`Message`接口定义了可以让你检查、操作、读写整个消息的方法，包括从二进制字符串中解析和序列化为二进制字符串。  

- `bool ParseFromString(const string& data)`：从给定的序列化后的二进制字符串（即wire格式）解析消息。
- `bool SerializeToString(string* output)`：将给定的消息序列化为二进制字符串。
- `string DebugString()`：返回字符串，文本格式的proto表述（只应在debugging时使用）。

作为上述方法的补充，`Foo`类定义了下列方法：  

- `Foo()`：默认构造函数。
- `~Foo()`：默认析构函数。
- `Foo(const Foo& other)`：拷贝构造。
- `Foo& operator=(const Foo& other)`：赋值运算符。
- `void Swap(Foo* other)`：与另一消息交换信息。
- `const UnknownFieldSet& unknown_fields() const`：返回解析消息遇到的未知字段的集合。

`Foo`类还定义了下面的静态方法：  

- `static const Descriptor* descriptor()`：返回类型的描述，包含该类型的信息，包括有什么字段以及它们的类型。用于[反射](https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.message.html#Message.Reflection)时，可以以编程的方式来检查字段。  
- `static const Foo& default_instance()`：返回一个单例模式的`Foo`实例，它与新构造的`Foo`实例相同（所以所有的单个字段都是未设置的，所有的重复字段都是空的）。。注意，通过调用`New()`方法，消息的默认实例可以当作工厂使用。  

可以在一个消息中声明另一个消息，就像`message Foo { message Bar { } }`。  

这种情况下，编译器会生成两个类：`Foo`和`Foo_Bar`。额外地，编译器会在`Foo`类中生成如下的typedef：  

> typedef Foo_Bar Bar;

这意味着你可以像使用内嵌类`Foo::Bar`那样使用内嵌类型的类。但是，注意C++不允许内嵌类型被前向声明。如果要在另一个文件中使用前向声明`Bar`并使用该声明，则必须将其标识为`Foo_Bar`。

### 字段  
### Any  
### Oneof  
### 枚举  
### 扩展  
### Arena调用  
### 服务  
### 插件