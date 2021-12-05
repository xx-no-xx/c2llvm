%{
	#include <stdio.h>
	#include <cstdlib>
	extern int yylex(void); // 被yyparse()调用
	extern int yyparse(void);  //提前声明

	void yyerror(const char* s) { // 用来报错
		printf("ERROR-IN-YACC: %s\n", s);
	}

	int yywrap() { // ? TODO: 目的不明
		return 1;
	}

	int main() { // TODO: 测试用，为了实验它是否ok
		yyparse();
		return 0;
	}
%}

// 扩展的yylval， yylval会在lex和yacc中传递
%union{ 
  char* type;  // TODO: 我们似乎只能用char, 学长使用了stirng, 待研究是怎么实现的
	int number; // number是int型的
}

%token <type> CONSTANT
// %token <xxx> 制定token从yylval.xxx获取
%token <number> INT

//%start commands // 最开始的规则
/* 这两个似乎是为了除去二义性的 */
// %nonassoc LOWER_THAN_ELSE  
// %nonassoc ELSE
%%

/* 接下来是语法部分 */
/* 这里可能需要认真研究一下 */ 

/* 最简单的一个文法例子 */
/* 运行这个东西，除非一直输入int[一个数字]这样，否则就会提示syntax error */
commands : /* commands -> commands | command */
| commands command
;
/* 这里似乎会有递归左右的问题 */

command :
INT CONSTANT { 
	// 匹配上了command->INTCONSTANT后
	// 在这里写要干什么
	printf("i found a 'intconstant' struct: <token>%d, <type>%s", $1, $2);
	// $x编号从1开始是参数
	// $1对应的INT是<number>，所以自己选择了number里存的值
	// $2对应的是CONSTANT是<type>，所以选择了type里存的值
	// 考虑之后在这里写，有关中间代码生成的函数: 依赖于LLVM，生成LLVM IR
}


/* 语法部分结束 */
%%

/* 这后面可以实现，一些出现在最开始部分的函数 */