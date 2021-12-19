#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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

extern FILE* yyin;
extern int yyparse();  // yacc & lex 以分析现有的文件
// extern entry* entry;
// extern ASTCodeBlockExpression* entry;
njson json_result;

void test_ast(ASTContext* context) {
  // 造一个空的函数参数列表
  std::vector<std::pair<int, ARGname> > args;
  args.clear();
  // 造printf
  args.push_back(std::make_pair(TYPE_CHAR_PTR, "s"));
  auto print = new ASTFunctionProto(TYPE_INT, "printf", args, true);
  print->generate(context);
  args.clear();
  //造add
  args.push_back(std::make_pair(TYPE_INT, "a"));
  args.push_back(std::make_pair(TYPE_INT, "b"));
  auto funcadd = new ASTFunctionProto(TYPE_VOID, "add", args);
  auto addentry = new ASTCodeBlockExpression();
  auto addimp = new ASTFunctionImp(funcadd, addentry);
  // c=a+b
  auto a = new ASTVariableExpression("a");
  auto b = new ASTVariableExpression("b");
  auto c = new ASTVariableExpression("c");
  auto calc = new ASTBinaryExpression(OP_BI_ADD, a, b);
  auto defc = new ASTVariableDefine(TYPE_INT, c, nullptr);
  auto assignc = new ASTVariableAssign(c, calc);
  addentry->append_code(defc);
  addentry->append_code(assignc);
  //造字符串

  // call printf
  std::vector<ASTExpression*> printArgs;
  auto formatStr = new ASTGlobalStringExpression("%d\n");
  printArgs.push_back(formatStr);
  printArgs.push_back(c);
  auto callprint = new ASTCallExpression("printf", printArgs);
  addentry->append_code(callprint);
  // args
  addimp->generate(context);
  args.clear();
  // 造一个函数原型
  auto funcproto = new ASTFunctionProto(TYPE_VOID, "main", args);

  // 造一个函数的入口代码块
  auto entry = new ASTCodeBlockExpression();

  // 造一个真正的函数实现
  auto funcimp = new ASTFunctionImp(funcproto, entry);

  // 如果碰到了一个纯粹的数字:
  int value = 1;
  auto integer = new ASTInteger(value);
  // 和另一个纯的数字:

  char value2 = 'a';
  auto integer2 = new ASTInteger(value2);

  auto integer3 = new ASTInteger(2);

  // 造了一个左值变量
  auto x = new ASTVariableExpression("x", true, 10);
  auto defexp = new ASTVariableDefine(TYPE_INT, x, nullptr);
  entry->append_code(defexp);

  auto x1 = new ASTArrayExpression("x", integer);  // x[1]

  auto assexp = new ASTArrayAssign(x1, integer2);  // x[1] = 97;
  entry->append_code(assexp);

  auto x2 = new ASTArrayExpression("x", integer3);  // x[2]
  auto ass2exp = new ASTArrayAssign(x2, x1);        // x[2] = x[1]
  entry->append_code(ass2exp);

  vector<ASTExpression*> Args;
  Args.clear();
  Args.push_back(static_cast<ASTExpression*>(x1));
  Args.push_back(static_cast<ASTExpression*>(x2));
  auto callAdd = new ASTCallExpression("add", Args);
  entry->append_code(callAdd);

  /*
    // 将它定义出来，值为上述的加法表达式
    auto defexp = new ASTVariableDefine(
        TYPE_INT, x,
        dynamic_cast<ASTExpression*>(integer2));  // int lhs = 100;

    entry->append_code(defexp);

    auto delexp = new ASTBinaryExpression(OP_BI_SUB, x, integer);
    auto assexp = new ASTVariableAssign(x, delexp);

    auto whilecode = new ASTCodeBlockExpression();
    whilecode->append_code(assexp);

    auto whileexp = new ASTWhileExpression(x, whilecode);

    entry->append_code(whileexp);

    auto assexp2 = new ASTVariableAssign(x, integer2);

    auto preparecode = new ASTCodeBlockExpression();
    preparecode->append_code(assexp2);

    auto actioncode = new ASTCodeBlockExpression();
    actioncode->append_code(delexp);

    auto forcode = new ASTCodeBlockExpression();
    auto forexp = new ASTForExpression(x, forcode, preparecode, actioncode);


    entry->append_code(forexp); */
  //  auto whileexp = new AST

  /* 下面是非常不好的实例 */
  /* 不要用，之后可以删除 */

  /*
  auto if_code = new ASTCodeBlockExpression();
  auto else_code = new ASTCodeBlockExpression();
  auto if_code_2 = new ASTCodeBlockExpression();
  auto else_code_2 = new ASTCodeBlockExpression();

  auto y = new ASTVariableExpression("y");

  auto defexp2 =
      new ASTVariableDefine(TYPE_INT, y, dynamic_cast<ASTExpression*>(integer));

  auto assexp = new ASTVariableAssign(y, x);

  entry->append_code(dynamic_cast<ASTNode*>(defexp));
  entry->debug();
  if_code->append_code(dynamic_cast<ASTNode*>(defexp2));
  if_code_2->append_code(dynamic_cast<ASTNode*>(defexp2));

  else_code->append_code(dynamic_cast<ASTNode*>(defexp2));
  else_code->append_code(dynamic_cast<ASTNode*>(assexp));

  else_code_2->append_code(dynamic_cast<ASTNode*>(defexp2));
  else_code_2->append_code(dynamic_cast<ASTNode*>(assexp));

  auto ifexp = new ASTIfExpression(x, if_code, else_code);

  auto ifexp_a = new ASTIfExpression(x, if_code_2, else_code_2);

  auto bb = new ASTCodeBlockExpression();

  bb->append_code(ifexp);

  auto bb2 = new ASTCodeBlockExpression();

  bb2->append_code(ifexp_a);

  auto ifexp2 = new ASTIfExpression(x, bb, bb2);

  entry->append_code(dynamic_cast<ASTNode*>(ifexp2)); */

  funcimp->debug();
  funcimp->generate(context);  // 生成函数的代码

  json_result = funcimp->generate_json();

  // 上面这一部分是应该在yacc中完成的。现在仅仅是测试用
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
  /* --------------------------------------------------------------- */
}

void test_gen(ASTContext* context) {
  entryCodeBlock->generate_from_root(context);
  entryCodeBlock->debug();
  json_result = entryCodeBlock->generate_json();
}

int main(int argc, char** argv) {
  if (argc <= 1) {
    std::cout << "panic: no argument" << std::endl;
    return 0;
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

      std::ofstream out;
      out.open("output.json", ios::out);
      out << json_result.dump(4);
      out.close();
    } else {
      std::error_code EC;
      llvm::raw_fd_ostream dest(argv[2], EC, llvm::sys::fs::OF_None);
      context->current_m->print(dest, nullptr);
      std::ofstream out;
      out.open("output.json", ios::out);
      out << json_result.dump(4);
      out.close();
    }
  } else if (strcmp(argv[1], "gen") == 0) {
    if (argc <= 2) {
      std::cout << "panic: no input" << std::endl;
      exit(0);
    }

    yyin = fopen(argv[2], "r");
    if (!yyin) {
      std::cout << "panic: wrong input" << std::endl;
      exit(0);
    }
    yyparse();

    auto context = new ASTContext();
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    context->current_m->setTargetTriple(TargetTriple);  // 生成目标平台的信息

    test_gen(context);
    std::ofstream out;
    out.open("output.json", ios::out);
    out << json_result.dump(4);
    out.close();

    if (argc <= 3) {
      context->current_m->print(llvm::errs(), nullptr);  // 输出测试
      std::cout << json_result.dump(4) << std::endl;
    } else {
      std::error_code EC;
      llvm::raw_fd_ostream dest(argv[3], EC, llvm::sys::fs::OF_None);
      context->current_m->print(dest, nullptr);
    }

  } else {
    std::cout << "panic: invalid argument" << std::endl;
  }
  return 0;
}
