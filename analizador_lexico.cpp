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
	OP_IGUAL = 8,
	COMILLAS = 9,
	PARENTESIS_ABRIR = 10,
	PARENTESIS_CERRAR = 11,
	COMPARADOR_MAYOR = 12
};

struct Token {
	std::string tipo;
	std::string lexema;
	int linea;
	Token() = default;
	Token(const std::string &t, const std::string &l, int n) : tipo(t), lexema(l), linea(n) {}
};

void construir_matriz_transicion() {
    std::vector<std::vector<int>> matriztmp = {
        // 0(esp) 1(dig) 2(.)  3(+-) 4(Ee) 5(let) 6(_)  7(op) 8(=)  9(")  10( ( ) 11( ) ) 12( > )
        {  0,     1,    -5,   500,  7,    7,    -5,   500,  8,    9,    900,   1000,    1100}, // 0: Inicio
        {100,     1,     2,    -3,  4,    -1,   -1,   -1,   -1,   -1,    -1,     -1,     -6 }, // 1: Entero
        { -2,     3,    -2,    -2, -2,    -2,   -2,   -2,   -2,   -1,    -2,     -2,     -2 }, // 2: Punto decimal
        {200,     3,    -2,    -1,  4,    -2,   -2,   -1,   -2,   -1,    -1,     -1,     -1 }, // 3: Real
        { -3,     6,    -3,     5, -3,    -3,   -3,   -1,   -2,   -2,    -3,     -3,     -3 }, // 4: 'e' / 'E'
        { -3,     6,    -3,    -3, -3,    -3,   -3,   -1,   -1,   -2,    -3,     -3,     -3 }, // 5: Signo exp
        {300,     6,    -3,    -4, -4,    -4,   -4,   -4,   -4,   -4,    -4,     -4,     -4 }, // 6: Cientifico
        {400,     7,    -4,    -6,  7,     7,    7,   -6,   -6,   -6,    -6,     -6,     -6 }, // 7: ID 
        {600,    -1,    -1,    -1, -1,    -1,   -1,   -1,  800,   -1,    -1,     -6,     -6 }, // 8: Asignacion (=) o Igualdad (==)
        {  9,     9,     9,     9,  9,     9,    9,    9,    9,  700,     9,      9,      9 }  // 9: Cadena ("...")
    };

    matriz = matriztmp;
}

int obtener_columna(char c) {
    switch (c) {
        case ' ': return DELIMITADOR;
        case '\t': return DELIMITADOR;
        case '\n': return DELIMITADOR;
        case '.': return PUNTO;
        case '+': return SIGNO;
        case '-': return SIGNO;
        case 'E': return NOTACION_CIENTIFICA;
        case 'e': return NOTACION_CIENTIFICA;
        case '_': return GUION_BAJO;
        case '*': return OPERADOR;
        case '/': return OPERADOR;
        case '%': return OPERADOR;
        case '=': return OP_IGUAL;
        case '"': return COMILLAS;
        case '(': return PARENTESIS_ABRIR;
        case ')': return PARENTESIS_CERRAR;
        case '>': return COMPARADOR_MAYOR;
        default:
            if (std::isdigit(static_cast<unsigned char>(c))) 
                return DIGITO;
            if (std::isalpha(static_cast<unsigned char>(c))) 
                return LETRA;
            return -1;
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
		if (palabrasClave.find(lexema) == palabrasClave.end()) {
			return "Id";
		}
		//Es una keyword
		std::transform(lexema_lower.begin(), lexema_lower.end(), lexema_lower.begin(), [](unsigned char c) {
			return std::toupper(c);
		});
		return "KW_" + lexema_lower;
	}
	return obtener_tipo_token(q);
}

void error_lexico(const std::string &lexema, int error_code, int linea) {
	std::cout << "[ERROR LEXICO] En linea " << linea << " | Token: \"" << lexema << "\" -> ";
	switch (error_code) {
		case ERROR_FORMATO_NUM_REAL:
			std::cout << "Error de formato en el numero real" << std::endl;
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
			std::cout << "Caracter no reconocido o invalido" << std::endl;
			break;
	}
}

std::vector<Token> busca_simbolos(const std::vector<std::vector<int>> &matriz, const std::string &cadena) {
	std::vector<Token> tokens;
	int q = 0;
	std::string lexema_actual;
	int linea_actual = 1;

	size_t i = 0;
	while (i < cadena.size()) {
		char c = cadena[i];

		// Omite los caracteres de salto de línea o retorno de carro sin reportar error léxico
		if (c == '\n' || c == '\r') {
			if (c == '\n') {
				linea_actual++;
			}
			if (!lexema_actual.empty()) {
				int estado_final = matriz[q][DELIMITADOR];
				if (estado_final != 0 && estado_final % 100 == 0) {
					tokens.emplace_back(obtener_tipo_lexema(estado_final, lexema_actual), lexema_actual, linea_actual - 1);
				}
				q = 0;
				lexema_actual.clear();
			}
			i++;
			continue;
		}

		int col = obtener_columna(c);

		if (col == -1) {
			int estado_final = matriz[q][DELIMITADOR];
			if (estado_final != 0 && estado_final % 100 == 0) {
				tokens.emplace_back(obtener_tipo_lexema(estado_final, lexema_actual), lexema_actual, linea_actual);
				q = 0;
				lexema_actual.clear();
				continue;
			}
			std::string s(1, c);
			error_lexico(s, -100, linea_actual);
			i++;
			continue;
		}

		int nuevo_estado = matriz[q][col];

		if (nuevo_estado < 0) {
			int estado_final = matriz[q][DELIMITADOR];
			if (estado_final != 0 && estado_final % 100 == 0) {
				tokens.emplace_back(obtener_tipo_lexema(estado_final, lexema_actual), lexema_actual, linea_actual);
				q = 0;
				lexema_actual.clear();
				continue;
			}
			if (!lexema_actual.empty()) {
				std::string l = lexema_actual + c;
				error_lexico(l, nuevo_estado, linea_actual);
			} else {
				std::string s(1, c);
				error_lexico(s, nuevo_estado, linea_actual);
			}
			q = 0;
			lexema_actual.clear();
			i++;
			continue;
		}

		q = nuevo_estado;

		if (col != 0 || q == 9)
			lexema_actual.push_back(c);
			

		if (q % 100 == 0 && q != 0) {
			tokens.emplace_back(obtener_tipo_lexema(q, lexema_actual), lexema_actual, linea_actual);
			q = 0;
			lexema_actual.clear();
		}

		++i;
	}

	return tokens;
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

	try {
		cadena = leer_archivo(nombre_archivo);
	} catch (...) {
		return 1;
	}
	
	cadena = cadena + " ";

	std::vector<Token> tokens = busca_simbolos(matriz, cadena);

	std::cout << "\n====================================================================\n";
	std::cout << std::left 
	          << std::setw(25) << "TOKEN" 
	          << std::setw(30) << "TIPO DE TOKEN" 
	          << std::setw(10) << "LINEA" << "\n";
	std::cout << "====================================================================\n";

	for (const auto &t : tokens) {
		std::cout << std::left 
		          << std::setw(25) << t.lexema 
		          << std::setw(30) << t.tipo 
		          << std::setw(10) << t.linea << "\n";
	}
	std::cout << "====================================================================\n";

	return 0;
}