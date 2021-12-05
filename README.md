# c2llvm



分为三个部分
- compiler.l
  - 词法分析
- compiler.y
  - 获取lex分析的词法，然后做语法分析
  - 调用另外的方法，生成llvm-ir。这部分的生成依赖于llvm库（暂时用的13.0.0的版本？）
- 代码生成方法
  - 依赖于llvm库
  - 例如，在compiler.y中，发现它是一个赋值的式。那么，在对应的{}操作中调用命令。

> 与代码生成方法有关逻辑，暂时还不清楚

已有的代码的使用方式：
```
lex compiler.l
yacc -dv compiler.y
g++ lex.yy.c y.tab.c -o example
```
运行example，可以解析形如`int[number]`或者`tni[number]`的字符串