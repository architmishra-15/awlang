#include "parser.h"
#include <iostream>
#include <cctype>
#include <unordered_map>

// Implementation of Lexer
std::vector<TokenData> LexerEngine::tokenize(const std::string& source) {
  Lexer lexer(source);
  std::vector<TokenData> tokens;

  TokenData token;
  while (token.type != END_OF_FILE) {
    token = nextToken(lexer);
    tokens.push_back(token);
  }

  return tokens;
}
