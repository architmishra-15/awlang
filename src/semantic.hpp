#pragma once
#include "ast.hpp"
#include <unordered_map>
#include <string>

enum class ValueType {
    STRING_TYPE,
    INT_TYPE,
    FLOAT_TYPE,
    BOOL_TYPE,
    ARRAY_TYPE,
    UNKNOWN_TYPE
};

struct VariableInfo {
    ValueType type;
    bool isArray;
    int line;
    int column;
    
    VariableInfo(ValueType t, bool arr, int l, int c) 
        : type(t), isArray(arr), line(l), column(c) {}
};

class SemanticAnalyzer {
private:
    std::unordered_map<std::string, VariableInfo> symbolTable;
    std::unordered_map<std::string, bool> usedVariables; // Track variable usage
    
    void checkUnusedVariables();
    bool isCompatibleType(ValueType from, ValueType to);
    
public:
    bool analyzeProgram(const ProgramNode* program);
    bool analyzeStatement(const ASTNode* stmt);
    ValueType analyzeExpression(const ASTNode* expr);
    
    void declareVariable(const std::string& name, ValueType type, bool isArray, int line, int col);
    void markVariableUsed(const std::string& name);
    bool isVariableDeclared(const std::string& name);
    ValueType getVariableType(const std::string& name);
    
    static ValueType tokenToValueType(Token token);
    static std::string valueTypeToString(ValueType type);
};