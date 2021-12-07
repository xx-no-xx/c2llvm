// 用于控制语法分析树的结点/BasicBlock
#ifndef C2LLVM_AST_HPP
#define C2LLVM_AST_HPP
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>

#include <iostream>
#include <utility>
#include <vector>

class ASTContext;

class ASTFunctionProto;
class ASTFunctionImp;

class ASTExpression;
class ASTGeneralExpression;

class ASTVariableExpression;

class ASTCodeBlockExpression;
class ASTFunctionProto;

#define TYPE_VOID 0
#define TYPE_INT 1

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

class ASTPrototype : public ASTNode {
  // 所有AST声明的基类
 public:
  ASTPrototype() {}
  virtual llvm::Value* generate(ASTContext& context) override {}
  virtual std::string get_class_name() { return "ASTPrototype"; }
};

class ASTExpression : public ASTNode {
  // 所有AST表达式的基类
 public:
  ASTExpression() {}
  virtual llvm::Value* generate(ASTContext& context) override {}
  virtual std::string get_class_name() { return "ASTExpression"; }
};

class ASTGeneralStatement : public ASTPrototype {
  // 常规的声明
 public:
  ASTGeneralStatement() {}
  llvm::Value* generate(ASTContext& context) override {}
  std::string get_class_name() { return "ASTGeneralStatement"; }
};

class ASTGeneralExpression : public ASTExpression {
  // 常规的表达式
 public:
  ASTGeneralExpression() {}
  llvm::Value* generate(ASTContext& context) override {}
  std::string get_class_name() { return "ASTGeneralExpression"; }
};

class ASTCodeBlockExpression : public ASTExpression {
  // 基本块: 区分于LLVM的BasicBlock类
  std::vector<ASTNode*> codes;

 public:
  ASTCodeBlockExpression() { codes.clear(); }
  llvm::BasicBlock* BB;  // ! I have NO IDEA about what's doing here
  llvm::Value* generate(ASTContext& context) override {}
  std::string get_class_name() { return "ASTCodeBlockExpression"; }
  void set_function(ASTFunctionImp*);
  void append_code(ASTNode*);
};

class ASTVariableExpression : public ASTExpression {
  // 对应了一个表达式的左值, 一个变量，例如a
 private:
  std::string name;
  // TODO: 对于数组应该另外处理
  bool is_array;     // TODO： 这个表达式可能是个数组本身。
  int array_length;  // 数组的长度

 public:
  ASTVariableExpression(std::string _name) : name(_name) {}
  llvm::Value* generate(ASTContext& context) override {}
  std::string get_class_name(void) { return "ASTVariableExpression"; }
};

class ASTVariableDefine : public ASTPrototype {
  // 给定了一个形如type lhs = rhs的变量声明
 private:
  int type;  // 参数类型
  ASTVariableExpression* lhs;
  ASTExpression* rhs;

  bool default_rhs = false;  // TODO:rhs不存在的情况

 public:
  ASTVariableDefine(int _type, ASTVariableExpression* _lhs,
                    ASTExpression* _rhs)
      : type(_type), lhs(_lhs), rhs(_rhs) {}
  llvm::Value* generate(ASTContext& context) override {}
  std::string get_class_name(void) override { return "ASTVariableDefine"; }
};

typedef ASTVariableDefine ARGdefine;  // 参数声明
typedef std::string ARGname;        // 参数名称
class ASTFunctionProto : public ASTPrototype {
  // 声明了一个形如ret_type name(args)的函数
 private:
  int ret_type;  // TODO: 现在默认不返回东西
  std::string name;
  std::vector<std::pair<ARGdefine, ARGname>> args;

 public:
  ASTFunctionProto(int _ret_type, std::string _name,
                   std::vector<std::pair<ARGdefine, ARGname>> _args)
      : ret_type(_ret_type), name(_name), args(_args) {}
  llvm::Value* generate(ASTContext& context) override {}
  std::string get_class_name(void) override { return "ASTFunctionProto"; }
  std::string get_name(void) { return name; }
};

class ASTFunctionImp : public ASTExpression {
  // 实现了一个函数，它的声明是prototype, 函数的入口是function_entry;
 private:
  ASTFunctionProto* prototype;
  ASTCodeBlockExpression* function_entry;

 public:
  ASTFunctionImp(ASTFunctionProto* _pro,
                 ASTCodeBlockExpression* _entry)
      : prototype(_pro), function_entry(_entry) {}
  llvm::Value* generate(ASTContext& context) override {}
  void set_entry(ASTCodeBlockExpression*);
};

class ASTInteger : public ASTExpression {
  // 所有整数的类，例如1, 2, 33000
 private:
  int value;

 public:
  ASTInteger(int _value) : value(_value) {}
  int get_value(void) { return value; }
};

class ASTBinaryExpression : public ASTExpression {
  // 所有二元运算的类，例如1+1, 2+2, 3+a, a+b
 private:
  char operation;
  ASTExpression *lhs;
  ASTExpression *rhs;

 public:
  ASTBinaryExpression(char _operation, ASTExpression* _lhs,
                      ASTExpression* _rhs)
      : operation(_operation), lhs(_lhs), rhs(_rhs) {}
};

class ASTCallExpression : public ASTExpression {
  // 所有call其他函数的类
 private:
  typedef ASTExpression* CallArgument;
  std::string callee;
  std::vector<CallArgument> args;

 public:
  ASTCallExpression(const std::string& _callee, std::vector<CallArgument> _args)
      : callee(_callee), args(_args) {}
};

#endif
