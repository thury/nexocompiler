#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

std::unordered_set<std::string> palabrasClave = { "inicio", "escribir", "capturar", "si", "entonces", "sino", "mientras", "hacer", "fin" };
std::unordered_map<std::string, std::string> tiposPalabrasClave = {
	{ "inicio", "KW_INICIO" },
	{ "escribir", "KW_ESCRIBIR" },
	{ "capturar", "KW_CAPTURAR" },
	{ "si", "KW_SI" },
	{ "entonces", "KW_ENTONCES" },
	{ "sino", "KW_SINO" },
	{ "mientras", "KW_MIENTRAS" },
	{ "hacer", "KW_HACER" },
	{ "fin", "KW_FIN" }
};

std::vector<std::vector<int>> matriz;

void construir_matriz_transicion() {
	std::vector<std::vector<int>> matriztmp = {
	    {  0,   1,  -5, 500,   7,   7,  -5, 500}, // estado 0
		{100,   1,   2, 100,   4,  -1,  -1,  -1}, // estado 1
		{ -2,   3,  -2,  -2,  -2,  -2,  -2,  -1}, // estado 2
		{200,   3,  -2, 200,   4,  -2,  -2,  -1}, // estado 3
		{ -3,   6,  -3,   5,  -3,  -3,  -3,  -1}, // estado 4
		{ -3,   6,  -3,  -3,  -3,  -3,  -3,  -1}, // estado 5
		{300,   6,  -3, 300,  -3,  -3,  -3,  -1}, // estado 6
		{400,   7,  -4,  -5,   7,   7,   7,  -1},  // estado 7
	};

	matriz = matriztmp;
}

struct Token {
	std::string tipo;
	std::string lexema;
	Token() = default;
	Token(const std::string &t, const std::string &l) : tipo(t), lexema(l) {}
};

int obtener_columna(char c) {
	switch (c) {
		case ' ': return 0; // delimitadores
		case '\t': return 0;
		case '\n': return 0;
		case '.': return 2; // punto
		case '+': return 3; // signos
		case '-': return 3;
		case 'E': return 4; // notacion cientifica
		case 'e': return 4;
		case '_': return 6; // guion bajo
		case '*': return 7; // operadores
		case '/': return 7;
		case '%': return 7;
		default:
			if (std::isdigit(static_cast<unsigned char>(c))) // digitos
				return 1;
			if (std::isalpha(static_cast<unsigned char>(c))) // letras
				return 5; 
			return -1; // caracter no reconocido
	}
	return -1;
}

std::string obtener_tipo_token(int q) {
	switch (q) {
		case 100: return "NumEntero";
		case 200: return "NumReal";
		case 300: return "NumNotacionCientifica";
		case 400: return "Id";
		case 500: return "OperadorAritmetico";
		default: return "Token";
	}
}

std::string obtener_tipo_lexema(int q, const std::string &lexema) {
	if (q == 400) {
		auto palabraClave = tiposPalabrasClave.find(lexema);
		if (palabraClave != tiposPalabrasClave.end())
			return palabraClave->second;
	}
	return obtener_tipo_token(q);
}

void error_lexico(const std::string &lexema, int error_code = -1) {
	std::cout << "Error lexico en: " << lexema << std::endl;
	switch (error_code) {
		case -1:
			std::cout << "Error de formato en el numero real o notacion cientifica" << std::endl;
			break;
		case -2:
			std::cout << "Error de formato en el numero real" << std::endl;
			break;
		case -3:
			std::cout << "El exponente debe ser un numero entero" << std::endl;
			break;
		case -4:
			std::cout << "No se permiten puntos despues de un identificador" << std::endl;
			break;
		case -5:
			std::cout << "No se permiten signos despues de un identificador" << std::endl;
			break;
		case -6:
			std::cout << "Identificador no valido" << std::endl;
			break;
		default:
			std::cout << "Error desconocido" << std::endl;
			break;
	}
}

std::vector<Token> busca_simbolos(const std::vector<std::vector<int>> &matriz, const std::string &cadena) {
	std::vector<Token> tokens;
	int q = 0;
	std::string lexema_actual;

	for (size_t i = 0; i < cadena.size(); ++i) {
		int col = obtener_columna(cadena[i]);

		if (col == -1) {
			std::string s(1, cadena[i]);
			error_lexico(s, -1);
			continue;
		}

		int nuevo_estado = matriz[q][col];

		if (nuevo_estado < 0) {
			if (!lexema_actual.empty()) {
				std::string l = lexema_actual + cadena[i];
				error_lexico(l, nuevo_estado);
			}
			q = 0;
			lexema_actual.clear();
			continue;
		}

		q = nuevo_estado;

		if (col != 0)
			lexema_actual.push_back(cadena[i]);

		if (q % 100 == 0 && q != 0) {
			tokens.emplace_back(obtener_tipo_lexema(q, lexema_actual), lexema_actual);
			q = 0;
			lexema_actual.clear();
		}
	}

	return tokens;
}

static inline std::string trim(const std::string &s) {
	size_t start = 0;
	while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
	size_t end = s.size();
	while (end > start && std::isspace(static_cast<unsigned char>(s[end-1]))) --end;
	return s.substr(start, end - start);
}

int main() {
	construir_matriz_transicion();
	std::string cadena;
	std::cout << "Introduce una linea: ";
	if (!std::getline(std::cin, cadena)) return 0;

	cadena = trim(cadena) + " "; // Agrega un espacio al final para procesar el último token

	std::vector<Token> tokens = busca_simbolos(matriz, cadena);

	for (const auto &t : tokens) {
		std::cout << t.lexema << " -> " << t.tipo << std::endl;
	}

	return 0;
}
