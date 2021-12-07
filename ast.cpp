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

/* ------------ ASTFunctionImplementaion ------------- */
void ASTFunctionImp::set_entry(ASTCodeBlockExpression* codeblock) {
  this->function_entry = codeblock;
  return;
}
