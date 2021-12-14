%{
	#include "ast.h"
	#include "generator.h"
	#include <string>
	#include <iostream>
	extern int yylex(void); // 被yyparse()调用
	extern int yyparse(void);  //提前声明

	void pass(); // pass : 啥也不干

	ASTCodeBlockExpression* entryBB = nullptr; // entry的"基本块"

	void yyerror(const char* s) { // 用来报错
		printf("ERROR-IN-YACC: %s\n", s);
		extern int yylineno;	// defined and maintained in lex
		extern char *yytext;	// defined and maintained in lex
		int len=strlen(yytext);
		int i;
		char buf[512]={0};
		for (i=0;i<len;++i){
			sprintf(buf,"%s%d ",buf,yytext[i]);
		}
		fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);
		yyparse();
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
	ASTCodeBlockExpression* BB;
	ASTFunctionProto* function_proto; // 函数声明
	ASTFunctionImp* function_imp; // 函数实现
	ASTExpression* expression; // 表达式
	ASTPrototype* statement; // 声明
	ASTVariableDefine* variable_define; // 变量的定义，形如int x = 1;
	ASTVariableExpression* identifier;
	int int_value; // 存储int型const的实际值
	std::string *str_value; // 存储identifier的实际值
	std::string *char_value;	// char型的实际值
	double double_value;	// double型的实际值
  	std::string *type;  // TODO: 存储type, 后续为了效率可以修改为int
}

%token <type> INT CHAR DOUBLE VOID
%token <type> TADD TSUB TMUL TDIV TMOD TLSFT TRSFT TBIT_OR TBIT_AND TBIT_XOR TBIT_NOT
%token <type> TAND TOR TCEQ TCNE TCLT TCLE TCGT TCGE
%token <type> TLPAREN TRPAREN TLBRACKET TRBRACKET TLBRACE TRBRACE
%token <type> TCR TCOMMA TDOT
%token <type> IF ELSE WHILE BREAK CONTINUE RETURN
%token <type> ASSIGN SEMICOLON
%token <int_value> INT_CONSTANT
%token <str_value> IDENTIFIER
%token <char_value> CHAR_CONSTANT
%token <double_value> DOUBLE_CONSTANT
// %token <xxx> 制定token从yylval.xxx获取

// 非终结符
%type <BB> entry areas area code_block
%type <function_proto> function_prototype // 函数声明
%type <function_imp> function_implementation // 函数实现
%type <identifier> var_name
%type <expression> test_expression test_expression_another// 用于测试的表达式
%type <expression> basic_expression postfix_expression primary_expression sum_expression calculate_expression compare_expression
%type <variable_define> var_defination
%type <type> type_specifier
// 表示这些非终结符，应该从哪里获取yylval

%start entry // 最开始的规则

/* 这两个似乎是为了除去二义性的 */
// %nonassoc LOWER_THAN_ELSE  
// %nonassoc ELSE

%%


entry: areas {entryBB = $1; } // 把语法分析树中entry的指针交给entryBB, 因为yacc是从底向上的，这里会被最后访问。此时，codeblock已经搭建好，于是把它交给entryBB，供后续的代码生成。

// 将程序划分为若干个区域
areas: area {$$ = new ASTCodeBlockExpression();
 	$$->append_code($1);}
	| areas area {$1->append_code($2);}
	;

area: function_implementation {
		pass();/* dosomething */
	}
	| function_prototype {
		pass();/* dosomething */
	}
	;
// | global_variable SEMICOLON {/* dosomething */}

// function_implementation : test_expression {pass(); /* dosomethign */ }

// function_implementation: 
//  	type_specifier identifier TLPAREN func_arguments TRPAREN 
function_implementation: 
	/* code_block {
		// 暂时这么写，不支持函数
		std::cout << "func_imple" << std::endl;
	} */
	code_block
	| test_expression
	;

code_block: 
	TLBRACE TRBRACE {
		new ASTCodeBlockExpression();
		std::cout << "code_block" << std::endl;
	}
	| calculate_expression

function_prototype : test_expression_another { 
	pass();  
	$1 = new ASTGeneralExpression();
	std::cout << "TEST:" << $1->get_class_name() << std::endl;
	/* dosomething */ 
}
// TODO: 全局变量声明 global_variable : { /* dosomething */ }

// 变量名，即标识符
var_name: 
	IDENTIFIER{
		std::cout << "var_name" << std::endl;
		$$ = new ASTVariableExpression(*$1);
	}

// 变量的声明和定义，若未初始化，为nullptr
var_defination: 
	type_specifier var_name { $$ = new ASTVariableDefine((int)(*$1)[0], $2, nullptr);}
	| type_specifier var_name ASSIGN calculate_expression{
		$$ = new ASTVariableDefine((int)(*$1)[0], $2, $4);
	}
	| type_specifier var_name TLBRACKET INT_CONSTANT TRBRACKET{
		// 数组
		// TODO: $2->is_array = true; 目前访问不到
		// TODO: $2->array_length = INT_CONSTANT; 目前访问不到
		$$ = new ASTVariableDefine((int)(*$1)[0], $2, nullptr);
	}
	;

// 基本表达式，如变量名、常量
basic_expression:
	var_name {}
	| INT_CONSTANT {
		$$ = new ASTInteger($1);
	}
	| CHAR_CONSTANT {
		char result;
		int length = (*$1).size();
		std::string number_str;
		if((*$1)[1] != '\\') {	// 处理\',\n等特殊字符
			result = (*$1)[1];
		}
		else{
			for (int i = 2; i < length && (*$1)[i] != '\''; i++)
				number_str.push_back((*$1)[i]);
			result = (char)atoi(number_str.c_str());
		}
		// TODO: $$ = new ASTCharExpression(result);
	}
	| DOUBLE_CONSTANT {
		// TODO: $$ = new ASTDoubleExpression($1);
	}
	;

// 后缀表达式，如a, a[10], foo(a, b)，目前仅有数组
postfix_expression: 
	basic_expression {$$ = $1;}
	| var_name TLBRACKET INT TRBRACKET {
		// TODO: 增加数组的AST
	}
	;

// 一元表达式，如!a, a!, *a，目前等价于后缀表达式
primary_expression:
	postfix_expression {$$ = $1;}
	;

// 所有的计算式，都可分解为和式的加减
calculate_expression:
	sum_expression{ $$ = $1; }
	| calculate_expression TADD sum_expression{
		// TODO: new 一个二元运算的node
		// $$ = $1 + $3;
		$$ = new ASTBinaryExpression((*$2)[0], $1, $3);
	}
	| calculate_expression TSUB sum_expression{
		// TODO: new 一个二元运算的node
		// $$ = $1 - $3;
		$$ = new ASTBinaryExpression((*$2)[0], $1, $3);
	}
;

// 和式 a*b 或 a/b, a%b
sum_expression:
	primary_expression {$$ = $1;}
	| sum_expression TMUL primary_expression {
		// $$ = $1 * $3;
		// TODO: new 一个二元运算的node
		$$ = new ASTBinaryExpression((*$2)[0], $1, $3);
	}
	| sum_expression TDIV primary_expression {
		// $$ = $1 / $3;
		// TODO: new 一个二元运算的node
		$$ = new ASTBinaryExpression((*$2)[0], $1, $3);
	}
	| sum_expression TMOD primary_expression {
		// $$ = $1 % $3;
		// TODO: new 一个二元运算的node
		$$ = new ASTBinaryExpression((*$2)[0], $1, $3); 
	}
	;

// 比较式
compare_expression:
	calculate_expression {$$ = $1;}
	| calculate_expression TCLT calculate_expression{/*TODO: new 一个二元运算的node*/} // <
	| calculate_expression TCLE calculate_expression{/*TODO: new 一个二元运算的node*/} // <=
	| calculate_expression TCGT calculate_expression{/*TODO: new 一个二元运算的node*/} // >
	| calculate_expression TCGE calculate_expression{/*TODO: new 一个二元运算的node*/} // >=
	| calculate_expression TCEQ calculate_expression{/*TODO: new 一个二元运算的node*/} // ==
	| calculate_expression TCNE calculate_expression{/*TODO: new 一个二元运算的node*/} // !=

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

/* 变量类型 */
type_specifier:
	VOID {$$ = $1; }
	| INT {$$ = $1; }
	| CHAR {$$ = $1; }
	| DOUBLE {$$ = $1; }
	;

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