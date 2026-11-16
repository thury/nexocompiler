#include "compiler.h"

int main(int argc, char *argv[]) {
    std::vector<LexerToken> tokens = analizar_lexico(argc, argv);
    return 0;
}
