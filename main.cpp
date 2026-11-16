#include "parser.cpp"

int main() {
	int opcion = 0;
	while (true) {
		std::cout << "\n--- MENU ANALIZADOR ---\n";
		std::cout << "1. Leer y analizar un archivo\n";
		std::cout << "2. Salir\n";
		std::cout << "Seleccione una opcion: ";
		if (!(std::cin >> opcion)) {
			std::cin.clear();
			std::string basura;
			std::cin >> basura;
			std::cout << "Opcion invalida. Intente de nuevo.\n";
			continue;
		}

		if (opcion == 2) {
			std::cout << "Saliendo del programa...\n";
			break;
		} else if (opcion == 1) {
			std::string nombre_archivo;
			std::cout << "Ingrese el nombre del archivo (ej. ejemplo.nexo): ";
			std::cin >> nombre_archivo;
			try {
				std::string cadena = leer_archivo(nombre_archivo);
				cadena = cadena + " ";
				std::vector<struct LexerToken> tokens = analizar_lexico(cadena);
				imprimir_tokens(tokens);
				parserTokens = tokens;
				int res = parsear();
				if (res == 0) {
					std::cout << "\nAnalisis sintactico exitoso.\n";
				} else {
					std::cout << "\nAnalisis sintactico fallido por errores.\n";
				}
			} catch (const std::exception &e) {
				std::cout << "No se pudo procesar el archivo.\n";
			}
		} else {
			std::cout << "Opcion no valida. Intente de nuevo.\n";
		}
	}
	return 0;
}
