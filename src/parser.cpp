#include "parser.hpp"
#include <iostream>
#include <cctype>
#include <unordered_map>

// Lexer Implementation
std::vector<TokenData> LexerEngine::tokenize(const std::string& source) {
    Lexer lexer(source);
    std::vector<TokenData> tokens;
    
    TokenData token;
    do {
        token = nextToken(lexer);
        tokens.push_back(token);
    } while (token.type != END_OF_FILE);
    
    return tokens;
}

Token LexerEngine::getKeywordToken(const std::string& word) {
    static std::unordered_map<std::string, Token> keywords = {
        {"new", NEW},
        {"bl", BL},
        {"stdout", STDOUT},
        {"string", STRING},
        {"int", INTEGER},
        {"float", FLOAT},
        {"bool", BOOL},
        {"char", CHARACTER},
        {"True", TRUE_VAL},
        {"true", TRUE_VAL},
        {"False", FALSE_VAL},
        {"false", FALSE_VAL}
    };
    
    auto it = keywords.find(word);
    return (it != keywords.end()) ? it->second : IDENTIFIER;
}

std::string LexerEngine::tokenTypeToString(Token type) {
    switch (type) {
        case NEW: return "NEW";
        case BL: return "BL";
        case STDOUT: return "STDOUT";
        case IDENTIFIER: return "IDENTIFIER";
        case INTEGER: return "INTEGER";
        case FLOAT: return "FLOAT";
        case STRING: return "STRING";
        case BOOL: return "BOOL";
        case TRUE_VAL: return "TRUE";
        case FALSE_VAL: return "FALSE";
        case ASSIGNMENT: return "ASSIGNMENT";
        case EQUAL: return "EQUAL";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case STDOPEN: return "STDOPEN";
        case STDCLOSE: return "STDCLOSE";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case COMMENT: return "COMMENT";
        case END_OF_FILE: return "EOF";
        case UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN_TOKEN";
    }
}

char LexerEngine::peek(const Lexer& lexer) {
    if (lexer.current >= lexer.source.length()) return '\0';
    return lexer.source[lexer.current];
}

char LexerEngine::peekNext(const Lexer& lexer) {
    if (lexer.current + 1 >= lexer.source.length()) return '\0';
    return lexer.source[lexer.current + 1];
}

char LexerEngine::advance(Lexer& lexer) {
    if (lexer.current >= lexer.source.length()) return '\0';
    
    char c = lexer.source[lexer.current++];
    if (c == '\n') {
        lexer.line++;
        lexer.column = 1;
    } else {
        lexer.column++;
    }
    return c;
}

void LexerEngine::skipWhitespace(Lexer& lexer) {
    while (std::isspace(peek(lexer))) {
        advance(lexer);
    }
}

TokenData LexerEngine::readIdentifier(Lexer& lexer) {
    size_t start = lexer.current;
    int line = lexer.line;
    int column = lexer.column;
    
    while (std::isalnum(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }
    
    std::string value = lexer.source.substr(start, lexer.current - start);
    Token type = getKeywordToken(value);
    
    return TokenData(type, value, line, column);
}

TokenData LexerEngine::readNumber(Lexer& lexer) {
    size_t start = lexer.current;
    int line = lexer.line;
    int column = lexer.column;
    bool isFloat = false;
    
    while (std::isdigit(peek(lexer))) {
        advance(lexer);
    }
    
    // Check for decimal point
    if (peek(lexer) == '.' && std::isdigit(peekNext(lexer))) {
        isFloat = true;
        advance(lexer); // consume '.'
        while (std::isdigit(peek(lexer))) {
            advance(lexer);
        }
    }
    
    std::string value = lexer.source.substr(start, lexer.current - start);
    Token type = isFloat ? FLOAT : INTEGER;
    
    return TokenData(type, value, line, column);
}

TokenData LexerEngine::readString(Lexer& lexer) {
    int line = lexer.line;
    int column = lexer.column;
    
    advance(lexer); // consume opening quote
    
    std::string value;
    while (peek(lexer) != '"' && peek(lexer) != '\0') {
        value += advance(lexer);
    }
    
    if (peek(lexer) == '"') {
        advance(lexer); // consume closing quote
    } else {
        g_errorHandler.addLexicalError("Unterminated string literal", line, column, 
                                      "Add closing quote '\"' to end the string");
    }
    
    return TokenData(STRING, value, line, column);
}

TokenData LexerEngine::readStdoutContent(Lexer& lexer) {
    int line = lexer.line;
    int column = lexer.column;
    
    
    advance(lexer); // consume '['
    
    std::string value;
    while (peek(lexer) != ']' && peek(lexer) != '\0') {
        value += advance(lexer);
    }
    
    if (peek(lexer) == ']') {
        advance(lexer); // consume ']'
    }
    
    return TokenData(STRING, value, line, column);
}

TokenData LexerEngine::readComment(Lexer& lexer) {
    int startLine = lexer.line;
    int startCol  = lexer.column;
    std::string value;

    // // single-line comment
    if (peek(lexer) == '/' && peekNext(lexer) == '/') {
        advance(lexer); // '/'
        advance(lexer); // '/'
        while (peek(lexer) != '\0' && peek(lexer) != '\n' && peek(lexer) != '\r') {
            value += advance(lexer);
        }
        return TokenData(COMMENT, value, startLine, startCol);
    }

    // ; comments
    if (peek(lexer) == ';') {
        // ;; single-line comment
        if (peekNext(lexer) == ';') {
            advance(lexer); // ';'
            advance(lexer); // ';'
            while (peek(lexer) != '\0' && peek(lexer) != '\n' && peek(lexer) != '\r') {
                value += advance(lexer);
            }
            return TokenData(COMMENT, value, startLine, startCol);
        }

        // ; ... ; multi-line comment (closes at next ';')
        advance(lexer); // consume opening ';'
        while (peek(lexer) != '\0') {
            if (peek(lexer) == ';') {
                advance(lexer); // consume closing ';'
                return TokenData(COMMENT, value, startLine, startCol);
            }
            value += advance(lexer);
        }

        // EOF reached without closing ';' — still return what we collected
        // optionally emit a warning about unterminated comment
        return TokenData(COMMENT, value, startLine, startCol);
    }

    // Shouldn't get here — fallback
    return TokenData(COMMENT, std::string(), startLine, startCol);
}


TokenData LexerEngine::nextToken(Lexer& lexer) {
    skipWhitespace(lexer);
    
    if (lexer.current >= lexer.source.length()) {
        return TokenData(END_OF_FILE, "EOF", lexer.line, lexer.column);
    }
    
    char c = peek(lexer);
    int line = lexer.line;
    int column = lexer.column;
    
    // Handle comments
    if (c == '/' && peekNext(lexer) == '/') {
        return readComment(lexer);
    }
    if (c == ';') {
        return readComment(lexer);
    }
    
    // Handle string literals
    if (c == '"') {
        return readString(lexer);
    }
    
    // Handle identifiers and keywords first to detect 'stdout'
    if (std::isalpha(c) || c == '_') {
        TokenData token = readIdentifier(lexer);
        // If we just parsed 'stdout', the next '[' should be stdout content
        return token;
    }
    
    // Handle numbers
    if (std::isdigit(c)) {
        return readNumber(lexer);
    }
    
    // Handle operators and symbols
    switch (c) {
        case '=':
            advance(lexer);
            if (peek(lexer) == '=') {
                advance(lexer);
                return TokenData(EQUAL, "==", line, column);
            } else {
                return TokenData(ASSIGNMENT, "=", line, column);
            }
        case '!':
            advance(lexer);
            if (peek(lexer) == '=') {
                advance(lexer);
                return TokenData(NOT_EQUAL, "!=", line, column);
            } else {
                return TokenData(UNKNOWN, "!", line, column);
            }
        case '>':
            advance(lexer);
            if (peek(lexer) == '=') {
                advance(lexer);
                return TokenData(GREATER_EQUAL, ">=", line, column);
            } else {
                return TokenData(GREATER, ">", line, column);
            }
        case '<':
            advance(lexer);
            if (peek(lexer) == '=') {
                advance(lexer);
                return TokenData(LESSER_EQUAL, "<=", line, column);
            } else {
                return TokenData(LESSER, "<", line, column);
            }
        case '+':
            advance(lexer);
            return TokenData(ADD, "+", line, column);
        case '-':
            advance(lexer);
            return TokenData(SUB, "-", line, column);
        case '*':
            advance(lexer);
            return TokenData(MUL, "*", line, column);
        case '/':
            advance(lexer);
            return TokenData(DIV, "/", line, column);
        case '%':
            advance(lexer);
            return TokenData(MOD, "%", line, column);
        case '(':
            advance(lexer);
            return TokenData(LPAREN, "(", line, column);
        case ')':
            advance(lexer);
            return TokenData(RPAREN, ")", line, column);
        case '{':
            advance(lexer);
            return TokenData(TYPE_OPEN, "{", line, column);
        case '}':
            advance(lexer);
            return TokenData(TYPE_CLOSE, "}", line, column);
        case '[':
            // Check if this is array literal or stdout content
            // For now, assume it's array literal if not after stdout
            advance(lexer);
            return TokenData(ARRAY_OPEN, "[", line, column);
        case ']':
            advance(lexer);
            return TokenData(ARRAY_CLOSE, "]", line, column);
        case ',':
            advance(lexer);
            return TokenData(COMMA, ",", line, column);
        case '.':
            advance(lexer);
            return TokenData(DOT, ".", line, column);
        case ':':
            advance(lexer);
            return TokenData(COLON, ":", line, column);
        default:
            std::string suggestion = "Remove this character or check if it's part of a valid token";
            if (c == '@' || c == '#' || c == '$') {
                suggestion = "This character is not valid in this language";
            }
            g_errorHandler.addLexicalError("Unexpected character '" + std::string(1, c) + "'", 
                                          line, column, suggestion);
            advance(lexer);
            return TokenData(UNKNOWN, std::string(1, c), line, column);
    }
}

// Parser Implementation
void ParserEngine::initParser(Parser& parser, const std::vector<TokenData>& tokens) {
    parser.tokens = tokens;
    parser.token_count = tokens.size();
    parser.current = 0;
    if (!tokens.empty()) {
        parser.line = tokens[0].line;
        parser.col = tokens[0].column;
    }
}

TokenData* ParserEngine::currentToken(Parser& parser) {
    if (parser.current >= parser.token_count) {
        return nullptr;
    }
    return &parser.tokens[parser.current];
}

TokenData* ParserEngine::peekToken(Parser& parser) {
    if (parser.current + 1 >= parser.token_count) {
        return nullptr;
    }
    return &parser.tokens[parser.current + 1];
}

void ParserEngine::advanceParser(Parser& parser) {
    if (parser.current < parser.token_count) {
        parser.current++;
        if (parser.current < parser.token_count) {
            parser.line = parser.tokens[parser.current].line;
            parser.col = parser.tokens[parser.current].column;
        }
    }
}

bool ParserEngine::matchToken(Parser& parser, Token expected) {
    TokenData* token = currentToken(parser);
    return token && token->type == expected;
}

bool ParserEngine::consumeToken(Parser& parser, Token expected) {
    if (matchToken(parser, expected)) {
        advanceParser(parser);
        return true;
    }
    return false;
}

void ParserEngine::parserError(Parser& parser, const std::string& message) {
    TokenData* token = currentToken(parser);
    if (token) {
        std::string fullMessage = message + " (found '" + token->value + "')";
        std::string suggestion = getSuggestionForToken(token->type, message);
        int endCol = token->column + (int)token->value.length() - 1;
        g_errorHandler.addSyntaxError(fullMessage, token->line, token->column, suggestion, endCol);
    } else {
        g_errorHandler.addSyntaxError(message + " (at end of input)", parser.line, parser.col);
    }
}

std::string ParserEngine::getSuggestionForToken(Token tokenType, const std::string& message) {
    if (message.find("Expected 'new'") != std::string::npos) {
        return "Variable declarations must start with 'new' keyword";
    }
    if (message.find("Expected variable name") != std::string::npos) {
        return "Provide a valid identifier after 'new'";
    }
    if (message.find("Expected type") != std::string::npos) {
        return "Specify a type: 'string', 'int', 'float', or 'bool'";
    }
    if (message.find("Expected '='") != std::string::npos) {
        return "Add '=' to assign a value to the variable";
    }
    if (message.find("Expected value") != std::string::npos) {
        return "Provide a value after '='";
    }
    if (message.find("Expected '['") != std::string::npos) {
        return "stdout statements require '[' to start the output content";
    }
    if (message.find("Expected ']'") != std::string::npos) {
        return "Close the stdout statement with ']'";
    }
    if (message.find("Expected '}'") != std::string::npos) {
        return "Close the variable interpolation with '}'";
    }
    return "";
}
