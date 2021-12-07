# c2llvm

## 项目结构：
- compiler.l
  - 词法分析
- compiler.y
  - 获取lex分析的词法，然后做语法分析
  - parse的过程中，使用ast.h&ast.c，生成合理的AST结构
- ast.h & ast.c
  - 依赖于llvm库
  - 每一个基于ASTNode的类对应AST的每一个结点
    - generate方法返回一个llvm::Value*， 即该结点生成的代码
- generator.h & generator.c
  - 生成llvm-ir
- main.cpp
  - 主函数
  - 调用yyparse对输入进行parse
- MAKEFILE

## 项目部署：
> version: llvm-13.0.0


- LLVM的安装：
- 下载：https://github.com/llvm/llvm-project/releases/tag/llvmorg-13.0.0
- 下载带project的那一个，解压后，编译
```cpp
$ cd llvm-project
$ mkdir build && cd build
$ cmake -G "Unix Makefiles" -DLLVM_ENABLE_PROJECTS="clang" \
    -DCMAKE_BUILD_TYPE=Debug -DLLVM_TARGETS_TO_BUILD="X86" \
    -DBUILD_SHARED_LIBS=On ../llvm
$ make -j7
```
make -j7的7可以改成其他数字，加速
- 需要设置环境变量
  - 一个脚本（第一个冒号前可能就够了，为避免问题全部加上）
``` 
export LLVM_HOME=/home/glassesq/llvm-project-13.0.0.src/build/bin:/home/glassesq/llvm-project-13.0.0.src/lib:/home/glassesq/llvm-project-13.0.0.src/build/lib
export PATH=$LLVM_HOME:$PATH
echo $PATH
echo "llvm path ok"
```
- 需要设置动态库
  - ~~添加动态库： https://blog.csdn.net/smilejiasmile/article/details/84307526~~
  - 编辑/etc/ld.so.conf文件，在新的一行中加入库文件所在目录；
  - 运行ldconfig，以更新/etc/ld.so.cache文件；
  - 添加xxxxx/llvm-project-13.0.0.src/build/lib
  - 具体参考：https://blog.csdn.net/david_xtd/article/details/7625626



## 项目使用
```
make
```

__使用make clean && make 来部署项目本身，否则有时候会炸__
运行compiler，目前会运行正常yyparse, 还没有接入代码生成的部分。



## 语法逻辑

yacc自底向上运行。

```
entry（总体） -> areas

areas -> areas | area

area -> 全局变量区域，函数原型区域（声明），函数定义区域)

区域 -> TODO
```

继续向下推理。

一般分为两类代码：声明类和实现类。

- 在ast.h中，实现类的一个私有成员是原型类（prototype/proto/declare）。例如函数实现的一个成员变量是函数原型类。

- 变量也类似，有变量原型类。

> 现在的例子是推理出一个inti=1;或者inti=j;的

---
---
---
---
###  过时的

```
g++ `llvm-config --cxxflags` main.cpp y.tab.cpp lex.yy.cpp
```

已有的代码的使用方式：
```
lex compiler.l
yacc -dv compiler.y
g++ lex.yy.c y.tab.c -o example
```
运行example，可以解析形如`int[number]`或者`tni[number]`的字符串

