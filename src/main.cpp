#include "ast.hpp"
#include "parser.hpp"
#include "error.hpp"
#include <fstream>
#include <iostream>

std::string read_file(const char* filename) {
  std::ifstream file(filename, std::ios::binary);

  return std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
}

int writeTokenToFile(const char* filename, std::vector<TokenData>& tokens ) {
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file for writing: " << filename << "\n";
    return 1;
  }

  for (const auto& token : tokens) {
    file << LexerEngine::tokenTypeToString(token.type) << " " << token.value << "\n";
  }

  file.close();
  return 0;
}

int writeASTToFile(const char* filename, std::string ast_rep){
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file for writing: " << filename << "\n";
    return 1;
  }

  file << ast_rep;

  file.close();
  return 0;
}

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cout << "Please give a file name.\nUsage:\tcompiler.exe <filename>\n";
    return 1;
  }

  char* output_file_parser = "output.lexerIR";
  char* output_file_ast = "output.astIR";

  char* filename = argv[1];

  std::string content = read_file(filename);

  std::cout << "Compiling " << filename << "...\n";

  // Clear any previous errors and set source content for context
  g_errorHandler.clear();
  g_errorHandler.setSourceContent(content, filename);

  std::cout << "Tokenizing...\n";
  auto tokens = LexerEngine::tokenize(content);

  std::cout << "Parsing...\n";
  Parser parser(tokens);

  std::cout << "Generating AST...\n";
  auto ast = ASTParser::parseProgram(parser);

  // Check for any errors (lexical or syntax)
  if (g_errorHandler.hasAnyErrors()) {
    g_errorHandler.printErrors();
    return 1;
  }

  if (ast) {
    std::cout << "\033[32m\033[1m✓ Compilation successful!\033[0m" << std::endl;
    
    // Only write output files if compilation was successful
    int success = writeTokenToFile(output_file_parser, tokens);
    if (success == 0) {
      std::cout << "\033[34m  → Lexer IR written to " << output_file_parser << "\033[0m" << std::endl;
    } else {
      std::cerr << "\033[31mFailed to write lexer data\033[0m" << std::endl;
    }

    success = writeASTToFile(output_file_ast, ASTParser::astToString(ast.get(), 0));
    if (success == 0) {
      std::cout << "\033[34m  → AST IR written to " << output_file_ast << "\033[0m" << std::endl;
    } else {
      std::cerr << "\033[31mFailed to write AST data\033[0m" << std::endl;
    }
  } else {
    std::cerr << "\033[31m\033[1m✗ AST generation failed!\033[0m" << std::endl;
    return 1;
  }

  return 0;
}
