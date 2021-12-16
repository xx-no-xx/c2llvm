// 用于控制语法分析树的结点/BasicBlock
#include "ast.h"

void foo() {}

/* ------------ ASTCodeBlockExpression ------------- */

void ASTCodeBlockExpression::set_function(ASTFunctionImp* function) {
  function->set_entry(this);
  return;
}

void ASTCodeBlockExpression::append_code(ASTNode* exp) {
  this->codes.push_back(exp);
  return;
}

void ASTCodeBlockExpression::clear_symbol(void) {
  this->symboltable.clear();
  return;
}

void ASTCodeBlockExpression::copy_symbol_from(
    ASTCodeBlockExpression* codeblock) {
  symboltable = codeblock->get_symboltable();
  return;
}

bool ASTCodeBlockExpression::add_symbol(std::string symbol_name,
                                        llvm::Value* symbol) {
  if (this->symboltable.count(symbol_name) > 0) {
    return false;
  }
  this->symboltable[symbol_name] = symbol;
  return true;
}

llvm::Value* ASTCodeBlockExpression::get_symbol(std::string name) {
  if (symboltable.count(name) == 0) {
    return nullptr;
  }
  return symboltable[name];
}

/* ------------ ASTFunctionImplementaion ------------- */
void ASTFunctionImp::set_entry(ASTCodeBlockExpression* codeblock) {
  this->function_entry = codeblock;
  return;
}
