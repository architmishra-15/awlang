#pragma once
#include <cstddef>
#include <string>
#include <vector>

typedef enum Token {
  ASSIGNMENT,    // =
  EQUAL,         // ==
  NOT_EQUAL,     // !=
  GREATER,       // >
  LESSER,        // <
  GREATER_EQUAL, // >=
  LESSER_EQUAL,  // <=

  // DataTypes
  VARIABLE,
  INTEGER,
  STRING,
  BOOL,
  FLOAT,
  CHARACTER,

  // Keywords
  STDOUT,
  NEW,
  TRUE,
  FALSE,

  // String interpolation
  INTERPOLATION_START, // {
  INTERPOLATION_END,   // }

  // Special stdout syntax
  STDOUT_START, // [
  STDOUT_END,   // ]

  // Identifiers and literals
  IDENTIFIER, // variable names like 'name', 'age'

  // Symbols
  LBACKET,
  RBRACKET,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  COMMA,
  DOT,
  SEMICOLON,
  COLON,
  COMMENT,

  // Arithematic Operators
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,

  END_OF_FILE,
  UNKNOWN,
} Token;

// Token data structure
typedef struct TokenData{
  Token type;
  char *value; // The actual text (e.g., "123", "hello", "myVar")
  int line;
  int column;

  TokenData() : type(UNKNOWN), value(nullptr), line(0), column(0) {}
  TokenData(Token t, char *v, int l, int c) : type(t), value(v), line(l), column(c) {}
} TokenData;

// Lexer structure
typedef struct Lexer {
  std::string filename;
  size_t current;
  int line;
  int column;

  Lexer(const std::string& src) : filename(src), current(0), line(1), column(1) {}
} Lexer;

// Parser structure
typedef struct Parser {
  std::vector<TokenData> tokens;
  int token_count;
  int current;
  int line, col;

  Parser() : token_count(0), current(0), line(1), col(1) {}
  Parser(const std::vector<TokenData> &tokens) : tokens(tokens), token_count(tokens.size()), current(0), line(1), col(1) {}
} Parser;


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
}; // Lexer

class ParserEngine {
  public:
    static void initParser(Parser& parser, const std::vector<TokenData>& tokens);
    static TokenData* currentToken(Parser& parser);
    static TokenData* peekToken(Parser& parser);
    static void advanceParser(Parser& parser);
    static bool matchToken(Parser& parser, Token expected);
    static bool consumeToken(Parser& parser, Token expected);
    static void parserError(Parser& parser, const std::string& message);
}; // Parser
