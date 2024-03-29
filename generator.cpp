// 用于实际生成llvm ir的generator
#include "generator.h"

/* ------------------ ASTContext 方法的实现 ---------------- */

llvm::Value* ASTContext::generate_condition(llvm::Value* condori) {
  llvm::Value* condinst;
  if (condori->getType() == get_type(TYPE_INT)) {
    auto zero =
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(*(context)), int(0));
    condinst = builder->CreateICmpNE(condori, zero);
  } else if (condori->getType() == get_type(TYPE_CHAR)) {
    auto zero =
        llvm::ConstantInt::get(llvm::Type::getInt8Ty(*(context)), int(0));
    condinst = builder->CreateICmpNE(condori, zero);
  } else if (condori->getType() == get_type(TYPE_FLOAT)) {
    auto zero =
        llvm::ConstantFP::get(llvm::Type::getDoubleTy(*(context)), double(0.0));
    condinst = builder->CreateFCmpONE(condori, zero);
  } else if (condori->getType() == llvm::Type::getInt1Ty(*context)) {
    condinst = condori;
  } else if (condori->getType() == get_type(TYPE_INT_PTR)) {
    condinst = builder->CreateIsNull(condori);
  } else {
    std::cout << "panic: wrong condition type" << std::endl;
    exit(0);
  }
  return condinst;
}

llvm::Value* ASTContext::get_var(std::string var_name) {
  if (codestack.empty()) {
    std::cout << "panic: empty code stack when getting" << std::endl;
    return nullptr;
  }
  auto symbol = codestack.top()->get_symbol(var_name);
  return symbol;
}

llvm::Value* ASTContext::create_local_var(int type, std::string var_name,
                                          int array_length) {
  if (codestack.empty()) {
    std::cout << "panic: empty code stack when creating" << std::endl;
    return nullptr;
  }
  if (array_length > 0) {
    auto alloctype = llvm::ArrayType::get(get_type(type), array_length);
    auto var = builder->CreateAlloca(alloctype, nullptr, var_name);
    codestack.top()->add_symbol(var_name, var);
    return var;
  } else {
    // 创造单个变量，所以ArraySize = nullptr
    auto var = builder->CreateAlloca(this->get_type(type), nullptr, var_name);
    codestack.top()->add_symbol(var_name, var);
    return var;
  }
}

llvm::Type* ASTContext::get_type(int type) {
  if (type == TYPE_VOID) {
    return llvm::Type::getVoidTy(*(this->context));
  } else if (type == TYPE_INT) {
    return llvm::Type::getInt32Ty(*(this->context));
  } else if (type == TYPE_CHAR) {
    return llvm::Type::getInt8Ty(*(this->context));
  } else if (type == TYPE_CHAR_PTR) {
    return llvm::Type::getInt8PtrTy(*(this->context));
  } else if (type == TYPE_DOUBLE) {
    return llvm::Type::getDoubleTy(*(this->context));
  } else if (type == TYPE_FLOAT) {
    return llvm::Type::getDoubleTy(*(this->context));
  } else if (type == TYPE_BOOL) {
    return llvm::Type::getInt1Ty(*(this->context));
  } else if (type == TYPE_INT_PTR) {
    return llvm::Type::getInt32PtrTy(*(this->context));
  }
  return nullptr;
}
/*
int ASTContext::get_type(llvm::Type* type) {
  if (type == llvm::Type::getVoidTy(*(this->context))) {
    return TYPE_VOID;
  } else if (type == llvm::Type::getInt32Ty(*(this->context))) {
    return TYPE_INT;
  } else if (type == llvm::Type::getInt8PtrTy(*(this->context))){
    return TYPE_CHAR_PTR;
  } else if (type == llvm::Type::getFloatTy(*(this->context))) {
    return TYPE_FLOAT;
  } else if (type == llvm::Type::getInt1Ty(*(this->context))) {
    return TYPE_BOOL;
  } else if (type == llvm::Type::getInt32PtrTy(*(this->context))) {
    return TYPE_INT_PTR;
  }
  return TYPE_INT;
}
*/
/* ----------------- 生成代码 ----------------------- */

llvm::Value* ASTExpression::generate(ASTContext* astcontext) {
  // ? 我怀疑这个是否会被调用
  return nullptr;
}

llvm::Value* ASTCodeBlockExpression::generate(ASTContext* astcontext) {
  // 更新codeblock，每次只选择当前路径语句上可能的symboltable
  this->entryBB = llvm::BasicBlock::Create(
      *(astcontext->context), "", astcontext->current_f,
      nullptr);  // 在这个函数的最后加入一个基本块

  // 在制造entryBB后，通过该BB的函数信息，获取可行的符号表
  astcontext->push_codeblock(this);

  astcontext->builder->SetInsertPoint(this->entryBB);
  // 设置IRBuilder在这个基本块上运行

  llvm::BasicBlock* now = this->entryBB;
  llvm::BasicBlock* nxt = nullptr;
  for (auto& code : this->codes) {
    code->generate(astcontext);  // retcode 是代码块最后的那一条命令
    nxt = astcontext->builder->GetInsertBlock();  // 为codeblock添加br跳转
    if (code->get_class_name() == "ASTCodeBlockExpression") {
      ASTCodeBlockExpression* block = (ASTCodeBlockExpression*)code;
      astcontext->builder->SetInsertPoint(now);
      astcontext->builder->CreateBr(block->entryBB);
      astcontext->builder->SetInsertPoint(block->exitBB);
    }
    if (code->check_return()) {
      this->set_return();
      break;
    }
    now = nxt;
  }

  if (!this->check_return()) {  // 如果当前ASTCodeBlock可以有后继，设置exitBB
    this->set_exit(astcontext->builder->GetInsertBlock());
  }
  astcontext->pop_codeblock();

  return nullptr;
}

// 生成某个常数
llvm::Value* ASTInteger::generate(ASTContext* astcontext) {
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*(astcontext->context)),
                                this->value, true);  // true代表有符合整数
}

// generate char
llvm::Value* ASTChar::generate(ASTContext* astcontext) {
  return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*(astcontext->context)),
                                this->value, true);  // true代表有符合整数
}

llvm::Value* ASTFloat::generate(ASTContext* astcontext) {
  return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*(astcontext->context)),
                               this->value);
}

// 返回一个变量的值。它只能被作为表达式的右值。
llvm::Value* ASTVariableExpression::generate(ASTContext* astcontext) {
  auto var = astcontext->get_codestack_top()->get_symbol(
      this->get_name());  // 获取符号表

  if (var != nullptr) {
    if (var->getType()->getPointerElementType()->isArrayTy()) {
      auto v = new ASTArrayExpression(this->name, new ASTInteger(0));
      return v->generate_ptr(astcontext);
    }
    auto var_load_name = var->getName() + llvm::Twine("_load");
    auto var_load = astcontext->builder->CreateLoad(
        var->getType()->getPointerElementType(), var,
        var_load_name);  // 将它load为右值 xxx_load = load xxx
    return var_load;
  } else {  // 参数
    return astcontext->local_symboltable[this->get_name()];
  }
}

llvm::Value* ASTGlobalStringExpression::generate(ASTContext* astcontext) {
  return astcontext->builder->CreateGlobalStringPtr(this->Str);
}

// A = B的赋值语句IR生成
llvm::Value* ASTVariableAssign::generate(ASTContext* astcontext) {
  auto var = astcontext->get_var(this->lhs->get_name());
  auto assign =
      astcontext->builder->CreateStore(this->rhs->generate(astcontext), var);
  return assign;
}

// int A = B;的赋值语句
llvm::Value* ASTVariableDefine::generate(ASTContext* astcontext) {
  auto inst = astcontext->create_local_var(type, this->lhs->get_name(),
                                           this->lhs->get_array_length());
  if (this->rhs != nullptr) {
    astcontext->builder->CreateStore(this->rhs->generate(astcontext), inst);
  }
  return inst;
}

// 在函数原型部分，生成这个函数本身
llvm::Value* ASTFunctionProto::generate(ASTContext* astcontext) {
  //函数返回类型
  llvm::Type* returnType = astcontext->get_type(this->ret_type);
  //函数参数类型
  std::vector<llvm::Type*> funcArgs;
  for (auto item : this->args)
    funcArgs.push_back(astcontext->get_type(item.first));
  //根据前两者构成函数类型
  llvm::FunctionType* FT =
      llvm::FunctionType::get(returnType, funcArgs, this->isVarArg);
  //构造函数
  astcontext->current_m->getOrInsertFunction(this->name, FT);
  //设置参数名称
  auto F = astcontext->current_m->getFunction(this->name);
  unsigned idx = 0;
  for (auto& Arg : F->args()) Arg.setName(this->args[idx++].second);
  // 把当前函数更新到上下文
  return astcontext->current_f = F;
}

llvm::Value* ASTFunctionImp::generate(ASTContext* astcontext) {
  llvm::Function* TheFunc = astcontext->current_m->getFunction(
      this->prototype->get_name());  // 获取函数的名称, 尝试获取函数

  if (!TheFunc) this->prototype->generate(astcontext);  // 首先生成函数

  TheFunc = astcontext->current_m->getFunction(this->prototype->get_name());

  if (!TheFunc) return nullptr;

  //将参数名称加入符号表
  unsigned int idx = 0;
  astcontext->local_symboltable.clear();
  for (auto& Arg : TheFunc->args()) {
    astcontext->local_symboltable[std::string(Arg.getName())] = &Arg;
  }

  // 如果codeblock顺利gen,
  // 它创造的bb会在函数的入口上。因为函数内部没有任何多余的bb了。
  this->function_entry->generate(astcontext);
  auto zero = new ASTInteger(0);
  astcontext->builder->CreateRet(zero->generate(astcontext));  // return
  return nullptr;
}

llvm::Value* ASTBinaryExpression::generate(ASTContext* astcontext) {
  llvm::Value* l_code = this->lhs->generate(astcontext);
  llvm::Value* r_code = this->rhs->generate(astcontext);

  if (!l_code | !r_code) return nullptr;

  if (this->operation == OP_BI_AND) {
    // * 逻辑与
    auto lef = astcontext->generate_condition(l_code);
    auto rig = astcontext->generate_condition(r_code);
    return astcontext->builder->CreateAnd(lef, rig);
  } else if (this->operation == OP_BI_OR) {
    // * 逻辑或
    auto lef = astcontext->generate_condition(l_code);
    auto rig = astcontext->generate_condition(r_code);
    return astcontext->builder->CreateOr(lef, rig);
  } else {
    if (r_code->getType() == l_code->getType()) {
      // * 非根据类型不同
      if (this->operation == OP_BI_ADD) {
        return astcontext->builder->CreateAdd(l_code, r_code);  // 加法
      } else if (this->operation == OP_BI_SUB) {
        return astcontext->builder->CreateSub(l_code, r_code);  // 减法
      } else if (this->operation == OP_BI_MUL) {
        return astcontext->builder->CreateMul(l_code, r_code);  // 乘法
      } else if (this->operation == OP_BI_DIV) {
        return astcontext->builder->CreateSDiv(l_code, r_code);  // 有符号除法
      } else {
        // * 根据类型不同
        if (r_code->getType() == astcontext->get_type(TYPE_INT) ||
            r_code->getType() == astcontext->get_type(TYPE_CHAR)) {
          if (this->operation == OP_BI_LESS)
            return astcontext->builder->CreateICmpSLT(l_code, r_code);
          else if (this->operation == OP_BI_MORE)
            return astcontext->builder->CreateICmpSGT(l_code, r_code);
          else if (this->operation == OP_BI_LESSEQ)
            return astcontext->builder->CreateICmpSLE(l_code, r_code);
          else if (this->operation == OP_BI_MOREEQ)
            return astcontext->builder->CreateICmpSGE(l_code, r_code);
          else if (this->operation == OP_BI_EQ)
            return astcontext->builder->CreateICmpEQ(l_code, r_code);
          else if (this->operation == OP_BI_NEQ)
            return astcontext->builder->CreateICmpNE(l_code, r_code);
          else if (this->operation == OP_BI_MOD) {
            auto res = astcontext->builder->CreateSDiv(l_code, r_code);
            // res = l_code / r_code
            return astcontext->builder->CreateSub(l_code, res);
            // inst = l_code - l_code / r_code
          }
        } else if (r_code->getType() == astcontext->get_type(TYPE_FLOAT)) {
          // * also for TYPE_DOUBLE
          if (this->operation == OP_BI_LESS)
            return astcontext->builder->CreateFCmpOLT(l_code, r_code);
          else if (this->operation == OP_BI_MORE)
            return astcontext->builder->CreateFCmpOGT(l_code, r_code);
          else if (this->operation == OP_BI_LESSEQ)
            return astcontext->builder->CreateFCmpOLE(l_code, r_code);
          else if (this->operation == OP_BI_MOREEQ)
            return astcontext->builder->CreateFCmpOGE(l_code, r_code);
          else if (this->operation == OP_BI_EQ)
            return astcontext->builder->CreateFCmpOEQ(l_code, r_code);
          else if (this->operation == OP_BI_NEQ)
            return astcontext->builder->CreateFCmpONE(l_code, r_code);
        } else if (r_code->getType() == astcontext->get_type(TYPE_BOOL)) {
          if (this->operation == OP_BI_EQ)
            return astcontext->builder->CreateICmpEQ(l_code, r_code);
          else if (this->operation == OP_BI_NEQ)
            return astcontext->builder->CreateICmpNE(l_code, r_code);
        }
      }
      std::cout << "panic: not allowed type" << std::endl;
      exit(0);
    } else {
      std::cout << "panic: different type when not logic expression"
                << std::endl;
      exit(0);
    }
  }
  return nullptr;
}

llvm::Value* ASTCallExpression::generate(ASTContext* astcontext) {
  //获取函数
  llvm::Function* func = astcontext->current_m->getFunction(this->callee);
  //异常处理
  if (!func) {
    std::cout << "Unknown Function referenced" << std::endl;
    return nullptr;
  }

  //构造参数
  std::vector<llvm::Value*> putargs;
  for (auto arg : this->args) putargs.push_back(arg->generate(astcontext));
  //调用
  return astcontext->builder->CreateCall(func, putargs);
}

llvm::Value* ASTIfExpression::generate(ASTContext* astcontext) {
  auto condori = this->condition->generate(astcontext);  // 原始condition
  auto condinst = astcontext->generate_condition(condori);

  auto ori = astcontext->builder->GetInsertBlock();

  this->ifcode->generate(astcontext);
  this->elsecode->generate(astcontext);
  astcontext->builder->SetInsertPoint(ori);

  auto brinst = astcontext->builder->CreateCondBr(
      condinst, ifcode->entryBB,
      elsecode->entryBB);  // 设置到if/else基本块开头的branch

  if (this->ifcode->check_return() && this->elsecode->check_return()) {
    set_return();
    return nullptr;
  }

  auto edBB = llvm::BasicBlock::Create(
      *(astcontext->context), "",
      astcontext->current_f);  // 设置if/else结束后汇总到这个bb

  if (!ifcode->check_return()) {
    astcontext->builder->SetInsertPoint(
        ifcode->get_exit());  // 设置在ifBB的末尾
    astcontext->builder->CreateBr(edBB);
  }
  if (!elsecode->check_return()) {
    std::cout << elsecode->get_class_name() << std::endl;
    astcontext->builder->SetInsertPoint(
        elsecode->get_exit());  // 设置在elseBB的末尾
    astcontext->builder->CreateBr(edBB);
  }
  astcontext->builder->SetInsertPoint(edBB);
  return brinst;
}

llvm::Value* ASTWhileExpression::generate(ASTContext* astcontext) {
  auto ori = llvm::BasicBlock::Create(*(astcontext->context), "",
                                      astcontext->current_f);
  astcontext->builder->CreateBr(ori);
  astcontext->builder->SetInsertPoint(ori);

  auto condori = this->condition->generate(astcontext);  // 原始condition
  auto condinst = astcontext->generate_condition(condori);

  this->code->generate(astcontext);
  astcontext->builder->SetInsertPoint(ori);

  auto edBB = llvm::BasicBlock::Create(*(astcontext->context), "",
                                       astcontext->current_f);  // 设置while循环

  auto brinst = astcontext->builder->CreateCondBr(
      condinst, this->code->entryBB,
      edBB);  // 设置到if/else基本块开头的branch

  if (!this->code->check_return()) {
    astcontext->builder->SetInsertPoint(
        this->code->get_exit());  // 设置在ifBB的末尾
    astcontext->builder->CreateBr(ori);
  }
  astcontext->builder->SetInsertPoint(edBB);
  return nullptr;
}

llvm::Value* ASTForExpression::generate(ASTContext* astcontext) {
  auto lst = astcontext->builder->GetInsertBlock();
  this->prepare->generate(astcontext);
  astcontext->builder->SetInsertPoint(lst);  // 从last -> prepare
  astcontext->builder->CreateBr(prepare->entryBB);
  astcontext->builder->SetInsertPoint(prepare->get_exit());

  auto stBB = llvm::BasicBlock::Create(*(astcontext->context), "",
                                       astcontext->current_f);
  astcontext->builder->CreateBr(stBB);  // 从prepare -> startBB
  astcontext->builder->SetInsertPoint(stBB);

  auto condori = this->condition->generate(astcontext);  // 原始condition

  auto condinst = astcontext->generate_condition(condori);

  this->code->generate(astcontext);

  auto edBB = llvm::BasicBlock::Create(
      *(astcontext->context), "",
      astcontext->current_f);  // 设置if/else结束后汇总到这个bb

  astcontext->builder->SetInsertPoint(stBB);
  auto brinst = astcontext->builder->CreateCondBr(condinst, this->code->entryBB,
                                                  edBB);  // 设置for循环

  if (!this->code->check_return()) {
    action->generate(astcontext);
    astcontext->builder->CreateBr(stBB);  // 设置action跳回condition

    astcontext->builder->SetInsertPoint(
        this->code->get_exit());  // 设置在从code跳到action
    astcontext->builder->CreateBr(action->entryBB);
  }
  astcontext->builder->SetInsertPoint(edBB);
  return brinst;
}

llvm::Value* ASTVariableExpression::generate_ptr(ASTContext* astcontext) {
  return astcontext->get_var(name);
}

llvm::Value* ASTArrayExpression::generate(ASTContext* astcontext) {
  auto ptr = this->generate_ptr(astcontext);
  auto var_load = astcontext->builder->CreateLoad(
      ptr->getType()->getPointerElementType(), ptr,
      "");  // 将它load为右值 xxx_load = load xxx
  return var_load;
}

llvm::Value* ASTArrayExpression::generate_ptr(ASTContext* astcontext) {
  auto realindex = index->generate(astcontext);

  llvm::SmallVector<llvm::Value*, 2> index_vec;
  auto off = new ASTInteger(0);
  index_vec.push_back(off->generate(astcontext));
  index_vec.push_back(realindex);

  auto var = astcontext->get_var(get_name());
  auto ptr = astcontext->builder->CreateGEP(
      var->getType()->getPointerElementType(), var, index_vec,
      "");  // create get element pointer
  return ptr;
}

llvm::Value* ASTArrayAssign::generate(ASTContext* astcontext) {
  auto var = this->lhs->generate_ptr(astcontext);
  auto assign =
      astcontext->builder->CreateStore(this->rhs->generate(astcontext), var);
  return assign;
}

llvm::Value* ASTSingleExpression::generate(ASTContext* astcontext) {
  if (operation == OP_SI_ADDRESS) {
    auto inst = this->exp->generate_ptr(astcontext);
    return inst;
  } else if (operation == OP_SI_NOT) {
    auto code = this->exp->generate(astcontext);
    auto condition = astcontext->generate_condition(code);
    return astcontext->builder->CreateNot(condition);
  }
  std::cout << "panic: not support single expression" << std::endl;
  return nullptr;
}