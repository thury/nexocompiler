#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <iomanip> // Necesario para dar formato a la tabla (std::setw, std::left)

const std::unordered_set<std::string> palabrasClave = { 
    "arranca", "cadena", "capturar", "carac", "cierra", 
    "dec", "entero", "entonces", "escribir", "fin", 
    "hacer", "hasta", "inc", "inicio", "leer", 
    "mientras", "muestra", "real", "si", "sino", "ver" 
};

struct LexerToken {
	std::string tipo;
	std::string lexema;
	int linea;
	LexerToken() = default;
	LexerToken(const std::string &t, const std::string &l, int n) : tipo(t), lexema(l), linea(n) {}
};

std::vector<LexerToken> analizar_lexico(int argc, char *argv[]);
