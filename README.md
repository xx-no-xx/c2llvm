# c2llvm

__建议不要把lex和yacc输出的东西扔上来__

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
```sh
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
编译：
```
make
```

__使用make clean && make 来部署项目本身，否则有时候会炸__

__运行compiler, 三种行为：__

`./compiler parse`: 运行yacc和lex的原始逻辑，yyparse();

`./compiler test`: 输出test_ast生成的IR到console

`./compiler test any_file_name.ll`: 输出test_ast生成的IR到对应的文件

补充：使用`clang file.ll -o file`生成可执行文件

main函数说明：

`test_ast()`函数用来模拟AST的生成与IR的生成

`yyparse()`调用lex和yacc来解析输入，对应compiler.l和compiler.y。最后项目中，应该在这一步完成AST的构造。

## 语法逻辑

yacc自底向上运行, 按照下列语法。

语法不能有冲突，不能有两条路径通向同一个地方。

```
entry（总体） -> areas

areas -> areas | area

area -> 全局变量区域，函数原型区域（声明），函数定义区域)

区域 -> TODO
```

类似如此，继续向下推理。


## 代码生成

总分类为两类代码：原型类和实现类，在ast.h中声明，有相应注释。

- 在ast.h中，实现类的一个私有成员是原型类（prototype/proto/declare）。例如函数实现的一个成员变量是函数原型类。

- 变量也类似，有变量定义类，和变量表达式类。前者定义了一个变量，例如int i = 1;后者单独指i。

`test_ast`给出了一个造AST的例子。yacc应该达到类似的效果。操作顺序不是问题，我可以另写api。

## 变量接口说明

变量：
左值：
```
int x;

// 先解析x为ASTVariableExpression exp
// 然后解析int x为ASTVariableDefine(type, exp, nullptr)

int x = 1;                       

// 先解析x为ASTVariableExpression exp
// 在把1解析为ASTExpression rhs
// ASTVariableDefine(type, exp, rhs), 


x = 1;                       
// 先解析x为ASTVariableExpression lhs
// 在把1解析为ASTExpression rhs
// 最后x=1解析为 ASTVariableAssign(lhs, rhs)
```
右值：
```
..... = x;                       
// 解析x为ASTVariableExpression()
```

数组本身：
左值：
```
int x[10000];    
// 先解析x[10000]为ASTVariableExpression exp（name="x", is_array = true, array_length = 10000）
//  然后解析int x[10000]为ASTVariableDefine(type_int, exp, nullptr)
 ```

右值：
```
（函数调用的参数）foo(x);
// 解析x为ASTVariableExpression, is_array和array_length不用管

（函数调用的参数，需要取地址，todo）scanf(&x); 
// todo
```

数组元素：
左值：
```
x[a + b] = 1; 
// 先解析a + b为ASTExpression*(BinaryExpression, Integer，具体看情况) indexexp,
// 然后解析x[a + b]为ASTArrayExpression(name="x", index=indexexp) left;
// 解析1为ASTInteger(ASTExpression) right;
// 再然后解析x[a + b] = 1为ASTArrayAssign(lhs = left, rhs = right); 
```

右值：
```
.... = x[a+b];
// 先解析a + b为ASTExpression*(BinaryExpression, Integer，具体看情况) indexexp,
// 然后解析x[a + b]为ASTArrayExpression(name="x", index=indexexp) left;
```

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

