# Protocol Buffer 基础：C++  

本教程提供protocol buffer在C++程序中的基础用法。通过创建一个简单的示例程序，向你展示如何：  

- 在`.proto`中定义消息格式
- 使用protocol buffer编译器
- 使用C++ protocol buffer API读写消息  

这并不是protocol buffer在C++中使用的完整指南。更多细节，详见[Protocol Buffer Language Guide](https://developers.google.com/protocol-buffers/docs/proto3)、[C++ API Reference](https://developers.google.com/protocol-buffers/docs/reference/cpp/index.html)、[C++ Generated Code Guide](https://developers.google.com/protocol-buffers/docs/reference/cpp-generated)和[Encoding Reference](https://developers.google.com/protocol-buffers/docs/encoding)。  

## 为什么使用Protocol Buffer  

我们要使用的例子是一个非常简单的“通讯录”应用程序，它可以从文件中读写联系人的信息。通讯录中每个人都有一个姓名、ID、邮箱和练习电话。  

你如何序列化并取回这样结构化的数据呢？下面有几条建议：  

- 原始内存中数据结构可以发送/保存为二进制。这是一种随时间推移而变得脆弱的方法，因为接收/读写的代码必须编译成相同的内存布局，endianness等。另外，文件已原始格式积累数据和在网络中到处传输副本，因此扩展这种格式十分困难。
- 你可以编写已临时的方法来讲数据元素编码到单个字符串中 --- 例如用“12:3:-23:67”来编码4个int。这是一种简单而灵活的方法，尽管它确实需要编写一次性的编码和解析代码，并且解析会增加少量的运行时成本。这对于编码非常简单的数据最有效。
- 序列化为XML。这种方法非常有吸引力，因为XML(某种程度上)是人类可读的，而且有许多语言的绑定库。如果您希望与其他应用程序/项目共享数据，这可能是一个不错的选择。然而，XML是出了名的空间密集型，对它进行编码/解码会给应用程序带来巨大的性能损失。而且，在XML DOM树中导航要比在类中导航简单字段复杂得多。

Protocol buffer是解决上述问题的一个灵活、高效、高度自动化的解决方案。使用Protocol buffer，你只需在`.proto`文件中描述你想要存储的数据结构。从文件中，protocol buffer编译器会创建一个类 --- 实现了可以自动编解码的、高效的二进制protocol buffer数据。生成的类为组成protocol buffer的字段提供getter和setter方法，并负责将protocol buffer作为一个整体进行读写的细节。重要的是，protocol buffer协议支持扩展格式，以便新的代码仍可读取旧格式的编码。  

## 从哪能找到示例代码呢？  

你可以从[这里](https://github.com/MasterMeng/protobuf.git)下载。  

## 定义你的Protocol格式  

要创建通讯录程序，始于`.proto`文件。`.proto`文件中的定义很简单：为你想要序列化的每一个数据结构添加一个*消息*，然后声明消息中每个字段的名称和类型。示例使用的`.proto`文件为`addressbook.proto`，其中定义如下：  

```proto
syntax = "proto3";

package tutorial;

message Person {
  string name = 1;
  int32 id = 2;
  string email = 3;

  enum PhoneType {
    MOBILE = 0;
    HOME = 1;
    WORK = 2;
  }

  message PhoneNumber {
    string number = 1;
    PhoneType type = 2;
  }

  repeated PhoneNumber phones = 4;
}
```  

如你所见，语法与C++/Java类似。接下来介绍文件中的每一部分以及它们如何工作。  

`.proto`开头声明使用`proto3`语法，若不明确指出，编译器默认使用`proto2`语法。之后是包声明，用来解决不同项目的命名冲突。在C++中，你生成的代码会被放在与包名对应的命名空间。  

接着，定义你的消息。消息只是一系列字段类型的集合体。很多标准的、简单的数据类型可以作为字段类型，包括`bool`、`int32`、`float`、`double`和`string`。你也可以使用其它消息类型作为字段类型来添加复杂结构到你的消息中 --- 就像上面例子中，`Person`消息包含`PhoneNumber`消息，同时`Person`消息包含在`AddressBook`消息中。你甚至可以定义消息类型嵌套在其它消息中 --- 就像上面`PhoneNumber`定义在`Person`中。你也可以定义`enum`类型，如果你想让你的字段只是用预定义列表中的一个值 --- 这里你想声明的电话类型可以是`MOBILE`、`HOME`或`WORK`其中之一。  

“= 1”，“= 2”标记每个字段在二进制编码中的唯一的“tag”。序号1-15编码的字节数比较高的数字少一个，因此，作为一种优化，您可以决定对常用或重复的元素使用这些标记，而对不常用的可选元素使用标记16或更高。重复字段中的每个元素都需要重新编码标记号，因此重复字段是此优化的特别好的候选项。  

每个字段都必须遵循下列规则之一：  

- singular：符合语法规则的消息可以拥有0个或1个该字段（但不能超过1个）。这是proto3默认的字段规则。  
- repeated：在符合语法规则的消息中，该字段可以重复任意次数（包括0次）。重复变量的顺序将被保留。    

完整的编写`.proto`文件指南，详见[Language Guide(proto3)](https://developers.google.com/protocol-buffers/docs/proto3)。  

## 编译Protocol Buffers  

现在你已经有`.proto`文件了，接下来你需要生成读写`AddressBook`（包括`Person`和`PhoneNumber`）消息的类。现在，你需要运行protocol buffer编译器`protoc`：  

- 如果你还没安装编译器，可从[这里](https://github.com/protocolbuffers/protobuf)下载并根据README编译安装。
- 现在运行编译器，指明源目录（应用程序源文件目录，不指定的话默认使用当前目录），目标路径（你要存放生成的代码的目录，通常与`$SRC_DIR`一样），`.proto`文件路径。这样，你可以：  

> protoc -I=\$SRC_DIR --cpp_out=\$DST_DIR $SRC_DIR/addressbook.proto  

因为要生成C++类，所以使用`--cpp_out`选项。若要生成其它支持的语言，提供类似选项即可。  

目标路径下会生成下列文件：  

- `addressbook.pb.h`，声明生成的类的头文件。
- `addressbook.pb.cc`，包含类的实现。