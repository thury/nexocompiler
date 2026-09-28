#include "compiler.h"
#include "analizador_lexico.cpp"

int main(int argc, char *argv[]) {
    std::vector<Token> tokens = analizar_lexico(argc, argv);
    return 0;
}