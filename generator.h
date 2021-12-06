// 用于实际生成llvm ir的generator
#ifndef C2LLVM_GENERATOR_HPP
#define C2LLVM_GENERATOR_HPP
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include <iostream>
#include <vector>

#include "ast.h"

class ASTContext {
 public:
  llvm::LLVMContext* context;  // 上下文
  llvm::IRBuilder<> builder;   // 用来造IR的工具

  /*
  TODO:
    支持表达式
    llvm::Module* current_m;
    llvm::Function* cuurent_f;
    std::vector<ASTBasicBLock*> BBstack;
  */
};

#endif