#pragma once
#include "parser.h"
#include <memory>
#include <vector>

// AST Node types
enum class ASTNodeType {
    PROGRAM,
    VARIABLE_DECLARATION,
    STDOUT_STATEMENT,
    BINARY_OPERATION,
    IDENTIFIER,
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_STRING,
    LITERAL_BOOL,
    STRING_INTERPOLATION,
};

class ASTNode {
  public:
    ASTNodeType type;
    int line, column;

    ASTNode(ASTNodeType t, int l, int c) : type(t), line(l), column(c) {}
    virtual ~ASTNode() = default;
};

class ProgramNode : public ASTNode {
  public:
    std::vector<std::unique_ptr<ASTNode>> statements;

    ProgramNode(int line, int column) : ASTNode(ASTNodeType::PROGRAM, line, column) {}
};
