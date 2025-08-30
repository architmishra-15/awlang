#include "ast.hpp"
#include "parser.hpp"
#include <fstream>
#include <iostream>

std::string read_file(const char* filename) {
  std::ifstream file(filename, std::ios::binary);

  return std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
}

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cout << "Please give a file name.\nUsage:\tcompiler.exe <filename>\n";
    return 1;
  }

  char* filename = argv[1];

  std::string content = read_file(filename);

  std::cout << "Compiling..." << filename << "\n";

  std::cout << "Tokenizing...\n";

  auto tokens = LexerEngine::tokenize(content);
  std::cout << "Generated " << tokens.size() << " tokens:\n";

  for (const auto &token : tokens) {
    if (token.type != COMMENT && token.type != END_OF_FILE) {
      std::cout << "  " << LexerEngine::tokenTypeToString(token.type) << " -> '"
                << token.value << "' (line " << token.line << ")\n";
    }
  }
  std::cout << "\n";

  std::cout << "Parsing...\n";

  Parser parser(tokens);

  std::cout << "Generating AST...\n";

  auto ast = ASTParser::parseProgram(parser);

  if (ast) {
    std::cout << "Generated AST:\n";
    ASTParser::printAST(ast.get());
  } else {
    std::cout << "Parsing failed!\n";
  }

  return 0;
}
