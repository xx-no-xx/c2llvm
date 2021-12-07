// 用于实际生成llvm ir的generator

#include "generator.h"

/* ------------------ ASTContext 方法的实现 ---------------- */

void ASTContext::clear_symboltable(void) { this->symboltable.clear(); }

// TODO: 载入参数
// # void ASTContext::load_argument(ASTFunctionImp* function) {
// # //  for(auto &arg: function->prototype->args) {
// # //  }
// # }

llvm::Value* ASTContext::get_var(std::string var_name) {
  if (this->symboltable.count(var_name) == 1) {
    return symboltable[var_name];
  }
  return nullptr;
}

llvm::Value* ASTContext::create_local_var(int type, std::string var_name) {
  if (this->symboltable.count(var_name) == 0) {
    symboltable[var_name] = this->builder->CreateAlloca(
        this->get_type(type), nullptr,
        var_name);  // 创造单个变量，所以ArraySize = nullptr
    return symboltable[var_name];
  }
  return nullptr;
}

llvm::Type* ASTContext::get_type(int type) {
  if (type == TYPE_VOID) {
    return llvm::Type::getVoidTy(*(this->context));
  } else if (type == TYPE_INT) {
    return llvm::Type::getInt32Ty(*(this->context));
  }
  // TODO
  return nullptr;
}

/* ----------------- 生成代码 ----------------------- */

llvm::Value* ASTPrototype::generate(ASTContext* astcontext) {
  // ? 我怀疑这个是否会被调用
  return nullptr;
}

llvm::Value* ASTExpression::generate(ASTContext* astcontext) {
  // ? 我怀疑这个是否会被调用
  return nullptr;
}

llvm::Value* ASTGeneralExpression::generate(ASTContext* astcontext) {
  // * 无用，预留
  return nullptr;
}

llvm::Value* ASTGeneralPrototype::generate(ASTContext* astcontext) {
  // * 无用，预留
  return nullptr;
}

llvm::Value* ASTCodeBlockExpression::generate(ASTContext* astcontext) {
  // TODO
  llvm::Value* retcode = nullptr;  // 认为它什么也不返回
  astcontext->codestack.push(this);
  // TODO: 设置名字
  //  if(!this->bbcreated) {
  // TODO: 循环生成CFG

  // TODO: 如果这个codeblock不属于任何函数，即全局变量的情况

  // }
  // * 假设它在函数内部，没有多余的基本块
  this->BB = llvm::BasicBlock::Create(
      *(astcontext->context), "foo_basicblock",
      astcontext->current_f);  // 在这个函数的最后加入一个基本块

  // ! 理论上需要有这句话
  astcontext->builder->CreateBr(this->BB);
  // 但前辈们没有...

  astcontext->builder->SetInsertPoint(
      this->BB);  // 设置IRBuilder在这个基本块上运行

  // TODO: 处理参数

  for (auto& code : this->codes) {
    retcode =
        code->generate(astcontext);  // retcode 是这个代码块最后的那一条命令
  }
  astcontext->codestack.pop();
  return retcode;
}

// 生成某个常数
llvm::Value* ASTInteger::generate(ASTContext* astcontext) {
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*(astcontext->context)),
                                this->value, true);  // true代表有符合整数
}

llvm::Value* ASTVariableExpression::generate(ASTContext* astcontext) {
  // TODO
  return nullptr;
}

llvm::Value* ASTVariableDefine::generate(ASTContext* astcontext) {
  auto inst = astcontext->builder->CreateAlloca(
      astcontext->get_type(this->type), nullptr, "foo_var");
  // TODO: change name, virtual function

  // TODO:操作符号表

  // TODO: 数组情况

  if (this->rhs != nullptr) {
    astcontext->builder->CreateStore(this->rhs->generate(astcontext), inst);
  }
  // TODO: 这个地方应该拆掉变量赋值去
  return inst;
}

llvm::Value* ASTFunctionProto::generate(ASTContext* astcontext) {
  // 在函数原型部分，生成这个函数本身
  llvm::FunctionType* functype =
      llvm::FunctionType::get(astcontext->get_type(TYPE_VOID), false);
  // TODO: 当前创造了一个void() {}的函数

  llvm::Function* func =
      llvm::Function::Create(functype, llvm::Function::ExternalLinkage,
                             this->get_name(), astcontext->current_m);
  // TODO: 函数参数

  astcontext->current_f = func;  // 把当前函数更新到上下文

  return func;
}

llvm::Value* ASTFunctionImp::generate(ASTContext* astcontext) {
  auto func = astcontext->current_m->getFunction(
      this->prototype->get_name());  // 获取函数的名称, 尝试获取函数

  if (!func) {
    this->prototype->generate(astcontext);  // 首先生成函数
  }

  func = astcontext->current_f;  // 函数已经生成好了
  if (!func) {
    return nullptr;
  }

  auto BB = llvm::BasicBlock::Create(*(astcontext->context), "entry",
                                     func);  // 创建这个函数的entry基本块
  astcontext->builder->SetInsertPoint(BB);  // 设置IRbuilder到这个基本块上

  astcontext->clear_symboltable();  // 清楚上一个函数的符号表
  // TODO: astcontext->load_argument(); // 载入函数的参数

  if (llvm::Value* retcode = function_entry->generate(astcontext)) {
    astcontext->builder->CreateRet(retcode);
    // TODO: 我不知道这个retval在做什么

    // TODO: 验证我们的函数
    return func;
  }
  return nullptr;
}

llvm::Value* ASTBinaryExpression::generate(ASTContext* astcontext) {
  llvm::Value* l_code = this->lhs->generate(astcontext);
  llvm::Value* r_code = this->rhs->generate(astcontext);

  if (!l_code | !r_code) return nullptr;

  if (this->operation == '+') {
    return astcontext->builder->CreateAdd(l_code, r_code, "foo_add");
  }
  // TODO 其他运算情况
  return nullptr;
}

llvm::Value* ASTCallExpression::generate(ASTContext* astcontext) {
  // * 目前不考虑多个函数的情况
  return nullptr;
}