// 用于控制语法分析树的结点/BasicBlock
#include "ast.h"

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
    std::map<std::string, llvm::Value*> table) {
  symboltable = table;
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

std::string getDefineStr(int type, int value) {
  if (type == 0) {
    switch (value) {
      case OP_BI_ADD:
        return "+";
      case OP_BI_SUB:
        return "-";
      case OP_BI_MUL:
        return "%";
      case OP_BI_DIV:
        return "/";
      case OP_BI_LESS:
        return "<";
      case OP_BI_MORE:
        return ">";
      case OP_BI_LESSEQ:
        return "<=";
      case OP_BI_MOREEQ:
        return ">=";
      case OP_BI_AND:
        return "&&";
      case OP_BI_OR:
        return "||";
      case OP_BI_MOD:
        return "%";
      case OP_BI_EQ:
        return "==";
      case OP_BI_NEQ:
        return "!=";
      case OP_SI_ADDRESS:
        return "&";
      case OP_SI_NOT:
        return "!";
    }
  } else if (type == 1) {
    switch (value) {
      case TYPE_VOID:
        return "void";
      case TYPE_INT:
        return "int";
      case TYPE_DOUBLE:
        return "double";
      case TYPE_CHAR:
        return "char";
      case TYPE_FLOAT:
        return "float";
      case TYPE_BOOL:
        return "bool";
      case TYPE_INT_PTR:
        return "int*";
      case TYPE_CHAR_PTR:
        return "char*";
    }
  }
  return "";
}