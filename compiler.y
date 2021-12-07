%{
	#include "ast.h"
	#include "generator.h"
	#include <string>
	#include <iostream>
	extern int yylex(void); // 被yyparse()调用
	extern int yyparse(void);  //提前声明

	void pass(); // pass : 啥也不干

	ASTBasicBlock* entryBB = nullptr; // entry的"基本块"

	void yyerror(const char* s) { // 用来报错
		printf("ERROR-IN-YACC: %s\n", s);
	}

	int yywrap() { // ? TODO: 目的不明
		return 1;
	}


//	int main() { // TODO: 测试用，为了实验它是否ok
//		yyparse();
//		return 0;
//	}
%}

// 扩展的yylval， yylval会在lex和yacc中传递
%union{ 
	ASTBasicBlock* BB;
	ASTFunctionDec* function_dec; // 函数声明
	ASTFunctionImp* function_imp; // 函数实现
	ASTGeneralExpression* expression; // 表达式
	ASTGeneralStatement* statement; // 声明
	ASTVariableDec* variable_dec; // 
	int int_value; // 存储int型const的实际值
	std::string *str_value; // 存储identifier的实际值
  std::string *type;  // TODO: 存储type, 后续为了效率可以修改为int 
}

%token <type> INT
%token <type> ASSIGN SEMICOLON
%token <int_value> INT_CONSTANT
%token <str_value> IDENTIFIER
// %token <xxx> 制定token从yylval.xxx获取

// 非终结符
%type <BB> entry areas area
%type <function_dec> function_declaration // 函数声明
%type <function_imp> function_implementation // 函数实现
%type <expression> test_expression test_expression_another// 用于测试的表达式
// 表示这些非终结符，应该从哪里获取yylval

%start entry // 最开始的规则

/* 这两个似乎是为了除去二义性的 */
// %nonassoc LOWER_THAN_ELSE  
// %nonassoc ELSE

%%


entry: areas {entryBB = $1; } // 把语法分析树中entry的指针交给entryBB, 因为yacc是从底向上的，这里会被最后访问。此时，basicblock已经搭建好，于是把它交给entryBB，供后续的代码生成。

areas: area // 将程序划分为若干个区域
| areas area

area: function_implementation {pass();/* dosomething */}
| function_declaration {pass();/* dosomething */}
// | global_variable SEMICOLON {/* dosomething */}

function_implementation : test_expression {pass(); /* dosomethign */ }
function_declaration : test_expression_another { pass();  /* dosomething */ }
// TODO: 全局变量声明 global_variable : { /* dosomething */ }

// TODO: 测试用
test_expression :
	INT IDENTIFIER ASSIGN INT_CONSTANT SEMICOLON { 
		// 案例：匹配一个inti=1;的语句 
		// identifer的长度为1
		// 数字的长度为1
		std::cout << "match a expression:" << *$1 << " " << *$2 << " " << *$3 << " " << $4 << " " << *$5 << std::endl;
	}

// TODO: 测试用
test_expression_another :
	INT IDENTIFIER ASSIGN IDENTIFIER SEMICOLON {
		// 案例：匹配一个inti=j;的语句
		// identifer的长度为1
		std::cout << "match another expression:" << *$1 << " " << *$2 << " " << *$3 << " " << *$4 << " " << *$5 << std::endl;
	}

/* 接下来是语法部分 */

//#  /* 最简单的一个文法例子 */
//#  /* 运行这个东西，除非一直输入int[一个数字]这样，否则就会提示syntax error */
//#  
//#  commands : /* commands -> commands | command */
//#  | commands command
//#  ;
//#  
//#  /* 这里似乎会有递归左右的问题 */
//#  
//#  command :
//#  INT CONSTANT { 
//#  	// 匹配上了command->INTCONSTANT后
//#  	// 在这里写要干什么
//#  //	printf("i found a 'intconstant' struct: <token>%d, <type>%s", $1, $2);
//#  	std::cout << "I found a 'intconstant' struct" << $1 << " " << $2 << std::endl;
//#  	// $x编号从1开始是参数
//#  	// $1对应的INT是<number>，所以自己选择了number里存的值
//#  	// $2对应的是CONSTANT是<type>，所以选择了type里存的值
//#  	// 考虑之后在这里写，有关中间代码生成的函数: 依赖于LLVM，生成LLVM IR
//#  }


/* 语法部分结束 */
%%

/* 这后面可以实现，一些出现在最开始部分的函数 */

void pass() {
	return;
}