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

#include "json.hpp"
using njson = nlohmann::json;

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
class ASTFunctionProto;   // 函数原型
class ASTFunctionImp;     // 函数实现
class ASTCallExpression;  // 函数调用实现

// 非数组/字符串变量相关
class ASTVariableDefine;  // 变量定义的类，形如int a = b;
class ASTVariableAssign;  // 变量赋值的类，形如a = b;
class
    ASTVariableExpression;  // 变量的类，不包括数组变量，调用它的generate方法，会生成该变量的右值代码

// 表达式相关
class ASTBinaryExpression;  // 二元运算的类，形如 a + 1 [不包括赋值]
class ASTSingleExpression;  // 一元运算的类，取地址，取反

// 常量
class ASTInteger;                 // int 常量，形如998244353
class ASTGlobalStringExpression;  // 字符串字面量
class ASTChar;                    // char
class ASTFloat;                    // float

// 控制流
class ASTIfExpression;  // IF/ELSE分支, 支持(condition, if_code)以及(condition,
                        // if_code, else_code)两种形式。
class ASTWhileExpression;  // While循环，支持while(condition){code}

// 预留

// Type Define
#define TYPE_VOID 0
#define TYPE_INT 1
#define TYPE_DOUBLE 2
#define TYPE_CHAR 3
#define TYPE_FLOAT 4
#define TYPE_BOOL 5
#define TYPE_INT_PTR 6
#define TYPE_CHAR_PTR 7

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
#define OP_BI_NEQ 12
#define OP_SI_ADDRESS 13  // 取地址
#define OP_SI_NOT 14

extern ASTCodeBlockExpression* entryCodeBlock;

std::string getDefineStr(int type, int value);

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
  virtual njson generate_json() {
    njson v;
    v["attr"] = njson();
    v["attr"]["class"] = get_class_name();
    v["child"] = njson();
    return v;
  }
  virtual llvm::Value* generate_ptr(ASTContext* astcontext) { return nullptr; }
  virtual std::string get_class_name() { return "ASTNode"; }
  virtual void debug() {
    std::cout << "this is " << get_class_name() << std::endl;
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
  void generate_from_root(ASTContext* astcontext) {
    for (auto& code : codes) {
      code->generate(astcontext);
    }
  }
  void debug(void) override {
    std::cout << "this is " << get_class_name() << " with " << codes.size()
              << " codes in here" << std::endl;
    for (auto& code : codes) {
      code->debug();
      puts("");
    }
  }
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["attr"]["class"] = get_class_name();

    v["child"] = njson();
    int count = 0;
    for (auto& code : codes) {
      v["child"][count] = code->generate_json();
      count++;
    }
    return v;
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
  llvm::Value* generate_ptr(ASTContext* astcontext) override;
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
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();

    v["attr"]["class"] = get_class_name();
    v["attr"]["name"] = get_name();
    v["attr"]["is_array"] = is_array;
    v["attr"]["array_length"] = array_length;
    v["child"] = njson();
    return v;
  }
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

  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"] = njson();
    v["attr"]["class"] = get_class_name();
    v["child"][0] = lhs->generate_json();
    v["child"][1] = rhs->generate_json();
    return v;
  }
};

class ASTVariableDefine : public ASTExpression {
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
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"] = njson();
    v["attr"]["type"] = getDefineStr(1, type);
    v["attr"]["class"] = get_class_name();
    v["child"][0] = lhs->generate_json();
    if (rhs) v["child"][1] = rhs->generate_json();
    return v;
  }
};

typedef ASTVariableDefine ARGdefine;  // 参数声明
typedef std::string ARGname;          // 参数名称
class ASTFunctionProto : public ASTExpression {
  // 声明了一个形如ret_type name(args)的函数
 private:
  int ret_type;      // 返回类型
  std::string name;  // 函数名
  std::vector<std::pair<int, ARGname> > args;
  bool isVarArg;  //是否为可变长参数
 public:
  ASTFunctionProto(int _ret_type, std::string _name,
                   std::vector<std::pair<int, ARGname> > _args,
                   bool _isVarArg = 0)
      : ret_type(_ret_type), name(_name), args(_args), isVarArg(_isVarArg) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return "ASTFunctionProto"; }
  std::string get_name(void) { return name; }
  int get_type_argi(int i) { return args[i].first; }
  void debug(void) override {
    for (auto arg : args)
      std::cout << arg.first << " " << arg.second << std::endl;
    std::cout << "Function Prototype Name: " << name << std::endl;
  }
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["attr"]["ret_type"] = getDefineStr(1, ret_type);
    v["attr"]["class"] = get_class_name();
    v["attr"]["name"] = get_name();
    v["attr"]["is_var_arg"] = isVarArg;
    v["child"] = njson();
    return v;
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
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"] = njson();
    v["attr"]["class"] = get_class_name();
    v["child"][0] = prototype->generate_json();
    if (function_entry) v["child"][1] = function_entry->generate_json();
    return v;
  }
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
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"] = njson();
    v["attr"]["value"] = value;
    v["attr"]["class"] = get_class_name();
    return v;
  }
};

class ASTChar : public ASTExpression {
 private:
  int value;

 public:
  ASTChar(int _value) : value(_value) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  int get_value(void) { return value; }
  std::string get_class_name(void) override { return "ASTChar"; }
  void debug(void) override { std::cout << "[CHAR]" << value << " "; }
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"] = njson();
    v["attr"]["value"] = char(value);
    v["attr"]["class"] = get_class_name();
    return v;
  }
};

class ASTFloat : public ASTExpression {
 private:
  float value;

 public:
  ASTFloat(float _value) : value(_value) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  int get_value(void) { return value; }
  std::string get_class_name(void) override { return "ASTFloat"; }
  void debug(void) override { std::cout << "[FLOAT]" << value << " "; }
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"] = njson();
    v["attr"]["value"] = value;
    v["attr"]["class"] = get_class_name();
    return v;
  }
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
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"] = njson();
    v["attr"]["operation"] = getDefineStr(0, operation);
    v["attr"]["class"] = get_class_name();
    v["child"][0] = lhs->generate_json();
    v["child"][1] = rhs->generate_json();
    return v;
  }
};

class ASTSingleExpression : public ASTExpression {
  // 所有一元运算的类
 private:
  int operation;  // !, &
  ASTExpression* exp;

 public:
  ASTSingleExpression(int _operation, ASTExpression* _exp)
      : operation(_operation), exp(_exp) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return "ASTSingleExpression"; }
  void debug(void) override {
    std::cout << " [SingleOperation]" << operation << " ";
    exp->debug();
  }
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"] = njson();
    v["attr"]["operation"] = getDefineStr(0, operation);
    v["attr"]["class"] = get_class_name();
    v["child"][0] = exp->generate_json();
    return v;
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
  std::string get_class_name(void) override { return "ASTCallExpression"; }
  void debug(void) override { return; }
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["attr"]["callee"] = callee;
    v["attr"]["class"] = get_class_name();
    v["child"] = njson();
    int count = 0;
    for (auto& e : args) {
      v["child"][count] = e->generate_json();
      count++;
    }
    return v;
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
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"][0] = condition->generate_json();
    v["attr"]["class"] = get_class_name();
    v["child"] = njson();
    v["child"][1] = ifcode->generate_json();
    v["child"][2] = elsecode->generate_json();
    return v;
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
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"][0] = condition->generate_json();
    v["attr"]["class"] = get_class_name();
    v["child"] = njson();
    v["child"][1] = code->generate_json();
    return v;
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
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["child"] = njson();
    v["child"][0] = prepare->generate_json();
    v["child"][1] = condition->generate_json();
    v["child"][2] = action->generate_json();
    v["attr"]["class"] = get_class_name();
    v["child"][3] = code->generate_json();
    return v;
  }
};

class ASTGlobalStringExpression : public ASTExpression {
  // 字符串字面量
 private:
  std::string Str;

 public:
  ASTGlobalStringExpression(std::string _Str) : Str(_Str) {}
  llvm::Value* generate(ASTContext* astcontext) override;
  std::string get_class_name(void) override { return "ASTStringExpression"; }
  void debug(void) override { std::cout << Str << std::endl; }
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["attr"]["string"] = Str;
    v["attr"]["class"] = get_class_name();
    v["child"] = njson();
    return v;
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
  llvm::Value* generate_ptr(ASTContext* astcontext) override;
  std::string get_name(void) { return name; }
  std::string get_class_name(void) override { return "ASTArrayExpression"; }
  void debug(void) override {
    std::cout << "array_var:" << name << "[ ";
    index->debug();
    std::cout << " ]";
  }
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["attr"]["name"] = get_name();
    v["child"][0] = index->generate_json();
    v["attr"]["class"] = get_class_name();
    v["child"] = njson();
    return v;
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
  njson generate_json(void) override {
    njson v;
    v["attr"] = njson();
    v["attr"]["class"] = get_class_name();
    v["child"] = njson();
    v["child"][0] = lhs->generate_json();
    v["child"][1] = rhs->generate_json();
    return v;
  }
};

#endif
