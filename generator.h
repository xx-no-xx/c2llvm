// 用于实际生成llvm ir的generator
#ifndef C2LLVM_GENERATOR_HPP
#define C2LLVM_GENERATOR_HPP
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <iostream>
#include <stack>
#include <vector>

#include "ast.h"

class ASTContext {
 private:
  std::stack<ASTCodeBlockExpression *> codestack;  // 处理跨函数而使用的符号表
 public:
  llvm::LLVMContext *context;  // 上下文
  llvm::IRBuilder<> *builder;  // 用来造IR的工具
  llvm::Module *current_m;  // 当前所在的module，大概可以理解为当前程序
  llvm::Function *current_f;

  ASTContext() {
    context = new llvm::LLVMContext();
    current_m = new llvm::Module("foo_module", *context);
    builder = new llvm::IRBuilder<>(*context);
    current_f = nullptr;
    std::cout << "AST Context made" << std::endl;
  }

  void push_codeblock(
      ASTCodeBlockExpression *codeblock) {  // 加入代码块，并复制已有的符号表
    if (!codestack.empty()) {
      codeblock->copy_symbol_from(codestack.top());
    }
    codestack.push(codeblock);
  }
  void pop_codeblock(void) { codestack.pop(); }  // 释放当前代码块
  ASTCodeBlockExpression *get_codestack_top(void) {
    return codestack.top();
  };  // 获取最新（当前的代码块）

  // TODO: 支持全局变量/支持多函数

  void clear_symboltable(void);
  //  void load_argument();
  // TODO: 载入函数的参数

  llvm::Value *create_local_var(int type, std::string);
  // 创建一个局部变量。
  // 如果重名，返回nullptr;

  llvm::Value *get_var(std::string);
  // 获取一个变量。如果不存在，返回nullptr;

  llvm::Type *get_type(int type);
  // 通过我们自定义的类型，返回llvm的type

  // TODO: ERROR WARNING! 用于输出警告的错误函数
};

#endif