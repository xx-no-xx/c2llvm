// 用于实际生成llvm ir的generator

#include "generator.h"

/* ------------------ ASTContext 方法的实现 ---------------- */

// TODO: 载入参数
// # void ASTContext::load_argument(ASTFunctionImp* function) {
// # //  for(auto &arg: function->prototype->args) {
// # //  }
// # }

llvm::Value* ASTContext::get_var(std::string var_name) {
  if (codestack.empty()) {
    std::cout << "panic: empty code stack when getting" << std::endl;
    return nullptr;
  }
  auto symbol = codestack.top()->get_symbol(var_name);
  return symbol;
}

llvm::Value* ASTContext::create_local_var(int type, std::string var_name) {
  if (codestack.empty()) {
    std::cout << "panic: empty code stack when creating" << std::endl;
    return nullptr;
  }
  auto var =
      builder->CreateAlloca(this->get_type(type), nullptr,
                            var_name);  // 创造单个变量，所以ArraySize = nullptr
  codestack.top()->add_symbol(var_name, var);
  return var;
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

// 返回一个变量的值。它只能被作为表达式的右值。
llvm::Value* ASTVariableExpression::generate(ASTContext* astcontext) {
  auto var =
      astcontext->codestack.top()->get_symbol(this->get_name());  // 获取符号表
  auto var_load_name = var->getName() + llvm::Twine("_load");  // 取名为xxx_load
  auto var_load = astcontext->builder->CreateLoad(
      var->getType()->getPointerElementType(), var,
      var_load_name);  // 将它load为右值 xxx_load = load xxx
  return var_load;
}

// A = B的赋值语句IR生成
llvm::Value* ASTVariableAssign::generate(ASTContext* astcontext) {
  auto var = astcontext->get_var(this->lhs->get_name());
  auto assign =
      astcontext->builder->CreateStore(this->rhs->generate(astcontext), var);
  return assign;
}

llvm::Value* ASTVariableDefine::generate(ASTContext* astcontext) {
  auto inst = astcontext->create_local_var(type, this->lhs->get_name());
  // TODO: 数组情况
  if (this->rhs != nullptr) {
    astcontext->builder->CreateStore(this->rhs->generate(astcontext), inst);
  }
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

  auto entryBB = llvm::BasicBlock::Create(
      *(astcontext->context), "entry__" + this->prototype->get_name(),
      func);  // 创建这个函数的entry基本块
  astcontext->builder->SetInsertPoint(entryBB);  // 设置IRbuilder到这个基本块上

  // TODO: 符号表
  // TODO: astcontext->load_argument(); // 载入函数的参数

  if (llvm::Value* retcode = this->function_entry->generate(astcontext)) {
    //    astcontext->builder->CreateRet(retcode);
    // TODO: retcode
    std::cout << "Function Generated!" << std::endl;
    //    return func;
  }
  astcontext->builder->CreateRet(0);  // return void
  return nullptr;
}

llvm::Value* ASTBinaryExpression::generate(ASTContext* astcontext) {
  llvm::Value* l_code = this->lhs->generate(astcontext);
  llvm::Value* r_code = this->rhs->generate(astcontext);

  if (!l_code | !r_code) return nullptr;

  if (this->operation == '+') {
    return astcontext->builder->CreateAdd(
        l_code, r_code, "foo_add");  // 创造l_code + r_code 的 add
  }
  // TODO 其他运算情况
  return nullptr;
}

llvm::Value* ASTCallExpression::generate(ASTContext* astcontext) {
  // * 目前不考虑多个函数的情况
  return nullptr;
}