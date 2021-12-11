#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "ast.h"
#include "generator.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace std;

extern int yyparse();  // yacc & lex 以分析现有的文件
// extern entry* entry;
// extern ASTCodeBlockExpression* entry;

void test_ast(ASTContext* context) {
  // 造一个空的函数参数列表
  std::vector<std::pair<ARGdefine, ARGname>> args;
  args.clear();

  // 造一个函数原型
  auto funcproto = new ASTFunctionProto(TYPE_VOID, "main", args);

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

  auto fun = new ASTVariableExpression("fun");

  auto bun = new ASTVariableExpression("bun");

  auto defexp2 =
      new ASTVariableDefine(TYPE_INT, fun, dynamic_cast<ASTExpression*>(lhs));

  auto defexp3 =
      new ASTVariableDefine(TYPE_INT, bun, dynamic_cast<ASTExpression*>(lhs));

  // 把这个变量放进上述的codeblock,
  // 首先用dynamic_cast把它强转为需要的指针类型。如果转换不成功，ptr会是nullptr
  if (auto ptr = dynamic_cast<ASTNode*>(defexp)) {  // int lhs = 100 + 300;
    entry->append_code(ptr);
  }

  if (auto ptr = dynamic_cast<ASTNode*>(defexp2)) {  // int fun = lhs;
    entry->append_code(ptr);
  }

  if (auto ptr = dynamic_cast<ASTNode*>(defexp3)) {  // int bun = lhs;
    entry->append_code(ptr);
  }

  auto divexp = new ASTBinaryExpression('/', fun, bun);

  auto assignexp = new ASTVariableAssign(fun, divexp);

  if (auto ptr = dynamic_cast<ASTNode*>(assignexp)) {  // fun = bun;
    entry->append_code(ptr);
  }

  funcimp->debug();

  funcimp->generate(context);  // 生成函数的代码

  // 上面这一部分是应该在yacc中完成的。现在仅仅是测试用
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
}

int main(int argc, char** argv) {
  if (argc <= 1) {
    std::cout << "panic: no argument" << std::endl;
  }
  if (strcmp(argv[1], "parse") == 0) {
    yyparse();
    // 调用yacc&lex以解析输入文件。
    // 同时，AST也在.y的过程中构建起来。
    // 在这一步，entry是一个超级基本块, 它对应着AST的根结点
  } else if (strcmp(argv[1], "test") == 0) {
    auto context = new ASTContext();
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    context->current_m->setTargetTriple(TargetTriple);  // 生成目标平台的信息
    test_ast(context);  // 生成AST与对应代码
    if (argc <= 2) {
      context->current_m->print(llvm::errs(), nullptr);  // 输出测试
    } else {
      std::error_code EC;
      llvm::raw_fd_ostream dest(argv[2], EC, llvm::sys::fs::OF_None);
      context->current_m->print(dest, nullptr);
    }
  } else {
    std::cout << "panic: invalid argument" << std::endl;
  }
  return 0;
}
