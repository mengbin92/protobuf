# 学习指南(proto3)  

这篇指南讲述如何使用Protocol Buffers来结构化你的Protocol Buffer数据，包括`.proto`文件语法以及如何从`.proto`文件生成你的访问类型。本文主要涵盖了**proto3**的语法，**proto2**的语法参见[Proto2 Language Guide](https://developers.google.com/protocol-buffers/docs/proto)。  

这是一篇参考教程 -- 本文中诸多功能的分步示例，详见[tutorial](https://developers.google.com/protocol-buffers/docs/tutorials)。  

[定义消息类型](##定义消息类型)  
[标量类型](##标量类型)
[默认变量](##默认变量)
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

## 标量类型  

## 默认变量  

## 枚举  

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