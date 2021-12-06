// 用于控制语法分析树的结点/BasicBlock
#ifndef C2LLVM_AST_HPP
#define C2LLVM_AST_HPP
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>

#include <iostream>
#include <vector>

class ASTContext;

class ASTNode {
  // 所有AST结点的基类
 public:
  virtual ~ASTNode() {}
  virtual llvm::Value* generate(ASTContext& context) {
    // 生成该AST结点对应的llvmIR代码
    return nullptr;
  }
  /*
  TODO:
    json化AST
    virtual getjson(void);
  */
};

class ASTBasicBlock : public ASTNode {
  // 基本块类；
  // 区分于LLVM的BasicBlock类
 public:
  llvm::BasicBlock* BB;
  llvm::Value* generate(ASTContext& context) override;
};
#endif
