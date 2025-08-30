#include <stdbool.h>
#include <string.h>
#define NOB_IMPLEMENTATION
#include "nob.h"
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#endif

#define BUILD "build"

int len(char* list[]) {
    int i = 0;
    while (list[i] != NULL) i++;
    return i;
}

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    Nob_Cmd cmd = {0};

    bool run = (argc > 2 && strcmp(argv[1], "run") == 0);

    if (!nob_mkdir_if_not_exists(BUILD)) {
        nob_log(NOB_INFO, "Created directory: %s", BUILD);
    }

    const char* output = BUILD "/main.exe";

    const char* filenames[] = {
      "src/main.cpp",
      "src/parser.cpp",
      "src/ast.cpp",
      // "src/lexer.cpp"
    };

    const char* flags[] = { NULL };

    if (len((char**)flags) > 0) {
        for (int i = 0; flags[i]; i++) {
            nob_cmd_append(&cmd, flags[i]);
        }
    }

    nob_cmd_append(&cmd, "g++", "-Wall", "-Wextra", "-std=c++20");

    for (int i = 0; i < (int)(sizeof(filenames)/sizeof(filenames[0])); i++) {
        nob_cmd_append(&cmd, filenames[i]);
    }

    nob_cmd_append(&cmd, "-o", output);

    if (!nob_cmd_run(&cmd)) {
        return 1;
    }

    return 0;
}

