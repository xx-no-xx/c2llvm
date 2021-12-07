#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "ast.h"
#include "generator.h"

using namespace std;

extern int yyparse();  // yacc & lex 以分析现有的文件
// extern entry* entry;
// extern ASTCodeBlockExpression* entry;

void test_ast() {
  // 造一个空的函数参数列表
  std::vector<std::pair<ARGdefine, ARGname>> args;
  args.clear();

  // 造一个函数原型
  auto funcproto = new ASTFunctionProto(TYPE_VOID, "foo_function", args);

  // 造一个函数的入口代码块
  auto entry = new ASTCodeBlockExpression();

  // 造一个真正的函数实现
  auto funcimp = new ASTFunctionImp(funcproto, entry);

  // 如果碰到了一个纯粹的数字:
  int value = 100;
  auto integer = new ASTInteger(value);
  // 和另一个纯的数字:

  int value2 = 200;
  auto integer2 = new ASTInteger(value2);
  // 现在造一个200 + 100的二元运算：
  auto expression =
      new ASTBinaryExpression('+', dynamic_cast<ASTExpression*>(integer),
                              dynamic_cast<ASTExpression*>(integer2));

  // 造了一个左值变量
  auto lhs = new ASTVariableExpression("lhs");

  // 将它定义出来，值为上述的加法表达式
  auto defexp = new ASTVariableDefine(TYPE_INT, lhs,
                                      dynamic_cast<ASTExpression*>(expression));

  // 把这个变量放进上述的codeblock,
  // 首先用dynamic_cast把它强转为需要的指针类型。如果转换不成功，ptr会是nullptr
  if (auto ptr = dynamic_cast<ASTNode*>(defexp)) {
    entry->append_code(ptr);
  }


  funcimp->debug();

  // 上面这一部分是应该在yacc中完成的。现在仅仅是测试用 
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */

  // 下面我们来尝试生成
  auto astcontext = new ASTContext();
  funcimp->generate(astcontext); // 生成这个函数的IR

  std::cout << "\n\n\n\n\n\n" << std::endl;
  astcontext->current_m->print(llvm::errs(), nullptr); // 输出测试
}

int main(int argc, char** argv) {
  test_ast();
  //  yyparse();
  // 调用yacc&lex以解析输入文件。
  // 同时，AST也在.y的过程中构建起来。
  // 在这一步，entry是一个超级基本块, 它对应着AST的根结点

  // TODO： 生成代码
  //  ASTContext* context = new ASTContext();  // 创造一个ASTContext,
  //  它代表着我们的AST里的上下文 llvm::Value* code = entry->generate(context);
  //  生成代码

  return 0;
}
