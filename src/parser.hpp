#pragma once
#include <string>
#include <vector>

typedef enum Token {
    ASSIGNMENT,   // =
    EQUAL,        // ==
    NOT_EQUAL,    // !=
    GREATER,      // >
    LESSER,       // <
    GREATER_EQUAL, // >=
    LESSER_EQUAL,  // <=
    
    VARIABLE,
    INTEGER,
    STRING,
    BOOL,
    FLOAT,
    CHARACTER,
    
    STDOUT,
    NEW,          // new keyword
    BL,           // bl keyword
    TRUE_VAL,     // True/true/1
    FALSE_VAL,    // False/false/0
    
    IDENTIFIER,   // variable names
    
    LBRACKET,
    RBRACKET,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    
    COMMA,
    DOT,
    SEMICOLON,
    COLON,
    COMMENT,
    
    // Array syntax
    ARRAY_OPEN,   // [
    ARRAY_CLOSE,  // ]
    TYPE_OPEN,    // {
    TYPE_CLOSE,   // }
    
    END_OF_FILE,
    UNKNOWN,
} Token;

struct TokenData {
    Token type;
    std::string value;
    int line;
    int column;
    
    TokenData() : type(UNKNOWN), value(""), line(0), column(0) {}
    TokenData(Token t, const std::string& v, int l, int c) 
        : type(t), value(v), line(l), column(c) {}
};

struct Lexer {
    std::string source;
    size_t current;
    int line;
    int column;
    
    Lexer(const std::string& src) : source(src), current(0), line(1), column(1) {}
};

struct Parser {
    std::vector<TokenData> tokens;
    int token_count;
    int current;
    int line, col;
    
    Parser() : token_count(0), current(0), line(1), col(1) {}
    Parser(const std::vector<TokenData>& toks) 
        : tokens(toks), token_count(toks.size()), current(0), line(1), col(1) {}
};

// Lexer functions
class LexerEngine {
public:
    static std::vector<TokenData> tokenize(const std::string& source);
    static Token getKeywordToken(const std::string& word);
    static std::string tokenTypeToString(Token type);
    
private:
    static char peek(const Lexer& lexer);
    static char peekNext(const Lexer& lexer);
    static char advance(Lexer& lexer);
    static void skipWhitespace(Lexer& lexer);
    static TokenData readIdentifier(Lexer& lexer);
    static TokenData readNumber(Lexer& lexer);
    static TokenData readString(Lexer& lexer);
    static TokenData readStdoutContent(Lexer& lexer);
    static TokenData readComment(Lexer& lexer);
    static TokenData nextToken(Lexer& lexer);
};

// Parser functions
class ParserEngine {
public:
    static void initParser(Parser& parser, const std::vector<TokenData>& tokens);
    static TokenData* currentToken(Parser& parser);
    static TokenData* peekToken(Parser& parser);
    static void advanceParser(Parser& parser);
    static bool matchToken(Parser& parser, Token expected);
    static bool consumeToken(Parser& parser, Token expected);
    static void parserError(Parser& parser, const std::string& message);
};
