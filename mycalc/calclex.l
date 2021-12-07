%{
#include <stdio.h>
#include "y.tab.h"
#define TOKEN(t) (yylval.token = t)

int yywrap(){
    return 1;
}
%}
%%
"+"         return TOKEN(ADD);
"-"         return TOKEN(SUB);
"*"         return TOKEN(MUL);
"/"         return TOKEN(DIV);
"\n"        return TOKEN(CR);
([1-9][0-9]*)|0|([0-9]\.[0-9]*) {
    // double temp;
    // sscanf(yytext, "%lf", &temp);
    yylval.double_value = atoi(&yytext[0]);
    return DOUBLE_LITERAL;
}
[ \t]
. {
    fprintf(stderr, "lexical error.\n");
    exit(1);
}
%%