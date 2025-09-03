#include "semantic.hpp"

bool SemanticAnalyzer::analyzeProgram(const ProgramNode *program) {
  bool success = true;

  // Analyze all statements
  for (const auto &stmt : program->statements) {
    if (!analyzeStatement(stmt.get())) {
      success = false;
    }
  }

  // Check for unused variables (warnings)
  checkUnusedVariables();

  return success;
}

bool SemanticAnalyzer::analyzeStatement(const ASTNode *stmt) {
  switch (stmt->type) {
  case ASTNodeType::VARIABLE_DECLARATION: {
    const VariableDeclarationNode *varDecl =
        static_cast<const VariableDeclarationNode *>(stmt);

    // Check if variable already exists
    if (isVariableDeclared(varDecl->varName)) {
      g_errorHandler.addSemanticError(
          "Variable '" + varDecl->varName + "' is already declared", stmt->line,
          stmt->column,
          "Use a different variable name or remove the duplicate declaration");
      return false;
    }

    // Analyze the value expression
    ValueType valueType = analyzeExpression(varDecl->value.get());
    ValueType declaredType = tokenToValueType(varDecl->varType);

    // Check type compatibility
    if (valueType != ValueType::UNKNOWN_TYPE &&
        declaredType != ValueType::UNKNOWN_TYPE && valueType != declaredType) {
      g_errorHandler.addSemanticError(
          "Type mismatch: cannot assign " + valueTypeToString(valueType) +
              " to variable of type " + valueTypeToString(declaredType),
          stmt->line, stmt->column,
          "Change the variable type or provide a value of the correct type");
      return false;
    }

    // Declare the variable
    declareVariable(varDecl->varName, declaredType, false, stmt->line, stmt->column);
    return true;
  }

  case ASTNodeType::STDOUT_STATEMENT: {
    const StdoutStatementNode *stdoutStmt =
        static_cast<const StdoutStatementNode *>(stmt);
    return analyzeExpression(stdoutStmt->content.get()) !=
           ValueType::UNKNOWN_TYPE;
  }

  case ASTNodeType::ARRAY_DECLARATION: {
    const ArrayDeclarationNode *arrayDecl =
        static_cast<const ArrayDeclarationNode *>(stmt);

    if (isVariableDeclared(arrayDecl->varName)) {
      g_errorHandler.addSemanticError(
          "Array '" + arrayDecl->varName + "' is already declared", stmt->line,
          stmt->column, "Use a different array name");
      return false;
    }

    ValueType elementType = ValueType::UNKNOWN_TYPE;
    if (arrayDecl->hasType) {
      elementType = tokenToValueType(arrayDecl->elementType);
    }

    // If array has initializer, check element types and infer type if needed
    if (arrayDecl->initializer) {
      ValueType initType = analyzeExpression(arrayDecl->initializer.get());
      if (initType != ValueType::ARRAY_TYPE) {
        g_errorHandler.addSemanticError(
            "Array initializer must be an array literal", stmt->line,
            stmt->column,
            "Use [element1, element2, ...] syntax for array initialization");
        return false;
      }
      
      // Infer element type from first element if no explicit type provided
      if (elementType == ValueType::UNKNOWN_TYPE) {
        const ArrayLiteralNode *arrayLit = static_cast<const ArrayLiteralNode *>(arrayDecl->initializer.get());
        if (!arrayLit->elements.empty()) {
          elementType = analyzeExpression(arrayLit->elements[0].get());
        }
      }
    }

    declareVariable(arrayDecl->varName, elementType, true, stmt->line,
                    stmt->column);
    return true;
  }

  default:
    return true;
  }
}

ValueType SemanticAnalyzer::analyzeExpression(const ASTNode *expr) {
  switch (expr->type) {
  case ASTNodeType::LITERAL_INT:
    return ValueType::INT_TYPE;
  case ASTNodeType::LITERAL_FLOAT:
    return ValueType::FLOAT_TYPE;
  case ASTNodeType::LITERAL_STRING:
    return ValueType::STRING_TYPE;
  case ASTNodeType::LITERAL_BOOL:
    return ValueType::BOOL_TYPE;

  case ASTNodeType::IDENTIFIER: {
    const IdentifierNode *id = static_cast<const IdentifierNode *>(expr);
    if (!isVariableDeclared(id->name)) {
      g_errorHandler.addSemanticError("Undefined variable '" + id->name + "'",
                                      expr->line, expr->column,
                                      "Declare the variable before using it");
      return ValueType::UNKNOWN_TYPE;
    }

    // Mark variable as used
    markVariableUsed(id->name);
    return getVariableType(id->name);
  }

  case ASTNodeType::BINARY_OPERATION: {
    const BinaryOperationNode *binOp =
        static_cast<const BinaryOperationNode *>(expr);
    ValueType leftType = analyzeExpression(binOp->left.get());
    ValueType rightType = analyzeExpression(binOp->right.get());

    // Type checking for arithmetic operations
    if (binOp->op == ADD || binOp->op == SUB || binOp->op == MUL ||
        binOp->op == DIV) {
      // String concatenation with +
      if (binOp->op == ADD && (leftType == ValueType::STRING_TYPE ||
                               rightType == ValueType::STRING_TYPE)) {
        return ValueType::STRING_TYPE;
      }

      // Arithmetic operations on strings (except +) are invalid
      if (leftType == ValueType::STRING_TYPE ||
          rightType == ValueType::STRING_TYPE) {
        g_errorHandler.addSemanticError(
            "Cannot perform arithmetic operations on strings", expr->line,
            expr->column, "Use string concatenation (+) or convert to numbers");
        return ValueType::UNKNOWN_TYPE;
      }

      // Type promotion: if either operand is float, result is float
      if (leftType == ValueType::FLOAT_TYPE ||
          rightType == ValueType::FLOAT_TYPE) {
        return ValueType::FLOAT_TYPE;
      }

      // Both are integers
      if (leftType == ValueType::INT_TYPE && rightType == ValueType::INT_TYPE) {
        return ValueType::INT_TYPE;
      }

      // Type mismatch
      g_errorHandler.addSemanticError(
          "Type mismatch in arithmetic operation: " +
              valueTypeToString(leftType) + " and " +
              valueTypeToString(rightType),
          expr->line, expr->column, "Ensure both operands are numbers");
      return ValueType::UNKNOWN_TYPE;
    }

    // Comparison operations
    if (binOp->op == EQUAL || binOp->op == NOT_EQUAL || binOp->op == GREATER ||
        binOp->op == LESSER || binOp->op == GREATER_EQUAL ||
        binOp->op == LESSER_EQUAL) {

      // Can compare same types
      if (leftType == rightType) {
        return ValueType::BOOL_TYPE;
      }

      // Can compare int and float
      if ((leftType == ValueType::INT_TYPE &&
           rightType == ValueType::FLOAT_TYPE) ||
          (leftType == ValueType::FLOAT_TYPE &&
           rightType == ValueType::INT_TYPE)) {
        return ValueType::BOOL_TYPE;
      }

      g_errorHandler.addSemanticError(
          "Cannot compare " + valueTypeToString(leftType) + " with " +
              valueTypeToString(rightType),
          expr->line, expr->column,
          "Ensure both operands are of compatible types");
      return ValueType::UNKNOWN_TYPE;
    }

    return leftType; // Default return
  }

  case ASTNodeType::STRING_INTERPOLATION: {
    const StringInterpolationNode *strInterp =
        static_cast<const StringInterpolationNode *>(expr);

    // Check all interpolated expressions
    for (const auto &subExpr : strInterp->expressions) {
      ValueType exprType = analyzeExpression(subExpr.get());
      if (exprType == ValueType::UNKNOWN_TYPE) {
        return ValueType::UNKNOWN_TYPE;
      }
    }

    return ValueType::STRING_TYPE;
  }

  case ASTNodeType::ARRAY_LITERAL: {
    const ArrayLiteralNode *arrayLit =
        static_cast<const ArrayLiteralNode *>(expr);

    if (arrayLit->elements.empty()) {
      return ValueType::ARRAY_TYPE;
    }

    // Check that all elements have the same type
    ValueType firstElementType = analyzeExpression(arrayLit->elements[0].get());
    for (size_t i = 1; i < arrayLit->elements.size(); i++) {
      ValueType elementType = analyzeExpression(arrayLit->elements[i].get());
      if (elementType != firstElementType) {
        g_errorHandler.addSemanticError(
            "Array elements must have the same type", expr->line, expr->column,
            "Ensure all array elements are of type " +
                valueTypeToString(firstElementType));
        return ValueType::UNKNOWN_TYPE;
      }
    }

    return ValueType::ARRAY_TYPE;
  }

  default:
    return ValueType::UNKNOWN_TYPE;
  }
}

void SemanticAnalyzer::declareVariable(const std::string &name, ValueType type,
                                       bool isArray, int line, int col) {
  symbolTable.emplace(name, VariableInfo(type, isArray, line, col));
}

bool SemanticAnalyzer::isVariableDeclared(const std::string &name) {
  return symbolTable.find(name) != symbolTable.end();
}

ValueType SemanticAnalyzer::getVariableType(const std::string &name) {
  auto it = symbolTable.find(name);
  if (it != symbolTable.end()) {
    return it->second.type;
  }
  return ValueType::UNKNOWN_TYPE;
}

ValueType SemanticAnalyzer::tokenToValueType(Token token) {
  switch (token) {
  case STRING:
    return ValueType::STRING_TYPE;
  case INTEGER:
    return ValueType::INT_TYPE;
  case FLOAT:
    return ValueType::FLOAT_TYPE;
  case BOOL:
    return ValueType::BOOL_TYPE;
  default:
    return ValueType::UNKNOWN_TYPE;
  }
}

std::string SemanticAnalyzer::valueTypeToString(ValueType type) {
  switch (type) {
  case ValueType::STRING_TYPE:
    return "string";
  case ValueType::INT_TYPE:
    return "int";
  case ValueType::FLOAT_TYPE:
    return "float";
  case ValueType::BOOL_TYPE:
    return "bool";
  case ValueType::ARRAY_TYPE:
    return "array";
  case ValueType::UNKNOWN_TYPE:
    return "unknown";
  default:
    return "unknown";
  }
}
void SemanticAnalyzer::markVariableUsed(const std::string &name) {
  usedVariables[name] = true;
}

void SemanticAnalyzer::checkUnusedVariables() {
  for (const auto &var : symbolTable) {
    const std::string &varName = var.first;
    const VariableInfo &info = var.second;

    // Check if variable was never used
    if (usedVariables.find(varName) == usedVariables.end()) {
      g_errorHandler.addError(
          ErrorType::WARNING, "Unused variable '" + varName + "'", info.line,
          info.column, "Remove this variable or use it in your code");
    }
  }
}

bool SemanticAnalyzer::isCompatibleType(ValueType from, ValueType to) {
  if (from == to)
    return true;

  // Allow implicit conversion from int to float
  if (from == ValueType::INT_TYPE && to == ValueType::FLOAT_TYPE) {
    return true;
  }

  return false;
}