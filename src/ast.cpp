#include "ast.hpp"
#include <iostream>
#include <sstream>

std::unique_ptr<ProgramNode> ASTParser::parseProgram(Parser& parser) {
    auto program = std::make_unique<ProgramNode>(1, 1);
    
    // Parse statements until EOF
    while (ParserEngine::currentToken(parser) && 
           ParserEngine::currentToken(parser)->type != END_OF_FILE) {
        
        auto stmt = parseStatement(parser);
        if (stmt) {
            program->statements.push_back(std::move(stmt));
        } else {
            // Skip invalid tokens and continue
            ParserEngine::advanceParser(parser);
        }
    }
    
    return program;
}

std::unique_ptr<ASTNode> ASTParser::parseStatement(Parser& parser) {
    TokenData* token = ParserEngine::currentToken(parser);
    if (!token) return nullptr;
    
    // Skip comments
    while (token && token->type == COMMENT) {
        ParserEngine::advanceParser(parser);
        token = ParserEngine::currentToken(parser);
    }
    
    if (!token) return nullptr;
    
    switch (token->type) {
        case NEW:
            // Check if it's array declaration
            if (ParserEngine::peekToken(parser) && 
                ParserEngine::peekToken(parser)->type == IDENTIFIER) {
                // Look ahead further to see if it's array syntax
                Parser tempParser = parser;
                ParserEngine::advanceParser(tempParser); // skip 'new'
                ParserEngine::advanceParser(tempParser); // skip identifier
                TokenData* nextToken = ParserEngine::currentToken(tempParser);
                if (nextToken && (nextToken->type == ARRAY_OPEN || nextToken->type == TYPE_OPEN)) {
                    return parseArrayDeclaration(parser);
                }
            }
            return parseVariableDeclaration(parser);
        
        case BL:
            return parseBoolDeclaration(parser);
            
        case STDOUT:
            return parseStdoutStatement(parser);
            
        default:
            ParserEngine::parserError(parser, "Unexpected token at start of statement");
            return nullptr;
    }
}

std::unique_ptr<VariableDeclarationNode> ASTParser::parseVariableDeclaration(Parser& parser) {
    TokenData* token = ParserEngine::currentToken(parser);
    int line = token->line, column = token->column;
    
    // Consume 'new'
    if (!ParserEngine::consumeToken(parser, NEW)) {
        ParserEngine::parserError(parser, "Expected 'new' keyword");
        return nullptr;
    }
    
    // Get variable name
    token = ParserEngine::currentToken(parser);
    if (!token || token->type != IDENTIFIER) {
        ParserEngine::parserError(parser, "Expected variable name after 'new'");
        return nullptr;
    }
    std::string varName = token->value;
    ParserEngine::advanceParser(parser);
    
    // Get variable type
    token = ParserEngine::currentToken(parser);
    if (!token || (token->type != STRING && token->type != INTEGER && 
                   token->type != FLOAT && token->type != BOOL)) {
        ParserEngine::parserError(parser, "Expected type (string, int, float, bool) after variable name");
        return nullptr;
    }
    Token varType = token->type;
    ParserEngine::advanceParser(parser);
    
    // Expect '='
    if (!ParserEngine::consumeToken(parser, ASSIGNMENT)) {
        ParserEngine::parserError(parser, "Expected '=' after variable type");
        return nullptr;
    }
    
    // Parse the value expression
    auto value = parseExpression(parser);
    if (!value) {
        ParserEngine::parserError(parser, "Expected value after '='");
        return nullptr;
    }
    
    return std::make_unique<VariableDeclarationNode>(varName, varType, std::move(value), line, column);
}

std::unique_ptr<VariableDeclarationNode> ASTParser::parseBoolDeclaration(Parser& parser) {
    TokenData* token = ParserEngine::currentToken(parser);
    int line = token->line, column = token->column;
    
    // Consume 'bl'
    if (!ParserEngine::consumeToken(parser, BL)) {
        ParserEngine::parserError(parser, "Expected 'bl' keyword");
        return nullptr;
    }
    
    // Get variable name
    token = ParserEngine::currentToken(parser);
    if (!token || token->type != IDENTIFIER) {
        ParserEngine::parserError(parser, "Expected variable name after 'bl'");
        return nullptr;
    }
    std::string varName = token->value;
    ParserEngine::advanceParser(parser);
    
    // Expect '='
    if (!ParserEngine::consumeToken(parser, ASSIGNMENT)) {
        ParserEngine::parserError(parser, "Expected '=' after variable name");
        return nullptr;
    }
    
    // Parse the value expression
    auto value = parseExpression(parser);
    if (!value) {
        ParserEngine::parserError(parser, "Expected value after '='");
        return nullptr;
    }
    
    return std::make_unique<VariableDeclarationNode>(varName, BOOL, std::move(value), line, column);
}

std::unique_ptr<StdoutStatementNode> ASTParser::parseStdoutStatement(Parser& parser) {
    TokenData* token = ParserEngine::currentToken(parser);
    int line = token->line, column = token->column;
    
    // Consume 'stdout'
    if (!ParserEngine::consumeToken(parser, STDOUT)) {
        ParserEngine::parserError(parser, "Expected 'stdout' keyword");
        return nullptr;
    }
    
    // Expect '['
    token = ParserEngine::currentToken(parser);
    if (!token || token->type != ARRAY_OPEN) {
        ParserEngine::parserError(parser, "Expected '[' after 'stdout'");
        return nullptr;
    }
    ParserEngine::advanceParser(parser); // consume '['
    
    // Parse string interpolation content
    auto interpolationNode = parseStringInterpolation(parser);
    if (!interpolationNode) {
        ParserEngine::parserError(parser, "Failed to parse stdout content");
        return nullptr;
    }
    
    // Expect ']'
    if (!ParserEngine::consumeToken(parser, ARRAY_CLOSE)) {
        ParserEngine::parserError(parser, "Expected ']' after stdout content");
        return nullptr;
    }
    
    return std::make_unique<StdoutStatementNode>(std::move(interpolationNode), line, column);
}

std::unique_ptr<StringInterpolationNode> ASTParser::parseStringInterpolation(Parser& parser) {
    TokenData* token = ParserEngine::currentToken(parser);
    int line = token->line, column = token->column;
    
    auto interpolationNode = std::make_unique<StringInterpolationNode>(line, column);
    std::string currentTextPart;
    
    while (token && token->type != ARRAY_CLOSE && token->type != END_OF_FILE) {
        if (token->type == TYPE_OPEN) { // '{'
            // Save current text part if any
            if (!currentTextPart.empty()) {
                interpolationNode->parts.push_back(currentTextPart);
                currentTextPart.clear();
            }
            
            ParserEngine::advanceParser(parser); // consume '{'
            
            // Parse the variable inside {}
            token = ParserEngine::currentToken(parser);
            if (!token || token->type != IDENTIFIER) {
                ParserEngine::parserError(parser, "Expected variable name inside {}");
                return nullptr;
            }
            
            auto varNode = std::make_unique<IdentifierNode>(token->value, token->line, token->column);
            interpolationNode->expressions.push_back(std::move(varNode));
            ParserEngine::advanceParser(parser);
            
            // Expect '}'
            if (!ParserEngine::consumeToken(parser, TYPE_CLOSE)) {
                ParserEngine::parserError(parser, "Expected '}' after variable name");
                return nullptr;
            }
            
            token = ParserEngine::currentToken(parser);
        } else {
            // Collect regular text
            if (!currentTextPart.empty()) currentTextPart += " ";
            currentTextPart += token->value;
            ParserEngine::advanceParser(parser);
            token = ParserEngine::currentToken(parser);
        }
    }
    
    // Save any remaining text part
    if (!currentTextPart.empty()) {
        interpolationNode->parts.push_back(currentTextPart);
    }
    
    return interpolationNode;
}

std::unique_ptr<ASTNode> ASTParser::parseExpression(Parser& parser) {
    auto left = parsePrimary(parser);
    if (!left) return nullptr;
    
    TokenData* token = ParserEngine::currentToken(parser);
    if (!token) return left;
    
    // Handle binary operators
    if (token->type == ADD || token->type == SUB || token->type == MUL || 
        token->type == DIV || token->type == EQUAL || token->type == NOT_EQUAL ||
        token->type == GREATER || token->type == LESSER || 
        token->type == GREATER_EQUAL || token->type == LESSER_EQUAL) {
        
        Token op = token->type;
        int line = token->line, column = token->column;
        ParserEngine::advanceParser(parser);
        
        auto right = parseExpression(parser);
        if (!right) {
            ParserEngine::parserError(parser, "Expected right operand");
            return nullptr;
        }
        
        return std::make_unique<BinaryOperationNode>(std::move(left), std::move(right), op, line, column);
    }
    
    return left;
}

std::unique_ptr<ASTNode> ASTParser::parsePrimary(Parser& parser) {
    TokenData* token = ParserEngine::currentToken(parser);
    if (!token) {
        ParserEngine::parserError(parser, "Unexpected end of input");
        return nullptr;
    }
    
    switch (token->type) {
        case INTEGER: {
            int value = std::stoi(token->value);
            ParserEngine::advanceParser(parser);
            return std::make_unique<LiteralIntNode>(value, token->line, token->column);
        }
        
        case FLOAT: {
            float value = std::stof(token->value);
            ParserEngine::advanceParser(parser);
            return std::make_unique<LiteralFloatNode>(value, token->line, token->column);
        }
        
        case STRING: {
            auto node = std::make_unique<LiteralStringNode>(token->value, token->line, token->column);
            ParserEngine::advanceParser(parser);
            return std::move(node);
        }
        
        case TRUE_VAL: {
            ParserEngine::advanceParser(parser);
            return std::make_unique<LiteralBoolNode>(true, token->line, token->column);
        }
        
        case FALSE_VAL: {
            ParserEngine::advanceParser(parser);
            return std::make_unique<LiteralBoolNode>(false, token->line, token->column);
        }
        
        case IDENTIFIER: {
            auto node = std::make_unique<IdentifierNode>(token->value, token->line, token->column);
            ParserEngine::advanceParser(parser);
            return std::move(node);
        }
        
        case LPAREN: {
            ParserEngine::advanceParser(parser); // consume '('
            auto expr = parseExpression(parser);
            if (!ParserEngine::consumeToken(parser, RPAREN)) {
                ParserEngine::parserError(parser, "Expected ')' after expression");
                return nullptr;
            }
            return expr;
        }
        
        case ARRAY_OPEN: {
            return parseArrayLiteral(parser);
        }
        
        default:
            ParserEngine::parserError(parser, "Expected primary expression");
            return nullptr;
    }
}

std::string ASTParser::astTypeToString(ASTNodeType type) {
    switch (type) {
        case ASTNodeType::PROGRAM: return "PROGRAM";
        case ASTNodeType::VARIABLE_DECLARATION: return "VAR_DECL";
        case ASTNodeType::STDOUT_STATEMENT: return "STDOUT";
        case ASTNodeType::BINARY_OPERATION: return "BINARY_OP";
        case ASTNodeType::IDENTIFIER: return "IDENTIFIER";
        case ASTNodeType::LITERAL_INT: return "INT_LITERAL";
        case ASTNodeType::LITERAL_FLOAT: return "FLOAT_LITERAL";
        case ASTNodeType::LITERAL_STRING: return "STRING_LITERAL";
        case ASTNodeType::LITERAL_BOOL: return "BOOL_LITERAL";
        case ASTNodeType::STRING_INTERPOLATION: return "STRING_INTERP";
        case ASTNodeType::ARRAY_LITERAL: return "ARRAY_LITERAL";
        case ASTNodeType::ARRAY_DECLARATION: return "ARRAY_DECL";
        default: return "UNKNOWN_AST";
    }
}

void ASTParser::printAST(const ASTNode* node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) std::cout << "  ";
    std::cout << astTypeToString(node->type);
    
    switch (node->type) {
        case ASTNodeType::PROGRAM: {
            const ProgramNode* program = static_cast<const ProgramNode*>(node);
            std::cout << " (" << program->statements.size() << " statements)" << std::endl;
            for (const auto& stmt : program->statements) {
                printAST(stmt.get(), indent + 1);
            }
            break;
        }
        
        case ASTNodeType::VARIABLE_DECLARATION: {
            const VariableDeclarationNode* varDecl = static_cast<const VariableDeclarationNode*>(node);
            std::cout << " '" << varDecl->varName << "' type=" 
                      << LexerEngine::tokenTypeToString(varDecl->varType) << std::endl;
            printAST(varDecl->value.get(), indent + 1);
            break;
        }
        
        case ASTNodeType::STDOUT_STATEMENT: {
            const StdoutStatementNode* stdoutStmt = static_cast<const StdoutStatementNode*>(node);
            std::cout << std::endl;
            printAST(stdoutStmt->content.get(), indent + 1);
            break;
        }
        
        case ASTNodeType::STRING_INTERPOLATION: {
            const StringInterpolationNode* stringInterp = static_cast<const StringInterpolationNode*>(node);
            std::cout << " [" << stringInterp->parts.size() << " parts, " 
                      << stringInterp->expressions.size() << " expressions]" << std::endl;
            
            // Print text parts and expressions interleaved
            size_t maxItems = std::max(stringInterp->parts.size(), stringInterp->expressions.size());
            for (size_t i = 0; i < maxItems; i++) {
                if (i < stringInterp->parts.size()) {
                    for (int j = 0; j < indent + 1; j++) std::cout << "  ";
                    std::cout << "TEXT_PART \"" << stringInterp->parts[i] << "\"" << std::endl;
                }
                if (i < stringInterp->expressions.size()) {
                    printAST(stringInterp->expressions[i].get(), indent + 1);
                }
            }
            break;
        }
        
        case ASTNodeType::BINARY_OPERATION: {
            const BinaryOperationNode* binaryOp = static_cast<const BinaryOperationNode*>(node);
            std::cout << " " << LexerEngine::tokenTypeToString(binaryOp->op) << std::endl;
            printAST(binaryOp->left.get(), indent + 1);
            printAST(binaryOp->right.get(), indent + 1);
            break;
        }
        
        case ASTNodeType::IDENTIFIER: {
            const IdentifierNode* identifier = static_cast<const IdentifierNode*>(node);
            std::cout << " '" << identifier->name << "'" << std::endl;
            break;
        }
        
        case ASTNodeType::LITERAL_INT: {
            const LiteralIntNode* intLiteral = static_cast<const LiteralIntNode*>(node);
            std::cout << " " << intLiteral->value << std::endl;
            break;
        }
        
        case ASTNodeType::LITERAL_FLOAT: {
            const LiteralFloatNode* floatLiteral = static_cast<const LiteralFloatNode*>(node);
            std::cout << " " << floatLiteral->value << std::endl;
            break;
        }
        
        case ASTNodeType::LITERAL_STRING: {
            const LiteralStringNode* stringLiteral = static_cast<const LiteralStringNode*>(node);
            std::cout << " \"" << stringLiteral->value << "\"" << std::endl;
            break;
        }
        
        case ASTNodeType::LITERAL_BOOL: {
            const LiteralBoolNode* boolLiteral = static_cast<const LiteralBoolNode*>(node);
            std::cout << " " << (boolLiteral->value ? "true" : "false") << std::endl;
            break;
        }
        
        case ASTNodeType::ARRAY_LITERAL: {
            const ArrayLiteralNode* arrayLiteral = static_cast<const ArrayLiteralNode*>(node);
            std::cout << " [" << arrayLiteral->elements.size() << " elements]" << std::endl;
            for (const auto& element : arrayLiteral->elements) {
                printAST(element.get(), indent + 1);
            }
            break;
        }
        
        case ASTNodeType::ARRAY_DECLARATION: {
            const ArrayDeclarationNode* arrayDecl = static_cast<const ArrayDeclarationNode*>(node);
            std::cout << " '" << arrayDecl->varName << "'";
            if (arrayDecl->hasType) {
                std::cout << " type=" << LexerEngine::tokenTypeToString(arrayDecl->elementType);
            }
            if (arrayDecl->hasSize) {
                std::cout << " size=" << arrayDecl->size;
            }
            std::cout << std::endl;
            if (arrayDecl->initializer) {
                printAST(arrayDecl->initializer.get(), indent + 1);
            }
            break;
        }
        
        default:
            std::cout << std::endl;
            break;
    }
}

std::string ASTParser::astToString(const ASTNode* node, int indent) {
    std::string out;
    out.reserve(1024); // pre-reserve to reduce reallocs for medium trees
    buildASTString(node, indent, out);
    return out;
}

// the worker that actually builds the string
void ASTParser::buildASTString(const ASTNode* node, int indent, std::string &out) {
    if (!node) return;

    // indentation: two spaces per indent level (matches your current print)
    out.append(static_cast<size_t>(indent) * 2, ' ');

    // node type name
    out += astTypeToString(node->type);

    switch (node->type) {
        case ASTNodeType::PROGRAM: {
            const ProgramNode* program = static_cast<const ProgramNode*>(node);
            out += " (" + std::to_string(program->statements.size()) + " statements)\n";
            for (const auto& stmt : program->statements) {
                buildASTString(stmt.get(), indent + 1, out);
            }
            break;
        }

        case ASTNodeType::VARIABLE_DECLARATION: {
            const VariableDeclarationNode* varDecl = static_cast<const VariableDeclarationNode*>(node);
            out += " '";
            out += varDecl->varName;
            out += "' type=";
            out += LexerEngine::tokenTypeToString(varDecl->varType);
            out += '\n';
            buildASTString(varDecl->value.get(), indent + 1, out);
            break;
        }

        case ASTNodeType::STDOUT_STATEMENT: {
            const StdoutStatementNode* stdoutStmt = static_cast<const StdoutStatementNode*>(node);
            out += '\n'; // print had a newline here
            buildASTString(stdoutStmt->content.get(), indent + 1, out);
            break;
        }

        case ASTNodeType::STRING_INTERPOLATION: {
            const StringInterpolationNode* stringInterp = static_cast<const StringInterpolationNode*>(node);
            out += " [" + std::to_string(stringInterp->parts.size()) + " parts, "
                        + std::to_string(stringInterp->expressions.size()) + " expressions]\n";

            size_t maxItems = std::max(stringInterp->parts.size(), stringInterp->expressions.size());
            for (size_t i = 0; i < maxItems; ++i) {
                if (i < stringInterp->parts.size()) {
                    out.append(static_cast<size_t>(indent + 1) * 2, ' ');
                    out += "TEXT_PART \"";
                    out += stringInterp->parts[i];
                    out += "\"\n";
                }
                if (i < stringInterp->expressions.size()) {
                    buildASTString(stringInterp->expressions[i].get(), indent + 1, out);
                }
            }
            break;
        }

        case ASTNodeType::BINARY_OPERATION: {
            const BinaryOperationNode* binaryOp = static_cast<const BinaryOperationNode*>(node);
            out += " ";
            out += LexerEngine::tokenTypeToString(binaryOp->op);
            out += '\n';
            buildASTString(binaryOp->left.get(), indent + 1, out);
            buildASTString(binaryOp->right.get(), indent + 1, out);
            break;
        }

        case ASTNodeType::IDENTIFIER: {
            const IdentifierNode* identifier = static_cast<const IdentifierNode*>(node);
            out += " '";
            out += identifier->name;
            out += "'\n";
            break;
        }

        case ASTNodeType::LITERAL_INT: {
            const LiteralIntNode* intLiteral = static_cast<const LiteralIntNode*>(node);
            out += " ";
            out += std::to_string(intLiteral->value);
            out += '\n';
            break;
        }

        case ASTNodeType::LITERAL_FLOAT: {
            const LiteralFloatNode* floatLiteral = static_cast<const LiteralFloatNode*>(node);
            out += " ";
            out += std::to_string(floatLiteral->value);
            out += '\n';
            break;
        }

        case ASTNodeType::LITERAL_STRING: {
            const LiteralStringNode* stringLiteral = static_cast<const LiteralStringNode*>(node);
            out += " \"";
            out += stringLiteral->value;
            out += "\"\n";
            break;
        }

        case ASTNodeType::LITERAL_BOOL: {
            const LiteralBoolNode* boolLiteral = static_cast<const LiteralBoolNode*>(node);
            out += " ";
            out += (boolLiteral->value ? "true" : "false");
            out += '\n';
            break;
        }

        case ASTNodeType::ARRAY_LITERAL: {
            const ArrayLiteralNode* arrayLiteral = static_cast<const ArrayLiteralNode*>(node);
            out += " [" + std::to_string(arrayLiteral->elements.size()) + " elements]\n";
            for (const auto& element : arrayLiteral->elements) {
                buildASTString(element.get(), indent + 1, out);
            }
            break;
        }

        case ASTNodeType::ARRAY_DECLARATION: {
            const ArrayDeclarationNode* arrayDecl = static_cast<const ArrayDeclarationNode*>(node);
            out += " '";
            out += arrayDecl->varName;
            out += "'";
            if (arrayDecl->hasType) {
                out += " type=";
                out += LexerEngine::tokenTypeToString(arrayDecl->elementType);
            }
            if (arrayDecl->hasSize) {
                out += " size=";
                out += std::to_string(arrayDecl->size);
            }
            out += '\n';
            if (arrayDecl->initializer) {
                buildASTString(arrayDecl->initializer.get(), indent + 1, out);
            }
            break;
        }

        default:
            out += '\n';
            break;
    }
}

std::unique_ptr<ArrayLiteralNode> ASTParser::parseArrayLiteral(Parser& parser) {
    TokenData* token = ParserEngine::currentToken(parser);
    int line = token->line, column = token->column;
    
    // Consume '['
    if (!ParserEngine::consumeToken(parser, ARRAY_OPEN)) {
        ParserEngine::parserError(parser, "Expected '[' for array literal");
        return nullptr;
    }
    
    auto arrayNode = std::make_unique<ArrayLiteralNode>(line, column);
    
    // Parse elements
    token = ParserEngine::currentToken(parser);
    if (token && token->type != ARRAY_CLOSE) {
        do {
            auto element = parseExpression(parser);
            if (!element) {
                ParserEngine::parserError(parser, "Expected array element");
                return nullptr;
            }
            arrayNode->elements.push_back(std::move(element));
            
            token = ParserEngine::currentToken(parser);
            if (token && token->type == COMMA) {
                ParserEngine::advanceParser(parser); // consume ','
            } else {
                break;
            }
        } while (true);
    }
    
    // Consume ']'
    if (!ParserEngine::consumeToken(parser, ARRAY_CLOSE)) {
        ParserEngine::parserError(parser, "Expected ']' after array elements");
        return nullptr;
    }
    
    return arrayNode;
}

std::unique_ptr<ArrayDeclarationNode> ASTParser::parseArrayDeclaration(Parser& parser) {
    TokenData* token = ParserEngine::currentToken(parser);
    int line = token->line, column = token->column;
    
    // Consume 'new'
    if (!ParserEngine::consumeToken(parser, NEW)) {
        ParserEngine::parserError(parser, "Expected 'new' keyword");
        return nullptr;
    }
    
    // Get variable name
    token = ParserEngine::currentToken(parser);
    if (!token || token->type != IDENTIFIER) {
        ParserEngine::parserError(parser, "Expected variable name after 'new'");
        return nullptr;
    }
    std::string varName = token->value;
    ParserEngine::advanceParser(parser);
    
    auto arrayDecl = std::make_unique<ArrayDeclarationNode>(varName, line, column);
    
    token = ParserEngine::currentToken(parser);
    
    // Case 1: new some_arr[] = [...]
    if (token && token->type == ARRAY_OPEN) {
        ParserEngine::advanceParser(parser); // consume '['
        
        if (!ParserEngine::consumeToken(parser, ARRAY_CLOSE)) {
            ParserEngine::parserError(parser, "Expected ']' after '['");
            return nullptr;
        }
        
        // Expect '='
        if (!ParserEngine::consumeToken(parser, ASSIGNMENT)) {
            ParserEngine::parserError(parser, "Expected '=' after array declaration");
            return nullptr;
        }
        
        // Parse array literal
        auto initializer = parseArrayLiteral(parser);
        if (!initializer) {
            ParserEngine::parserError(parser, "Expected array literal after '='");
            return nullptr;
        }
        
        arrayDecl->initializer = std::move(initializer);
    }
    // Case 2: new some_undeclared_array{int}[10]
    else if (token && token->type == TYPE_OPEN) {
        ParserEngine::advanceParser(parser); // consume '{'
        
        // Get type
        token = ParserEngine::currentToken(parser);
        if (!token || (token->type != STRING && token->type != INTEGER && 
                       token->type != FLOAT && token->type != BOOL)) {
            ParserEngine::parserError(parser, "Expected type inside {}");
            return nullptr;
        }
        arrayDecl->elementType = token->type;
        arrayDecl->hasType = true;
        ParserEngine::advanceParser(parser);
        
        // Consume '}'
        if (!ParserEngine::consumeToken(parser, TYPE_CLOSE)) {
            ParserEngine::parserError(parser, "Expected '}' after type");
            return nullptr;
        }
        
        // Expect '['
        if (!ParserEngine::consumeToken(parser, ARRAY_OPEN)) {
            ParserEngine::parserError(parser, "Expected '[' after type specification");
            return nullptr;
        }
        
        // Get size
        token = ParserEngine::currentToken(parser);
        if (!token || token->type != INTEGER) {
            ParserEngine::parserError(parser, "Expected array size");
            return nullptr;
        }
        arrayDecl->size = std::stoi(token->value);
        arrayDecl->hasSize = true;
        ParserEngine::advanceParser(parser);
        
        // Consume ']'
        if (!ParserEngine::consumeToken(parser, ARRAY_CLOSE)) {
            ParserEngine::parserError(parser, "Expected ']' after array size");
            return nullptr;
        }
    }
    else {
        ParserEngine::parserError(parser, "Expected array syntax after variable name");
        return nullptr;
    }
    
    return arrayDecl;
}
