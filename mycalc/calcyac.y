%{
#include <stdio.h>
#include <stdlib.h>
#define YYDEBUG 1
int yylex();  // 增加声明
int yyerror(const char *str); // 增加声明
%}
%union{
    int         int_value;
    double      double_value;
    int         token;
}   // 声明记号和非终结符类型
%token <double_value>       DOUBLE_LITERAL
%token <token> ADD SUB MUL DIV CR   // 记号的声明
%type <double_value> expression term primary_expression // 非终结符类型
%%  // 规则区块
line_list       // 多行规则
    : line      // 单行
    | line_list line    // 或一个多行后接单行
    ;
line            // 单行规则
    : expression CR // 表达式后接换行符
    // 动作
    {
        printf(">>%lf\n", $1);
    }
expression      // 表达式
    : term      // 和项
    | expression ADD term   // 或 表达式+ 和项
    {
        $$ = $1 + $3;
    }
    | expression SUB term   // 或 表达式- 和项
    {
        $$ = $1 - $3;
    }
    ;   
term            // 和项的规则
    : primary_expression    // 一元表达式
    | term MUL primary_expression   // 或 和项 * 一元表达式
    {
        $$ = $1 * $3;
    }
    | term DIV primary_expression   // 或 和项 / 一元表达式
    {
        $$ = $1 / $3;
    }
    ;   // 这里体现了优先级！！
primary_expression  // 一元表达式规则
    :  DOUBLE_LITERAL   // 实数的字面常量
    ;
%%
int 
yyerror(char const *str){
    extern char *yytext;
    fprintf(stderr, "parser error near %s\n", yytext);
    return 0;
}
    
int main(void){
    extern int yyparse(void);
    extern FILE *yyin;

    yyin = stdin;
    if(yyparse()){
        fprintf(stderr, "Error !\n");
        exit(1);
    }
}