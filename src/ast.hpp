#pragma once
#include "parser.hpp"
#include "error.hpp"
#include <memory>
#include <vector>

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
    ARRAY_LITERAL,
    ARRAY_DECLARATION,
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

class VariableDeclarationNode : public ASTNode {
public:
    std::string varName;
    Token varType;
    std::unique_ptr<ASTNode> value;
    
    VariableDeclarationNode(const std::string& name, Token type, std::unique_ptr<ASTNode> val, int line, int column)
        : ASTNode(ASTNodeType::VARIABLE_DECLARATION, line, column), varName(name), varType(type), value(std::move(val)) {}
};

class StdoutStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> content;
    
    StdoutStatementNode(std::unique_ptr<ASTNode> cont, int line, int column)
        : ASTNode(ASTNodeType::STDOUT_STATEMENT, line, column), content(std::move(cont)) {}
};

class BinaryOperationNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    Token op;
    
    BinaryOperationNode(std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r, Token operation, int line, int column)
        : ASTNode(ASTNodeType::BINARY_OPERATION, line, column), left(std::move(l)), right(std::move(r)), op(operation) {}
};

class IdentifierNode : public ASTNode {
public:
    std::string name;
    
    IdentifierNode(const std::string& n, int line, int column)
        : ASTNode(ASTNodeType::IDENTIFIER, line, column), name(n) {}
};

class LiteralIntNode : public ASTNode {
public:
    int value;
    
    LiteralIntNode(int val, int line, int column)
        : ASTNode(ASTNodeType::LITERAL_INT, line, column), value(val) {}
};

class LiteralFloatNode : public ASTNode {
public:
    float value;
    
    LiteralFloatNode(float val, int line, int column)
        : ASTNode(ASTNodeType::LITERAL_FLOAT, line, column), value(val) {}
};

class LiteralStringNode : public ASTNode {
public:
    std::string value;
    
    LiteralStringNode(const std::string& val, int line, int column)
        : ASTNode(ASTNodeType::LITERAL_STRING, line, column), value(val) {}
};

class LiteralBoolNode : public ASTNode {
public:
    bool value;
    
    LiteralBoolNode(bool val, int line, int column)
        : ASTNode(ASTNodeType::LITERAL_BOOL, line, column), value(val) {}
};

class StringInterpolationNode : public ASTNode {
public:
    std::vector<std::string> parts;
    std::vector<std::unique_ptr<ASTNode>> expressions;
    
    StringInterpolationNode(int line, int column)
        : ASTNode(ASTNodeType::STRING_INTERPOLATION, line, column) {}
};

class ArrayLiteralNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> elements;
    
    ArrayLiteralNode(int line, int column)
        : ASTNode(ASTNodeType::ARRAY_LITERAL, line, column) {}
};

class ArrayDeclarationNode : public ASTNode {
public:
    std::string varName;
    Token elementType;  // Type for uninitialized arrays
    bool hasType;       // Whether type is explicitly specified
    bool hasSize;       // Whether size is specified
    int size;           // Size for uninitialized arrays
    std::unique_ptr<ASTNode> initializer;  // For initialized arrays
    
    ArrayDeclarationNode(const std::string& name, int line, int column)
        : ASTNode(ASTNodeType::ARRAY_DECLARATION, line, column), 
          varName(name), elementType(UNKNOWN), hasType(false), hasSize(false), size(0) {}
};

// AST Parser class
class ASTParser {
public:
    static std::unique_ptr<ProgramNode> parseProgram(Parser& parser);
    static std::unique_ptr<ASTNode> parseStatement(Parser& parser);
    static std::unique_ptr<VariableDeclarationNode> parseVariableDeclaration(Parser& parser);
    static std::unique_ptr<VariableDeclarationNode> parseBoolDeclaration(Parser& parser);
    static std::unique_ptr<StdoutStatementNode> parseStdoutStatement(Parser& parser);
    static std::unique_ptr<StringInterpolationNode> parseStringInterpolation(Parser& parser);
    static std::unique_ptr<ASTNode> parseExpression(Parser& parser);
    static std::unique_ptr<ASTNode> parsePrimary(Parser& parser);
    static std::unique_ptr<ArrayLiteralNode> parseArrayLiteral(Parser& parser);
    static std::unique_ptr<ArrayDeclarationNode> parseArrayDeclaration(Parser& parser);
    
    // Utility functions
    static std::string astTypeToString(ASTNodeType type);
    static void printAST(const ASTNode* node, int indent = 0);
    static std::string astToString(const ASTNode* node, int indent);
    static void buildASTString(const ASTNode* node, int indent, std::string &out);
    static void synchronizeParser(Parser& parser);
};
