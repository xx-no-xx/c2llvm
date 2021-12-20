%{
	#include "ast.h"
	#include "generator.h"
	#include <string>
	#include <iostream>
	extern int yylex(void); // 被yyparse()调用
	extern int yyparse(void);  //提前声明
	
	void pass(); // pass : 啥也不干

	ASTCodeBlockExpression* entryCodeBlock; // entry的"基本块"
	std::vector<ASTFunctionProto*> functionDeclarations;

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
	ASTCodeBlockExpression* CodeBlock;
	ASTFunctionProto* function_proto; // 函数声明
	ASTFunctionImp* function_imp; // 函数实现
	ASTExpression* expression; // 表达式
	ASTPrototype* statement; // 声明
	ASTVariableDefine* variable_define; // 变量的定义，形如int x = 1;
	ASTVariableExpression* identifier;	// 变量名
	ASTCallExpression* func_identifier;
	ASTArrayExpression* array_identifier;
	int int_value; // 存储int型const的实际值
	std::string *str_value; // 存储identifier的实际值
	std::string *char_value;	// char型的实际值
	double double_value;	// double型的实际值
//  std::string *type;  // TODO: 存储type, 后续为了效率可以修改为int
  	int type;
	std::vector<ASTExpression*> *expression_list;	// 多行代码
	std::vector<std::pair<int, std::string> > *var_list;	// 参数列表
}

%token <type> INT CHAR DOUBLE VOID CHAR_PTR
%token <type> TADD TSUB TMUL TDIV TMOD TLSFT TRSFT TBIT_OR TBIT_AND TBIT_XOR TBIT_NOT
%token <type> TAND TOR TCEQ TCNE TCLT TCLE TCGT TCGE
%token <type> TLPAREN TRPAREN TLBRACKET TRBRACKET TLBRACE TRBRACE
%token <type> TCR TCOMMA TDOT
%token <type> IF ELSE WHILE BREAK CONTINUE RETURN FOR VARARG EXTERN
%token <type> ASSIGN SEMICOLON
%token <int_value> INT_CONSTANT
%token <str_value> IDENTIFIER STR_CONSTANT
%token <char_value> CHAR_CONSTANT
%token <double_value> DOUBLE_CONSTANT
// %token <xxx> 制定token从yylval.xxx获取

// 非终结符
%type <CodeBlock> entry areas code_block  for_assign_block
%type <function_proto> function_prototype function_declaration// 函数声明
%type <function_imp> function_implementation // 函数实现
%type <identifier> var_name
%type <array_identifier>  array_with_index_name // 形如x[i]的带索引数组变量
/* %type <func_identifier> func_with_args			// 形如foo(a, b)的函数调用 */
%type <expression> basic_expression postfix_expression primary_expression sum_expression calculate_expression compare_expression assign_expression 
%type <expression> logic_expression logic_and_expression
%type <expression> code_line assign_line func_call_exp
%type <expression_list> code_lines call_list
%type <var_list> arg_list
%type <variable_define> var_defination
%type <type> type_specifier
%type <expression> condition_exp  for_exp loop_exp  area
// 表示这些非终结符，应该从哪里获取yylval

%start entry // 最开始的规则

/* 规定了IF的匹配优先级 */
%nonassoc LOWER_THAN_ELSE  // 占位用但优先级比ELSE低
%nonassoc ELSE

%%
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

entry: areas {entryCodeBlock = $1; } // 把语法分析树中entry的指针交给entryCodeBlock, 因为yacc是从底向上的，这里会被最后访问。此时，codeblock已经搭建好，于是把它交给entryCodeBlock，供后续的代码生成。

// 将程序划分为若干个区域
areas: area {$$ = new ASTCodeBlockExpression();
 	$$->append_code($1);}
	| areas area {$1->append_code($2);}
	;

area: function_implementation {
		$$ = $1;
		//new ASTCodeBlockExpression();
//		$$->append_code($1);
	}
	| function_declaration {
		$$ = $1;
		//new ASTCodeBlockExpression();
//		$$->append_code($1);
	}
	;
// | global_variable SEMICOLON {/* dosomething */}
// TODO: 全局变量声明 global_variable : { /* dosomething */ }

// 函数声明，目前为外部 extern int bar(int a, int b);
function_declaration: EXTERN function_prototype SEMICOLON{
	// 放进一张声明表？
//	functionDeclarations.push_back($2); 
	$$ = $2;
}

// 函数实现 int bar(int a, int b){...}
function_implementation: 
	function_prototype code_block {
		// 暂时这么写，不支持函数
		$$ = new ASTFunctionImp($1, $2);
		std::cout << "func_imple" << std::endl;
	}
//	| test_expression
	;

// 函数原型 int foo(int a, int b)
function_prototype : 
	type_specifier var_name TLPAREN TRPAREN{
		$$ = new ASTFunctionProto($1, $2->get_name(), 
		std::vector<std::pair<int, std::string> >());
		std::cout << "proto empty matched" << std::endl;
	}
	| type_specifier var_name TLPAREN arg_list TRPAREN{
		$$ = new ASTFunctionProto($1, $2->get_name(), *$4);
		std::cout << "proto list matched" << std::endl;
	}
	| type_specifier var_name TLPAREN arg_list TCOMMA VARARG TRPAREN{
		$$ = new ASTFunctionProto($1, $2->get_name(), *$4, true);
		std::cout << "proto vararg matched" << std::endl;
	}
	;

// 参数列表
arg_list: 
	type_specifier var_name {
		$$ = new std::vector<std::pair<int, std::string> >();
		$$->push_back(std::make_pair($1, $2->get_name()));
	}
	| arg_list TCOMMA type_specifier var_name {
		$1->push_back(std::make_pair($3, $4->get_name()));
	}
	;

// 代码块，即被两个大括号之间括起来的部分
code_block: 
	TLBRACE TRBRACE {
		$$ = new ASTCodeBlockExpression();
		std::cout << "code_block" << std::endl;
	}
	| TLBRACE code_lines TRBRACE{
		$$ = new ASTCodeBlockExpression();
		for(auto p = $2->begin(); p != $2->end(); p++)
			$$->append_code(*p);
		std::cout << "enter code_lines" << std::endl;
	}
	;

// while循环块
loop_exp: 
	WHILE TLPAREN compare_expression TRPAREN code_block{
		$$ = new ASTWhileExpression($3, $5);
		// TODO: $$ = new ASTWhile($3, $5);
		std::cout<< "get while_loop" << std::endl;
	}

// 条件块
condition_exp:
	IF TLPAREN logic_expression TRPAREN code_block %prec LOWER_THAN_ELSE{
		$<expression>$ = new ASTIfExpression($3, $5);
		std::cout << "get if_block without else" << std::endl;
	}
	| IF TLPAREN logic_expression TRPAREN code_block ELSE code_block{
		$<expression>$ = new ASTIfExpression($3, $5, $7);
		std::cout << "get if_block with else" << std::endl;
	}

// for循环块
for_exp:
	FOR TLPAREN for_assign_block SEMICOLON compare_expression SEMICOLON for_assign_block TRPAREN code_block{
		$$ = new ASTForExpression($5, $9, $3, $7); 
	}
	| FOR TLPAREN for_assign_block SEMICOLON compare_expression SEMICOLON TRPAREN code_block{
		$$ = new ASTForExpression($5, $8, $3);
	}
	| FOR TLPAREN SEMICOLON compare_expression SEMICOLON TRPAREN code_block{
		$$ = new ASTForExpression($4, $7);
	}

// 多行语句
code_lines:
	code_line{
		$$ = new std::vector<ASTExpression*>();
		$$->push_back($1);
	}
	| code_lines code_line{
		$1->push_back($2);
	}
	;

// 单行语句(缩起来的)
code_line:
	var_defination{$<variable_define>$ = $1;} // $$ = $1会报错
	| assign_line{
		$$ = $1;
		std::cout << "get assign line" << std::endl;
	}
	| loop_exp{
		$$ = $1;
	}	// ???没有操作？疑惑
	| condition_exp{
		$$ = $1;
	}
   	| for_exp{
		$$ = $1;
	}
	| func_call_exp{
		$$ = $1;
	}
	;

// 函数调用行
func_call_exp:
	var_name TLPAREN call_list TRPAREN SEMICOLON {
		$$ = new ASTCallExpression($1->get_name(), *$3);
		std::cout << "function with args" << std::endl;
	}
	|var_name TLPAREN TRPAREN SEMICOLON {
		$$ = new ASTCallExpression($1->get_name(), 
		std::vector<ASTExpression*>());
		std::cout << "function without args" << std::endl;
	}
;

call_list:
	calculate_expression{
		$$ = new std::vector<ASTExpression*>();
		$$->push_back($1);
	}
	| call_list TCOMMA calculate_expression{
		$1->push_back($3);
	}

// 赋值运算行
assign_line:
	SEMICOLON {
		$$ = nullptr;
	}
	| assign_expression SEMICOLON{
		$$ = $1;
	}

// 变量名，即标识符
var_name: 
	IDENTIFIER{
		std::cout << "var_name" << std::endl;
		$$ = new ASTVariableExpression(*$1);
	}
	;

// 变量的声明和定义，若未初始化，为nullptr。如int i = 1;
var_defination: 
	type_specifier var_name SEMICOLON { $$ = new ASTVariableDefine($1, $2, nullptr);}
	| type_specifier var_name ASSIGN calculate_expression SEMICOLON {
		$$ = new ASTVariableDefine($1, $2, $4);
	}
	| type_specifier var_name TLBRACKET INT_CONSTANT TRBRACKET SEMICOLON {
		$2->set_array($4);
		$$ = new ASTVariableDefine($1, $2, nullptr);
	}
	// char a[10] = "abc";
/*	| CHAR var_name TLBRACKET INT_CONSTANT TRBRACKET ASSIGN STR_CONSTANT SEMICOLON {
		$2->set_array($4);
//		$$ = new ASTVaribleDefine($1, $2, ASTGlobalStringExpression(*$7));
	}*/
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
		$$ = new ASTChar(int(result));
		// TODO: 是否有需要使用char的情况
	}
	| DOUBLE_CONSTANT {
		// TODO: $$ = new ASTDoubleExpression($1);
	}
	| STR_CONSTANT {
		$$ = new ASTGlobalStringExpression(*$1);
	}
  |array_with_index_name  { $$ = $1; }
	;

// 后缀表达式，如a, a[10], foo(a, b)，目前仅有数组
postfix_expression: 
	basic_expression {$$ = $1;}
	/* | func_with_args {
		$$ = $1;
	} */
	;

// 一元表达式，如a!, *a，目前等价于后缀表达式
primary_expression:
	postfix_expression {$$ = $1;}
	| TBIT_NOT postfix_expression {
		$$ = new ASTSingleExpression(OP_SI_NOT, $2);
		std::cout << "not" << $2->get_class_name() << std::endl;
	}
	| TBIT_AND postfix_expression {
		$$ = new ASTSingleExpression(OP_SI_ADDRESS, $2);
		std::cout << "address" << $2->get_class_name() << std::endl;
	}
	;

// 所有的计算式，都可分解为和式的加减
calculate_expression:
	sum_expression{ $$ = $1; }
	| calculate_expression TADD sum_expression{
		$$ = new ASTBinaryExpression(OP_BI_ADD, $1, $3);
	}
	| calculate_expression TSUB sum_expression{
		$$ = new ASTBinaryExpression(OP_BI_SUB, $1, $3);
	}
;

// 和式 a*b 或 a/b, a%b
sum_expression:
	primary_expression {$$ = $1;}
	| sum_expression TMUL primary_expression {
		$$ = new ASTBinaryExpression(OP_BI_MUL, $1, $3);
	}
	| sum_expression TDIV primary_expression {
		$$ = new ASTBinaryExpression(OP_BI_DIV, $1, $3);
	}
	| sum_expression TMOD primary_expression {
		$$ = new ASTBinaryExpression(OP_BI_MOD, $1, $3); 
	}
	;

// 比较式
compare_expression:
	calculate_expression {$$ = $1;}
	| calculate_expression TCLT calculate_expression{
		$$ = new ASTBinaryExpression(OP_BI_LESS, $1, $3);} // <
	| calculate_expression TCLE calculate_expression{
		$$ = new ASTBinaryExpression(OP_BI_LESSEQ, $1, $3);} // <=
	| calculate_expression TCGT calculate_expression{
		$$ = new ASTBinaryExpression(OP_BI_MORE, $1, $3);} // >
	| calculate_expression TCGE calculate_expression{
		$$ = new ASTBinaryExpression(OP_BI_MOREEQ, $1, $3);} // >=
	| calculate_expression TCEQ calculate_expression{
		$$ = new ASTBinaryExpression(OP_BI_MORE, $1, $3);} // ==
	| calculate_expression TCNE calculate_expression{
		$$ = new ASTBinaryExpression(OP_BI_MOREEQ, $1, $3);} // !=
	;

// 逻辑式，一定可以拆分为逻辑与的或
logic_expression:
	logic_and_expression {$$ = $1; }
	| logic_expression TOR logic_and_expression{
		$$ = new ASTBinaryExpression(OP_BI_OR, $1, $3);
	}
	;

// 逻辑与，一堆比较式的与
logic_and_expression:
	compare_expression{ $$ = $1; }
	| logic_and_expression TAND compare_expression{
		$$ = new ASTBinaryExpression(OP_BI_AND, $1, $3);
	}
	;


// 赋值式
assign_expression:
	var_name ASSIGN calculate_expression{
		$$ = new ASTVariableAssign($1, $3);
	}
	| array_with_index_name ASSIGN calculate_expression{
		$$ = new ASTArrayAssign($1, $3);
		std::cout << "++++++++++++++++++++++++++++++++" << std::endl;
	}
	;

// a[10]
array_with_index_name:
	IDENTIFIER TLBRACKET calculate_expression TRBRACKET {
		$$ = new ASTArrayExpression(*$1, $3);
	}



for_assign_block:
	assign_expression{
		$$ = new ASTCodeBlockExpression();
 		$$->append_code($1);
	}
	| for_assign_block TCOMMA assign_expression{
		$1->append_code($3);
	}

/* 变量类型 */
type_specifier:
	VOID {$$ = TYPE_VOID; }
	| INT {$$ = TYPE_INT; }
	| CHAR {$$ = TYPE_CHAR; }
	| DOUBLE {$$ = TYPE_DOUBLE; }
	| CHAR_PTR {$$ = TYPE_CHAR_PTR; }
	;


/* 语法部分结束 */
%%

/* 这后面可以实现，一些出现在最开始部分的函数 */

void pass() {
	return;
}