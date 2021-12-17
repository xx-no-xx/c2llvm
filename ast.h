// 用于控制语法分析树的结点/BasicBlock
#ifndef C2LLVM_AST_HPP
#define C2LLVM_AST_HPP
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>
#include <map>
#include <utility>
#include <vector>

// 环境
class ASTContext;  // 用于存储当前语法分析树的上下文信息

// 基础的类
class ASTNode;        // 所有 AST 结点的类
class ASTExpression;  // 所有表达式的类，除原型外所有表达式的基类,
                      // 它们可以做赋值的右值。
class ASTPrototype;  // 所有原型式子的基类, 包含了许多重要的信息
class
    ASTCodeBlockExpression;  // 一段代码的类, 对应了一些代码.
                             // 生成从entryBB->exitBB的一大长段，如果check_return()==True,那么没有exitBB，在这里就全部返回了。

// 函数相关
class ASTFunctionProto;  // 函数原型
class ASTFunctionImp;    // 函数实现
class ASTCallExpresion;  // todo: 函数调用实现

// 非数组/字符串变量相关
class ASTVariableDefine;  // 变量定义的类，形如int a = b;
class ASTVariableAssign;  // 变量赋值的类，形如a = b;
class
    ASTVariableExpression;  // 变量的类，不包括数组变量，调用它的generate方法，会生成该变量的右值代码

// 表达式相关
class ASTBinaryExpression;  // 二元运算的类，形如 a + 1 [不包括赋值]

// 常量
class ASTInteger;  // int 常量，形如998244353

// 控制流
class ASTIfExpression;  // IF/ELSE分支, 支持(condition, if_code)以及(condition,
                        // if_code, else_code)两种形式。
class ASTWhileExpression;  // While循环，支持while(condition){code}

// 预留
class ASTGeneralExpression;  // not used: 预留
class ASTGeneralPrototype;   // not used: 预留

// Type Define
#define TYPE_VOID 0
#define TYPE_INT 1
#define TYPE_DOUBLE 2
#define TYPE_CHAR 3
#define TYPE_FLOAT 4

// [二元]运算符Define
#define OP_BI_ADD 0
#define OP_BI_SUB 1
#define OP_BI_MUL 2
#define OP_BI_DIV 3
#define OP_BI_LESS 4
#define OP_BI_MORE 5
#define OP_BI_LESSEQ 6
#define OP_BI_MOREEQ 7
#define OP_BI_AND 8  // 逻辑
#define OP_BI_OR 9   // 逻辑
#define OP_BI_MOD 10
#define OP_BI_EQ 11

extern ASTCodeBlockExpression* entryCodeBlock;

class ASTNode {
  // 所有AST结点的基类
  bool is_ret = false;  // 该ASTNode是否没有ret, 以至于没有后继
 public:
  ASTNode() {}
  void set_return() { is_ret = true; }
  bool check_return() { return is_ret; }
  virtual llvm::Value* generate(ASTContext* astcontext) {
    // 生成该AST结点对应的llvmIR代码
    return nullptr;
  }
  virtual std::string get_class_name() { return "ASTNode"; }
  virtual void debug() {
    std::cout << "this is " << get_class_name() << std::endl;
  }
  /*
  TODO:
    json化AST
    virtual getjson(void);
  */
};

class ASTPrototype : public ASTNode {
  // 所有AST原型或者声明的基类
  // 表达式不能作为左值
 public:
  ASTPrototype() {}
  virtual llvm::Value* generate(ASTContext* astcontext) override;
  virtual std::string get_class_name() override { return "ASTPrototype"; }
  virtual void debug() override {
    std::cout << "this is " << get_class_name() << " " << std::endl;
  }
};

class ASTExpression : public ASTNode {
  // 所有AST表达式的基类
 public:
  ASTExpression() {}

  virtual llvm::Value* generate(ASTContext* astcontext) override;
  virtual std::string get_class_name() override { return "ASTExpression"; }
  virtual void debug() override {
    std::cout << "this is " << get_class_name() << " " << std::endl;
  }
};

class ASTGeneralPrototype : public ASTPrototype {
  // TODO: 没有用，预留
 public:
  ASTGeneralPrototype() {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name() override { return "ASTGeneralPrototype"; }
  void debug() override {}
};

class ASTGeneralExpression : public ASTExpression {
  // TODO: 没有用，预留
 public:
  ASTGeneralExpression() {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name() override { return "ASTGeneralExpression"; }
  void debug() override {}
};

class ASTCodeBlockExpression : public ASTExpression {
  // 该类对应着一序列的代码块，及一些AST的序列

  std::vector<ASTNode*> codes;
  std::map<std::string, llvm::Value*> symboltable;  // 符号表

 public:
  ASTCodeBlockExpression() {
    codes.clear();
    symboltable.clear();
  }
  llvm::BasicBlock* entryBB;  // 入口BB。它可能后面有一堆BB。
  llvm::BasicBlock* exitBB;   // 出口基本块，至多只有一个。
  llvm::BasicBlock* get_exit(void) {
    // 如果为is_ret为true, exitBB不成立。
    if (this->check_return()) {
      std::cout << "panic: get exit for a ret block" << std::endl;
    }
    return exitBB;
  }
  void copy_symbol_from(std::map<std::string, llvm::Value*> table);
  std::map<std::string, llvm::Value*> get_symboltable(void) {
    return this->symboltable;
  }  // 获取符号表
  void set_exit(llvm::BasicBlock* _exit) { exitBB = _exit; }
  llvm::Value* generate(ASTContext* astcontext)
      override;  // 此处会生成一个以entryBB开头的控制流图
  std::string get_class_name() override { return "ASTCodeBlockExpression"; }
  void set_function(ASTFunctionImp*);  // 设置代码块对应着哪一个函数的开头
  void append_code(ASTNode*);  // 将新的AST结点接在已有代码的末尾
  void clear_symbol(void);     // 清空符号表
  bool add_symbol(std::string,
                  llvm::Value*);  // 将一个llvm中的symbol加入代码块的符号表。
  //代码块对应的基本块集合共享这些变量。因此，这些变量在实际IR实现中要被放在最前面。
  llvm::Value* get_symbol(std::string);  // 获取该名称的符号表
  void debug(void) override {
    std::cout << "this is " << get_class_name() << " with " << codes.size()
              << " codes in here" << std::endl;
    for (auto& code : codes) {
      code->debug();
      puts("");
    }
  }
};

class ASTVariableExpression : public ASTExpression {
  // 对应了一个表达式的左值, 一个变量，例如a
 private:
  std::string name;
  bool is_array;     // 它是否是数组
  int array_length;  // 数组的长度

 public:
  ASTVariableExpression(std::string _name, bool _is_array = false,
                        int _array_length = -1)
      : name(_name), is_array(_is_array), array_length(_array_length) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  void set_array(int len) {
    array_length = len;
    is_array = true;
  }
  std::string get_name(void) { return name; }
  int get_array_length(void) {
    if (is_array == false) return 0;
    return array_length;
  }
  std::string get_class_name(void) override { return "ASTVariableExpression"; }
  void debug(void) override { std::cout << "var:" << name << " "; }
};

class ASTVariableAssign : public ASTExpression {
 private:
  ASTVariableExpression* lhs;
  ASTExpression* rhs;

 public:
  ASTVariableAssign(ASTVariableExpression* _lhs, ASTExpression* _rhs)
      : lhs(_lhs), rhs(_rhs) {}
  llvm::Value* generate(ASTContext* astcontext) override;

  std::string get_class_name(void) override { return "ASTVariableAssign"; }
  void debug(void) override {
    lhs->debug();
    std::cout << " [ASSIGN]="
              << " ";
    rhs->debug();
  }
};

class ASTVariableDefine : public ASTPrototype {
  // 给定了一个形如type lhs = rhs的变量声明
 private:
  int type;  // 参数类型
  ASTVariableExpression* lhs;
  ASTExpression* rhs;  // rhs不存在的情况，令rus=nullptr

 public:
  ASTVariableDefine(int _type, ASTVariableExpression* _lhs, ASTExpression* _rhs)
      : type(_type), lhs(_lhs), rhs(_rhs) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return "ASTVariableDefine"; }
  void debug(void) override {
    std::cout << "type:" << type << " ";
    lhs->debug();
    std::cout << " [ASSIGN FROM DEFINE]= ";
    if (rhs)
      rhs->debug();
    else
      std::cout << "none";
  }
};

typedef ASTVariableDefine ARGdefine;  // 参数声明
typedef std::string ARGname;          // 参数名称
class ASTFunctionProto : public ASTPrototype {
  // 声明了一个形如ret_type name(args)的函数
 private:
  int ret_type;  // TODO: 现在默认不返回东西
  std::string name;
  std::vector<std::pair<ARGdefine, ARGname>> args;  // ! ARGname 必须不一样
  // TODO: 参数符号表，在push_codeblock中合

 public:
  ASTFunctionProto(int _ret_type, std::string _name,
                   std::vector<std::pair<ARGdefine, ARGname>> _args)
      : ret_type(_ret_type), name(_name), args(_args) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return "ASTFunctionProto"; }
  std::string get_name(void) { return name; }
  void debug(void) override {
    // TODO: 展示参数
    std::cout << "Function Prototype Name: " << name << std::endl;
  }
};

class ASTFunctionImp : public ASTExpression {
  // 实现了一个函数，它的声明是prototype, 函数的入口是function_entry;
 private:
  ASTFunctionProto* prototype;
  ASTCodeBlockExpression* function_entry;

 public:
  ASTFunctionImp(ASTFunctionProto* _pro, ASTCodeBlockExpression* _entry)
      : prototype(_pro), function_entry(_entry) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  void set_entry(ASTCodeBlockExpression*);
  void debug(void) override {
    prototype->debug();
    function_entry->debug();
  }
  std::string get_class_name(void) override { return "ASTFunctionImp"; }
};

class ASTInteger : public ASTExpression {
  // 所有整数的类，例如1, 2, 33000
 private:
  int value;

 public:
  ASTInteger(int _value) : value(_value) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  int get_value(void) { return value; }
  std::string get_class_name(void) override { return "ASTInteger"; }
  void debug(void) override { std::cout << "[INTEGER]" << value << " "; }
};

class ASTBinaryExpression : public ASTExpression {
  // 所有二元运算的类，例如1+1, 2+2, 3+a, a+b
 private:
  int operation;  // +, -, *, /
  ASTExpression* lhs;
  ASTExpression* rhs;

 public:
  ASTBinaryExpression(int _operation, ASTExpression* _lhs, ASTExpression* _rhs)
      : operation(_operation), lhs(_lhs), rhs(_rhs) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return "ASTBinaryExpression"; }
  void debug(void) override {
    lhs->debug();
    std::cout << " [BinaryOperation]" << operation << " ";
    rhs->debug();
  }
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
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return " ASTCallExpression"; }
  void debug(void) override {
    // TODO
    return;
  }
};

class ASTIfExpression : public ASTExpression {
  /* 对应if-else分支： if(condition) {ifcode} else {elsecode} */
 private:
  ASTExpression* condition;
  ASTCodeBlockExpression* ifcode;
  ASTCodeBlockExpression* elsecode;

 public:
  ASTIfExpression(
      ASTExpression* _condition, ASTCodeBlockExpression* _ifcode,
      ASTCodeBlockExpression* _elsecode = new ASTCodeBlockExpression())
      : condition(_condition), ifcode(_ifcode), elsecode(_elsecode) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return "ASTIfExpression"; }
  void debug(void) override {
    std::cout << "if( ";
    condition->debug();
    std::cout << " ) [then]{ ";
    ifcode->debug();
    std::cout << " }";
    if (elsecode) {
      std::cout << "else { ";
      elsecode->debug();
      std::cout << " }";
    }
    std::cout << "[END IF/ELSE]";
  }
};

class ASTWhileExpression : public ASTExpression {
  /* 对应while： while(condition(expression)) {code(code block)} */
 private:
  ASTExpression* condition;
  ASTCodeBlockExpression* code;

 public:
  ASTWhileExpression(ASTExpression* _condition, ASTCodeBlockExpression* _code)
      : condition(_condition), code(_code) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return "ASTWhileExpression"; }
  void debug(void) override {
    std::cout << "while( ";
    condition->debug();
    std::cout << " ) [then]{ ";
    code->debug();
    std::cout << " }";
    std::cout << "[END WHILE]";
  }
};

class ASTForExpression : public ASTExpression {
  /* 对应for： for(start(codeblock); condition(expression); action(codeblock))
   * {code(code block)} */
 private:
  ASTExpression* condition;
  ASTCodeBlockExpression* prepare;
  ASTCodeBlockExpression* code;
  ASTCodeBlockExpression* action;

 public:
  ASTForExpression(
      ASTExpression* _condition, ASTCodeBlockExpression* _code,
      ASTCodeBlockExpression* _prepare = new ASTCodeBlockExpression(),
      ASTCodeBlockExpression* _action = new ASTCodeBlockExpression())
      : condition(_condition),
        code(_code),
        prepare(_prepare),
        action(_action) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return "ASTForExpression"; }
  void debug(void) override {
    std::cout << "for( ";
    prepare->debug();
    std::cout << ";";
    condition->debug();
    std::cout << ";";
    action->debug();
    std::cout << " ) [then]{ ";
    code->debug();
    std::cout << " }";
    std::cout << "[END FOR]";
  }
};

class ASTArrayExpression : public ASTExpression {
  // 对应name[index]的数组元素
 private:
  std::string name;
  ASTExpression* index;

 public:
  ASTArrayExpression(std::string _name, ASTExpression* _index)
      : name(_name), index(_index) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  llvm::Value* generate_ptr(ASTContext* astcontext);
  std::string get_name(void) { return name; }
  std::string get_class_name(void) override { return "ASTArrayExpression"; }
  void debug(void) override {
    std::cout << "array_var:" << name << "[ ";
    index->debug();
    std::cout << " ]";
  }
};

class ASTArrayAssign : public ASTExpression {
  // 对应name[index]的数组元素
 private:
  ASTArrayExpression* lhs;
  ASTExpression* rhs;

 public:
  ASTArrayAssign(ASTArrayExpression* _lhs, ASTExpression* _rhs)
      : lhs(_lhs), rhs(_rhs) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return "ASTArrayAssign"; }
  void debug(void) override {
    lhs->debug();
    std::cout << "[assign] = ";
    rhs->debug();
  }
};

#endif
