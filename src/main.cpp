#include <iostream>
#include "parser.hpp"

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cout << "Please give a file name.\nUsage:\tcompiler.exe <filename>\n";
    return 1;
  }

  std::string filename = argv[1];

  std::cout << "Compiling..." << filename << "\n";



  std::cout << "Hello World\n";
  return 0;
}
