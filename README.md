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
$ make
```
- 需要设置环境变量
  - 一个脚本（第一个冒号前可能就够了，为避免问题全部加上）
``` 
export LLVM_HOME=/home/glassesq/llvm-project-13.0.0.src/build/bin:/home/glassesq/llvm-project-13.0.0.src/lib:/home/glassesq/llvm-project-13.0.0.src/build/lib
export PATH=$LLVM_HOME:$PATH
echo $PATH
echo "llvm path ok"
```
- 需要设置动态库
  - 添加动态库： https://blog.csdn.net/smilejiasmile/article/details/84307526
  - 添加xxxxx/llvm-project-13.0.0.src/build/lib

```
g++ `llvm-config --cxxflags` main.cpp y.tab.cpp lex.yy.cpp
```

## 项目使用
```
make
```
运行compiler，目前会运行正常yyparse, 还没有接入代码生成的部分。



---
---
---
---
###  过时的

已有的代码的使用方式：
```
lex compiler.l
yacc -dv compiler.y
g++ lex.yy.c y.tab.c -o example
```
运行example，可以解析形如`int[number]`或者`tni[number]`的字符串

