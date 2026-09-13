#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring> // Necesario para strerror
#include <cerrno>  // Necesario para errno


std::unordered_set<std::string> palabrasClave = { "arranca", "muestra", "capturar",
	 "si", "entonces", "sino", "mientras", "hacer", "cierra" };

std::vector<std::vector<int>> matriz;

enum error_codes {
	ERROR_FORMATO_NUM_REAL = -1,
	ERROR_FORMATO_NUM_NOTACION_CIENTIFICA = -2,
	ERROR_EXPONENTE_NO_ENTERO = -3,
	ERROR_PUNTO_DESPUES_IDENTIFICADOR = -4,
	ERROR_SIGNO_DESPUES_IDENTIFICADOR = -5,
	ERROR_IDENTIFICADOR_NO_VALIDO = -6,
	ERROR_DEFINICION = -7
};

enum columna {
	DELIMITADOR = 0,
	DIGITO = 1,
	PUNTO = 2,
	SIGNO = 3,
	NOTACION_CIENTIFICA = 4,
	LETRA = 5,
	GUION_BAJO = 6,
	OPERADOR = 7,
	COMILLAS = 8,
	OP_IGUAL = 9,
	COMILLAS = 10,
	PARENTESIS_ABRIR = 11,
	PARENTESIS_CERRAR = 12,
	COMPARADOR_MAYOR = 13
};

struct Token {
	std::string tipo;
	std::string lexema;
	Token() = default;
	Token(const std::string &t, const std::string &l) : tipo(t), lexema(l) {}
};

void construir_matriz_transicion() {
    std::vector<std::vector<int>> matriztmp = {
        // 0(esp) 1(dig) 2(.)  3(+-) 4(Ee) 5(let) 6(_)  7(op) 8(=)  9(")  10( ( ) 11( ) ) 12( > )
        {  0,     1,    -5,   500,  7,    7,    -5,   500,  8,    9,    900,    1000,   1100}, // 0: Inicio
        {100,     1,     2,   100,  4,    -1,   -1,   -1,   -1,   -1,   100,    100,    100 }, // 1: Entero
        { -2,     3,    -2,   -2,  -2,    -2,   -2,   -2,   -2,   -1,    -2,     -2,     -2 }, // 2: Punto decimal
        {200,     3,    -2,   200,  4,    -2,   -2,   -1,   -2,  200,   200,    200,    200 }, // 3: Real
        { -3,     6,    -3,     5, -3,    -3,   -3,   -1,   -2,   -2,    -3,     -3,     -3 }, // 4: 'e' / 'E'
        { -3,     6,    -3,    -3, -3,    -3,   -3,   -1,   -1,   -2,    -3,     -3,     -3 }, // 5: Signo exp
        {300,     6,    -3,   300, -3,    -3,   -3,   -1,   -1,  300,   300,    300,    300 }, // 6: Cientifico
        {400,     7,   400,   400,  7,     7,    7,  400,  400,  400,   400,    400,    400 }, // 7: ID 
        {600,   600,   600,   600, 600,  600,  600,  600,  800,  600,   600,    600,    600 }, // 8: Asignacion (=) o Igualdad (==)
        {  9,     9,     9,     9,  9,     9,    9,    9,    9,  700,     9,      9,      9 }  // 9: Cadena ("...")
    };

    matriz = matriztmp;
}

int obtener_columna(char c) {
    switch (c) {
        case ' ': return DELIMITADOR; // delimitadores
        case '\t': return DELIMITADOR;
        case '\n': return DELIMITADOR;
        case '.': return PUNTO; // punto
        case '+': return SIGNO; // signos
        case '-': return SIGNO;
        case 'E': return NOTACION_CIENTIFICA; // notacion cientifica
        case 'e': return NOTACION_CIENTIFICA;
        case '_': return GUION_BAJO; // guion bajo
        case '*': return OPERADOR; // operadores
        case '/': return OPERADOR;
        case '%': return OPERADOR;
        case '=': return OP_IGUAL; // asignacion
        case '"': return COMILLAS; // comillas
        case '(': return PARENTESIS_ABRIR; // parentesis abre
        case ')': return PARENTESIS_CERRAR; // parentesis cierra
        case '>': return COMPARADOR_MAYOR; // comparador
        default:
            if (std::isdigit(static_cast<unsigned char>(c))) 
                return 1;
            if (std::isalpha(static_cast<unsigned char>(c))) 
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
        case 600: return "Asignacion";
        case 700: return "CadenaCaracteres";
        case 800: return "Igualdad";
        case 900: return "ParentesisAbre";
        case 1000: return "ParentesisCierra";
        case 1100: return "ComparadorMayor";
        default: return "Token";
    }
}
std::string obtener_tipo_lexema(int q, const std::string &lexema) {
	if (q == 400) {
		auto lexema_lower = lexema;
		if (palabrasClave.find(lexema) != palabrasClave.end()) {
			std::transform(lexema_lower.begin(), lexema_lower.end(), lexema_lower.begin(), [](unsigned char c) {
				return std::toupper(c);
			});
			return "KW_" + lexema_lower; // Palabra clave
		}
	}
	return obtener_tipo_token(q);
}

void error_lexico(const std::string &lexema, int error_code = -1) {
	std::cout << "Error lexico en: " << lexema << std::endl;
	switch (error_code) {
		case ERROR_FORMATO_NUM_REAL:
			std::cout << "Error de formato en el numero real o notacion cientifica" << std::endl;
			break;
		case ERROR_FORMATO_NUM_NOTACION_CIENTIFICA:
			std::cout << "Error de formato en la notacion cientifica" << std::endl;
			break;
		case ERROR_EXPONENTE_NO_ENTERO:
			std::cout << "El exponente debe ser un numero entero" << std::endl;
			break;
		case ERROR_PUNTO_DESPUES_IDENTIFICADOR:
			std::cout << "No se permiten puntos despues de un identificador" << std::endl;
			break;
		case ERROR_SIGNO_DESPUES_IDENTIFICADOR:
			std::cout << "No se permiten signos despues de un identificador" << std::endl;
			break;
		case ERROR_IDENTIFICADOR_NO_VALIDO:
			std::cout << "Identificador no valido" << std::endl;
			break;
		case ERROR_DEFINICION:
			std::cout << "Error de definicion" << std::endl;
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

	size_t i = 0;
	while (i < cadena.size()) {
		int col = obtener_columna(cadena[i]);

		if (col == -1) { // Carácter escaneado no reconocido
			int estado_final = matriz[q][DELIMITADOR];
			if (estado_final != 0 && estado_final % 100 == 0) {
				tokens.emplace_back(obtener_tipo_lexema(estado_final, lexema_actual), lexema_actual);
				q = 0;
				lexema_actual.clear();
				continue;
			}
			std::string s(1, cadena[i]);
			error_lexico(s, -100);
			i++;
			continue;
		}

		int nuevo_estado = matriz[q][col];

		if (nuevo_estado < 0) { // Error en la transición
			int estado_final = matriz[q][DELIMITADOR];
			if (estado_final != 0 && estado_final % 100 == 0) {
				tokens.emplace_back(obtener_tipo_lexema(estado_final, lexema_actual), lexema_actual);
				q = 0;
				lexema_actual.clear();
				continue;
			}
			if (!lexema_actual.empty()) {
				std::string l = lexema_actual + cadena[i];
				error_lexico(l, nuevo_estado);
			}
			q = 0;
			lexema_actual.clear();
			i++;
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

		++i;
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

std::string leer_archivo(const std::string &nombre_archivo) throw(std::ios_base::failure) {
	std::ifstream archivo(nombre_archivo);
	if (!archivo.is_open()) {
		std::cerr << "No se pudo abrir el archivo: " << nombre_archivo << std::endl << "Error: " << std::strerror(errno) << std::endl;
		throw std::ios_base::failure("No se pudo abrir el archivo");
	}
	std::stringstream buffer;
	buffer << archivo.rdbuf();
	return buffer.str();
}

int main(int argc, char *argv[]) {
	construir_matriz_transicion();
	std::string cadena;
	std::string nombre_archivo = argc > 1 ? argv[1] : "ejemplo.nexo";

	cadena = leer_archivo(nombre_archivo);
	
	cadena = trim(cadena) + " "; // Agrega un espacio al final para procesar el último token

	std::vector<Token> tokens = busca_simbolos(matriz, cadena);

	for (const auto &t : tokens) {
		std::cout << t.lexema << " -> " << t.tipo << std::endl;
	}

	return 0;
}
