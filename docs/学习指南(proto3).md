# 学习指南(proto3)  

这篇指南讲述如何使用Protocol Buffers来结构化你的Protocol Buffer数据，包括`.proto`文件语法以及如何从`.proto`文件生成你的访问类型。本文主要涵盖了**proto3**的语法，**proto2**的语法参见[Proto2 Language Guide](https://developers.google.com/protocol-buffers/docs/proto)。  

这是一篇参考教程 -- 本文中诸多功能的分步示例，详见[tutorial](https://developers.google.com/protocol-buffers/docs/tutorials)。  

[定义消息类型](##定义消息类型)  
[标量类型](##标量类型)
[默认值](##默认值)
[枚举](##枚举)
[使用其他消息类型](##使用其他消息类型)
[嵌套类型](##嵌套类型)
[更新消息类型](##更新消息类型)
[未知字段](##未知字段)
[Any](##Any)
[Oneof](##Oneof)
[Maps](##Maps)
[包](##包)
[定义服务](##定义服务)
[Json Mapping](##Json\ Mapping)
[可选项](##可选项)
[编译生成](##编译生成)

## 定义消息类型  

首先来看一个非常简单的例子。假设你想定义一个**搜素请求**的消息格式，其中每个搜索请求都包含一个检索字段、特定的结果页（你感兴趣的结果所在的页面）以及每个页面的结果数量。你可以使用下面的`.proto`文件来定义消息类型。  

```proto
syntax = "proto3";

message SearchRequest {
    string query = 1;
    int32 page_number = 2;
    int32 result_per_page = 3;
}
```  

- 文件的第一行指明你要使用`proto3`语法：如果你不指定的话，protocol buffer编译器将默认你使用的是`proto2`。这必须写在文件中非空、非注释的第一行。  
- `SearchRequest`消息明确定义了三个字段（键值对），对应每一条你想包含在这个消息类型中的数据。每个字段都有一个名称和类型。  

### 指明字段类型  

在上面的例子中，所有的字段都是明确类型的：两个integers（`page_number`和`result_per_page`）和一个string（`query`）。当然，你也可以将你的字段指定成复合类型，包括[枚举](##枚举)和其他消息类型。  

### 分配字段序号  

如你所见，在定义的消息中的每个字段都有一个**唯一的序号**。这些序号用来在二进制消息结果中标识你的字段，而且一旦使用了消息类型，就不应该再变动。注意，字段序号在1到15的范围内占用1个字节来编码，包括字段序号和字段类型（详见[Protocol Buffer Encoding](https://developers.google.com/protocol-buffers/docs/encoding.html#structure)）。字段序号在16到2047范围内占两个字节。所以你应该为经常使用的消息元素保留1到15的序号。切记为将来可能新增的常用元素预留一些空间。  

你能使用的最小字段序号为1，最大为$2^{29}-1$，或 536,870,911。但是你不能使用19000到19999（`FieldDescriptor::kFirstReservedNumber`到`FieldDescriptor::kLastReservedNumber`），因为它们是为Protocol Buffers实现预留的，如果在你的`.proto`文件中使用了，protocol buffer编译器会报错。同样，你也不能使用任何以前保留的字段序号。  

### 标明字段规则  

消息字段可以遵循下列规则之一：  

- singular：符合语法规则的消息可以拥有0个或1个该字段（但不能超过1个）。这是proto3默认的字段规则。  
- repeated：在符合语法规则的消息中，该字段可以重复任意次数（包括0次）。重复变量的顺序将被保留。  

在proto3中，`repeated`字段的标量数字默认使用`packed`编码。关于`packed`编码，详见[Protocol Buffer Encoding](https://developers.google.com/protocol-buffers/docs/encoding.html#packed)。  

### 新增更多消息类型  

在单个`.proto`文件中可以定义多个消息类型。这在你定义多个关联的消息类型时非常有用，例如，如果你想定义应答消息格式来满足你的`SearchResponse`消息类型，你可以在同一个`.proto`文件中添加：  

```proto
...

message SearchRequest {
    string query = 1;
    int32 page_number = 2;
    int32 result_per_page = 3;
}

message SearchResponse {
    ...
}
```  

### 添加注释  

要在你的`.proto`文件中添加注释，可以使用C/C++风格的`//`和`/* ... */`语法。  

```proto
/* SearchRequest represents a search query, with pagination options to
 * indicate which results to include in the response. */

message SearchRequest {
  string query = 1;
  int32 page_number = 2;  // Which page number do we want?
  int32 result_per_page = 3;  // Number of results to return per page.
}
```  

### 预留字段  

如果你通过完全删除或注释一个字段来[更新](##更新消息类型)，那么此后的用户在更新他们自己的类型时将可以重用该字段的序号。如果之后他们使用旧版的`.proto`时，会引起严重的问题，包括数据损坏、隐私bug等。避免给问题的途径之一就是指明你要删除的字段需要（或者会在JSON序列化时会引起问题的名称）是`reserved`的，这样将来用户在使用这些字段时protocol buffer编译器就会告警。  

```proto
message Foo {
  reserved 2, 15, 9 to 11;
  reserved "foo", "bar";
}
```  

注意，不能在同一个`reserved`语句中混用字段名称和字段序号。  

### 你的`.proto`文件会生成什么？  

当你使用[protocol buffer 编译器](##编译生成)时，编译器会根据你选定的语言来生成你`.proto`文件中描述的消息类型，包括获取和设置字段的值，序列化你的消息到一个输出流中，从输入流中解析你的消息。  

- 对**C++**，编译器会根据每个`.proto`文件生成一个`.h`和一个`.cc`，你定义的每个消息类型都会变成一个类。  
- 对**Java**，编译器会生成一个`.java`文件，包含每个消息类型的类，同时还会指明一个`Builder`类来创建消息类的实例。  
- **Python**有点不同，Python编译器会生成一个模块，包含你`.proto`文件每一消息类型的静态描述，之后在运行时通过*基类*来创建必要的Python数据访问类。  
- 对于**Go**，编译器会生成一个`.pb.go`的文件，包含文件中每个消息类型。  
- 对于**Ruby**，编译器会生成一个`.rb`的文件，包含消息类型的Ruby模块。  
- 对于**Objective-C**，编译器会根据每个`.proto`文件生成一个`pbobjc.h`和一个`pbobjc.m`文件，你定义的每个消息类型都会变成一个类。  
- 对**C#**，编译器会根据每个`.proto`文件生成一个`.cs`文件，你定义的每个消息类型都会变成一个类。  
- 对**Dart**，编译器生成一个`.pb.dart`文件，文件中定义的每个消息类型都会变成一个类。  

你可以在之后的教程中找关于对应语言的APIs的使用。更多APIs细节，详见[API reference](https://developers.google.com/protocol-buffers/docs/reference/overview)。

## 标量类型  

标量字段可以是下面类型中的任意一个。下表展示了`.proto`文件中标明的类型，以及在自动生成的类中对应的类型：  

| .proto 类型 |                                 说明                                 | C++类型 |   Java类型   | Python类型$^{[2]}$  | Go类型  |           Ruby类型            |   C#类型   |         PHP类型          |  Dart类型  |
| :---------: | :------------------------------------------------------------------: | :-----: | :----------: | :-----------------: | :-----: | :---------------------------: | :--------: | :----------------------: | :--------: |
|   double    |                                                                      | double  |    double    |        float        | float64 |             Float             |   double   |          float           |   double   |
|    float    |                                                                      |  float  |    float     |        float        | float32 |             Folat             |   float    |          float           |   double   |
|    int32    | 使用可变长度编码。编码负数低效，如果字段可能有负数，使用sint32代替。 |  int32  |     int      |         int         |  int32  | Fixnum or Bignum(as required) |    int     |         integer          |    int     |
|    int64    | 使用可变长度编码。编码负数低效，如果字段可能有负数，使用sint64代替。 |  int64  |     long     |  int/long$^{[3]}$   |  int64  |            Bignum             |    long    |  integer/string$^{[5]}$  |   int64    |
|   uint32    |                          使用可变长度编码。                          | uint32  | int$^{[1]}$  |  int/long$^{[3]}$   | uint32  | Fixnum or Bignum(as required) |    uint    |         integer          |    int     |
|   uint64    |                          使用可变长度编码。                          | uint64  | long$^{[1]}$ |  int/long$^{[3]}$   | uint64  |            Bignum             |   ulong    | integer/string$^{[5]}$}$ |   int64    |
|   sint32    |       使用可变长度编码。有符号整数。编码负数比普通int32高效。        |  int32  |     int      |         int         |  int32  | Fixnum or Bignum(as required) |    int     |         integer          |    int     |
|   sint64    |       使用可变长度编码。有符号整数。编码负数比普通int64高效。        |  int64  |     long     |  int/long$^{[3]}$   |  int64  |            Bignum             |    long    |  integer/string$^{[5]}$  |   int64    |
|   fixed32   |              4字节。如果变量大于2$^{28}$比uint32高效。               | uint32  | int$^{[1]}$  |         int         |  int32  | Fixnum or Bignum(as required) |    int     |         integer          |    int     |
|   fixed64   |              8字节。如果变量大于2$^{56}$比uint64高效。               | uint64  | long$^{[1]}$ |  int/long$^{[3]}$   | uint64  |            Bignum             |   ulong    |  integer/string$^{[5]}$  |   int64    |
|  sfixed32   |                                4字节                                 | uint32  | int$^{[1]}$  |         int         |  int32  | Fixnum or Bignum(as required) |    int     |         integer          |    int     |
|  sfixed64   |                                8字节                                 | uint64  | long$^{[1]}$ |  int/long$^{[3]}$   | uint64  |            Bignum             |   ulong    |  integer/string$^{[5]}$  |   int64    |
|    bool     |                                                                      |  bool   |   boolean    |        bool         |  bool   |     TrueClass/FalseClass      |    bool    |         boolean          |    bool    |
|   string    |       必须是UTF-8编码或7-bit的ASCII文本，长度不能大于2$^{32}$        | string  |    String    | str/unicode$^{[4]}$ | string  |         String(UTF-8)         |   string   |          string          |   String   |
|    bytes    |                 可以包含任何长度不超过232的字节序列                  | string  |  ByteString  |         str         | []byte  |      String(ASCII-8BIT)       | ByteString |          string          | List\<int> |

更多编码细节，详见[Protocol Buffer Encoding](https://developers.google.com/protocol-buffers/docs/encoding)。  

$^{[1]}$在Java中，无符号32位和64位整数使用它们的有符号对应的整数表示，顶部的整数只存储在符号位中。  

$^{[2]}$在所有场景中，给字段设置值时将调用类型检查来确保有效。  

$^{[3]}$64位或无符号32位整数会解码成对应的长度，但如果在设置字段时给定的是int，也可以解码为int。  

$^{[4]}$Python strings会解码成unicode，但如果给定的是ASCII string，会被解码成str。  

$^{[5]}$Integer用在64位机器上，string用于32位机器。  

## 默认值  

在解析消息时，如果编码的消息不包含特定的singular元素，则解析对象中的相应字段将设置为该字段的默认值。这些默认值与类型有关：  

- 对于string，默认值为空字符串。
- 对于bytes，默认值是空bytes。
- 对于bool，默认值为false。
- 对于数字类型，默认值为0。
- 对于[枚举](##枚举)，默认值为第一个定义的枚举变量，其值必须为0。
- 对于消息字段，未设置。他都值因语言不同而不同，详见[generated code guide](https://developers.google.com/protocol-buffers/docs/reference/overview)。  

对于repeated字段，其默认值为空（通常是目标语言的空列表）。  

对于标量消息字段来说，一旦消息被解析，就无法判断该字段是真实被设为默认值（例如bool变量被设为false）还是就没有设置：在定义消息类型时需要牢记这一点。例如，如果你不想在默认情况向执行某种行为，那么就不要用boolean被设置为`false`来切换这些行为。同时，如果一个标量消息字段被设为它的默认值，那么改值在传输时将不会被序列化。  

在生成代码时，在你所选的语言中默认值如何工作，详见[generated code guide](https://developers.google.com/protocol-buffers/docs/reference/overview)。  

## 枚举  

当你在定义消息类型时，你可能想让它的字段只是用预定义列表中的一个值。例如，假如你想给`SearchRequest`添加一个`corpus`字段，其值可以是`UNIVERSAL`、`WEB`、`IMAGE`、`LOCAL`、`NEWS`、`PRODUCT`或者`VIDEO`。通过`enum`，在你的消息中定义一个包含每个可能值得枚举变量，可以很简单地做的。  

在下面的例子中，我们添加了一个名为`Corpus`的`enum`类型，它包含了所有可能的值，和一个`Corpus`类型的字段：  

```proto
message SearchRequest {
  string query = 1;
  int32 page_number = 2;
  int32 result_per_page = 3;
  enum Corpus {
    UNIVERSAL = 0;
    WEB = 1;
    IMAGES = 2;
    LOCAL = 3;
    NEWS = 4;
    PRODUCTS = 5;
    VIDEO = 6;
  }
  Corpus corpus = 4;
}
```

## 使用其他消息类型  

## 嵌套类型  

## 更新消息类型  

## 未知字段  

## Any  

## Oneof  

## Maps  

## 包  

## 定义服务  

## Json Mapping  

## 可选项  

## 编译生成