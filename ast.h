// 用于控制语法分析树的结点/BasicBlock
#ifndef C2LLVM_AST_HPP
#define C2LLVM_AST_HPP
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>

#include <iostream>
#include <vector>

class ASTContext;

class ASTFunctionProto;
class ASTFunctionImp;

class ASTExpression;
class ASTGeneralExpression;

class ASTNode {
  // 所有AST结点的基类
 public:
  ASTNode() {}
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

class ASTStatement : public ASTNode {
  // 所有AST声明的基类
 public:
  ASTStatement() {}
  virtual llvm::Value* generate(ASTContext& context) override {}
  virtual std::string get_class_name() { return "ASTStatement"; }
};

class ASTExpression : public ASTNode {
  // 所有AST表达式的基类
 public:
  ASTExpression() {}
  virtual llvm::Value* generate(ASTContext& context) override {}
  virtual std::string get_class_name() { return "ASTExpression"; }
};


class ASTGeneralStatement : public ASTStatement {
  // 常规的声明
 public:
  ASTGeneralStatement() {}
  llvm::Value* generate(ASTContext& context) override {}
  std::string get_class_name()  { return "ASTGeneralStatement"; }
};

class ASTGeneralExpression : public ASTExpression {
  // 常规的表达式
 public:
  ASTGeneralExpression() {}
  llvm::Value* generate(ASTContext& context) override {}
  std::string get_class_name()  { return "ASTGeneralExpression"; }
};

class ASTBasicBlock : public ASTExpression {
  // 基本块: 区分于LLVM的BasicBlock类
 public:
  llvm::BasicBlock* BB;
  llvm::Value* generate(ASTContext& context) override {}
};

class ASTIdentifier : public ASTExpression {
  // 对应了一个表达式的左值, 一个identifier
 private:
  std::string name;
  // TODO: 对于数组应该另外处理
  bool is_array;     // TODO： 这个表达式可能是个数组本身。
  int array_length;  // 数组的长度

 public:
  llvm::Value* generate(ASTContext& context) override {}
};

class ASTVariableProto : public ASTStatement {
  // 给定了一个形如type left = right的变量声明
 private:
  int type;  // 参数类型
  ASTIdentifier left;
  ASTGeneralExpression right;

 public:
  llvm::Value* generate(ASTContext& context) override {}
};

typedef ASTVariableProto ARGproto;  // 参数声明
typedef std::string ARGname;    // 参数名称
class ASTFunctionProto : public ASTStatement {
  // 声明了一个形如ret_type name(args)的函数
 private:
  std::string name;
  int ret_type;
  std::vector<std::tuple<ARGproto, ARGname>> args;

 public:
  llvm::Value* generate(ASTContext& context) override {}
};

class ASTFunctionImp : public ASTExpression {
  // 实现了一个函数，它的声明是prototype, 函数的入口是function_entry;
 private:
  ASTFunctionProto* prototype;
  ASTBasicBlock* function_entry;

 public:
  llvm::Value* generate(ASTContext& context) override {}
};

#endif
