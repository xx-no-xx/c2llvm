#include <iostream>

#include "ast.h"
#include "generator.h"

using namespace std;

extern int yyparse();  // yacc & lex 以分析现有的文件
// extern entry* entry;
// extern ASTBasicBlock* entry;

int main(int argc, char** argv) {
  yyparse();
  // 调用yacc&lex以解析输入文件。
  // 同时，AST也在.y的过程中构建起来。
  // 在这一步，entry是一个超级基本块, 它对应着AST的根结点

  // TODO： 生成代码
  //  ASTContext* context = new ASTContext();  // 创造一个ASTContext,
  //  它代表着我们的AST里的上下文 llvm::Value* code = entry->generate(context);
  //  // 生成代码

  return 0;
}
