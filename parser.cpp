#include "analizador_lexico.cpp"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <stack>

extern std::vector<LexerToken> parserTokens;
extern int token_actual;
extern bool error_sintactico;

std::vector<LexerToken> parserTokens;
int token_actual = 0;

bool error_sintactico = false;

struct ASTNode {
    std::string valor;
    std::vector<ASTNode*> hijos;
    ASTNode(const std::string& v) : valor(v) {}
};

void imprimir_arbol(ASTNode* nodo, const std::string& prefijo = "", bool es_ultimo = true) {
    if (!nodo) return;
    std::cout << prefijo << (es_ultimo ? "└── " : "├── ") << nodo->valor << std::endl;
    std::string nuevo_prefijo = prefijo + (es_ultimo ? "    " : "│   ");
    for (size_t i = 0; i < nodo->hijos.size(); ++i) {
        imprimir_arbol(nodo->hijos[i], nuevo_prefijo, i == nodo->hijos.size() - 1);
    }
}

std::unordered_set<std::string> no_terminales = {
    "statements", "statement", "block_statement", "io_statement", "ml_statement",
    "control_statement", "assignment_statement", "expression", "expr_prime",
    "term", "term_prime", "factor", "opt_sino", "ml_func", "opt_semic"
};

std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> tabla_parsing;

void inicializar_tabla_ll1() {
    // --- statements ---
    tabla_parsing["statements"]["KW_ARRANCA"]  = {"statement", "statements"};
    tabla_parsing["statements"]["KW_REPETIR"]   = {"statement", "statements"};
    tabla_parsing["statements"]["KW_SI"]        = {"statement", "statements"};
    tabla_parsing["statements"]["KW_MUESTRA"]   = {"statement", "statements"};
    tabla_parsing["statements"]["KW_LIMPIA"]    = {"statement", "statements"};
    tabla_parsing["statements"]["KW_DIVIDE"]    = {"statement", "statements"};
    tabla_parsing["statements"]["KW_ENTRENA"]   = {"statement", "statements"};
    tabla_parsing["statements"]["KW_GUARDA"]    = {"statement", "statements"};
    tabla_parsing["statements"]["Id"]           = {"statement", "statements"};
    tabla_parsing["statements"]["NumEntero"]     = {"statement", "statements"};
    tabla_parsing["statements"]["NumReal"]       = {"statement", "statements"};
    tabla_parsing["statements"]["ParentesisAbre"] = {"statement", "statements"};
    tabla_parsing["statements"]["KW_CIERRA"]    = {"epsilon"};
    tabla_parsing["statements"]["KW_SINO"]      = {"epsilon"};
    tabla_parsing["statements"]["KW_HASTA"]     = {"epsilon"};
    tabla_parsing["statements"]["EOF"]          = {"epsilon"};

    // --- statement ---
    tabla_parsing["statement"]["KW_ARRANCA"]  = {"block_statement"};
    tabla_parsing["statement"]["KW_REPETIR"]   = {"control_statement"};
    tabla_parsing["statement"]["KW_SI"]        = {"control_statement"};
    tabla_parsing["statement"]["KW_MUESTRA"]   = {"io_statement"};
    tabla_parsing["statement"]["KW_LIMPIA"]    = {"ml_statement"};
    tabla_parsing["statement"]["KW_DIVIDE"]    = {"ml_statement"};
    tabla_parsing["statement"]["KW_ENTRENA"]   = {"ml_statement"};
    tabla_parsing["statement"]["KW_GUARDA"]    = {"ml_statement"};
    tabla_parsing["statement"]["Id"]           = {"assignment_statement"};
    tabla_parsing["statement"]["NumEntero"]     = {"expression", "opt_semic"};
    tabla_parsing["statement"]["NumReal"]       = {"expression", "opt_semic"};
    tabla_parsing["statement"]["ParentesisAbre"] = {"expression", "opt_semic"};

    // --- block_statement ---
    tabla_parsing["block_statement"]["KW_ARRANCA"] = { "KW_ARRANCA", "statements", "KW_CIERRA" };

    // --- io_statement ---
    tabla_parsing["io_statement"]["KW_MUESTRA"] = { "KW_MUESTRA", "CadenaCaracteres" };

    // --- ml_statement ---
    tabla_parsing["ml_statement"]["KW_LIMPIA"]  = { "KW_LIMPIA", "Id" };
    tabla_parsing["ml_statement"]["KW_DIVIDE"]  = { "KW_DIVIDE", "Id", "KW_EN", "NumEntero" };
    tabla_parsing["ml_statement"]["KW_ENTRENA"] = { "KW_ENTRENA", "Id", "KW_CON", "Id" };
    tabla_parsing["ml_statement"]["KW_GUARDA"]  = { "KW_GUARDA", "Id", "KW_EN", "CadenaCaracteres" };

    // --- control_statement ---
    tabla_parsing["control_statement"]["KW_REPETIR"] = { "KW_REPETIR", "NumEntero", "KW_VECES", "statements", "KW_HASTA", "KW_ALCANZADO", "expression" };
    tabla_parsing["control_statement"]["KW_SI"]      = { "KW_SI", "expression", "statements", "opt_sino", "KW_CIERRA" };

    // --- opt_sino ---
    tabla_parsing["opt_sino"]["KW_SINO"]   = { "KW_SINO", "statements" };
    tabla_parsing["opt_sino"]["KW_CIERRA"] = { "epsilon" };

    // --- assignment_statement ---
    tabla_parsing["assignment_statement"]["Id"] = { "Id", "Asignacion", "expression", "opt_semic" };

    // --- opt_semic ---
    tabla_parsing["opt_semic"]["SEMIC"]       = {"SEMIC"};
    tabla_parsing["opt_semic"]["KW_ARRANCA"]   = {"epsilon"};
    tabla_parsing["opt_semic"]["KW_REPETIR"]   = {"epsilon"};
    tabla_parsing["opt_semic"]["KW_SI"]        = {"epsilon"};
    tabla_parsing["opt_semic"]["KW_MUESTRA"]   = {"epsilon"};
    tabla_parsing["opt_semic"]["KW_LIMPIA"]    = {"epsilon"};
    tabla_parsing["opt_semic"]["KW_DIVIDE"]    = {"epsilon"};
    tabla_parsing["opt_semic"]["KW_ENTRENA"]   = {"epsilon"};
    tabla_parsing["opt_semic"]["KW_GUARDA"]    = {"epsilon"};
    tabla_parsing["opt_semic"]["Id"]           = {"epsilon"};
    tabla_parsing["opt_semic"]["KW_CIERRA"]    = {"epsilon"};
    tabla_parsing["opt_semic"]["EOF"]          = {"epsilon"};

    // --- expression ---
    tabla_parsing["expression"]["NumEntero"]     = {"term", "expr_prime"};
    tabla_parsing["expression"]["NumReal"]       = {"term", "expr_prime"};
    tabla_parsing["expression"]["Id"]            = {"term", "expr_prime"};
    tabla_parsing["expression"]["Signo"]          = {"term", "expr_prime"};
    tabla_parsing["expression"]["ParentesisAbre"] = {"term", "expr_prime"};
    tabla_parsing["expression"]["KW_CARGA"]      = {"term", "expr_prime"};
    tabla_parsing["expression"]["KW_CREA"]       = {"term", "expr_prime"};
    tabla_parsing["expression"]["KW_PREDICE"]    = {"term", "expr_prime"};

    // --- expr_prime ---
    tabla_parsing["expr_prime"]["Signo"]              = {"Signo", "term", "expr_prime"};
    tabla_parsing["expr_prime"]["Operador"]           = {"Operador", "term", "expr_prime"};
    tabla_parsing["expr_prime"]["ComparadorMayor"]    = {"ComparadorMayor", "term", "expr_prime"};
    tabla_parsing["expr_prime"]["SEMIC"]              = {"epsilon"};
    tabla_parsing["expr_prime"]["ParentesisCierra"]   = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_CIERRA"]          = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_SINO"]            = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_HASTA"]           = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_VECES"]           = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_ARRANCA"]         = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_REPETIR"]         = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_SI"]              = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_MUESTRA"]         = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_LIMPIA"]          = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_DIVIDE"]          = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_ENTRENA"]         = {"epsilon"};
    tabla_parsing["expr_prime"]["KW_GUARDA"]          = {"epsilon"};
    tabla_parsing["expr_prime"]["Id"]                 = {"epsilon"};
    tabla_parsing["expr_prime"]["EOF"]                = {"epsilon"};

    // --- term ---
    tabla_parsing["term"]["NumEntero"]     = {"factor", "term_prime"};
    tabla_parsing["term"]["NumReal"]       = {"factor", "term_prime"};
    tabla_parsing["term"]["Id"]            = {"factor", "term_prime"};
    tabla_parsing["term"]["ParentesisAbre"] = {"factor", "term_prime"};
    tabla_parsing["term"]["KW_CARGA"]      = {"factor", "term_prime"};
    tabla_parsing["term"]["KW_CREA"]       = {"factor", "term_prime"};
    tabla_parsing["term"]["KW_PREDICE"]    = {"factor", "term_prime"};
    tabla_parsing["term"]["Signo"]         = {"factor", "term_prime"};

    // --- term_prime ---
    tabla_parsing["term_prime"]["Signo"]              = {"epsilon"};
    tabla_parsing["term_prime"]["Operador"]           = {"epsilon"};
    tabla_parsing["term_prime"]["ComparadorMayor"]    = {"epsilon"};
    tabla_parsing["term_prime"]["SEMIC"]              = {"epsilon"};
    tabla_parsing["term_prime"]["ParentesisCierra"]   = {"epsilon"};
    tabla_parsing["term_prime"]["KW_CIERRA"]          = {"epsilon"};
    tabla_parsing["term_prime"]["KW_SINO"]            = {"epsilon"};
    tabla_parsing["term_prime"]["KW_HASTA"]           = {"epsilon"};
    tabla_parsing["term_prime"]["KW_VECES"]           = {"epsilon"};
    tabla_parsing["term_prime"]["KW_ARRANCA"]         = {"epsilon"};
    tabla_parsing["term_prime"]["KW_REPETIR"]         = {"epsilon"};
    tabla_parsing["term_prime"]["KW_SI"]              = {"epsilon"};
    tabla_parsing["term_prime"]["KW_MUESTRA"]         = {"epsilon"};
    tabla_parsing["term_prime"]["KW_LIMPIA"]          = {"epsilon"};
    tabla_parsing["term_prime"]["KW_DIVIDE"]          = {"epsilon"};
    tabla_parsing["term_prime"]["KW_ENTRENA"]         = {"epsilon"};
    tabla_parsing["term_prime"]["KW_GUARDA"]          = {"epsilon"};
    tabla_parsing["term_prime"]["Id"]                 = {"epsilon"};
    tabla_parsing["term_prime"]["EOF"]                = {"epsilon"};

    // --- factor ---
    tabla_parsing["factor"]["NumEntero"]     = {"NumEntero"};
    tabla_parsing["factor"]["NumReal"]       = {"NumReal"};
    tabla_parsing["factor"]["Id"]            = {"Id"};
    tabla_parsing["factor"]["Signo"]         = {"Signo", "factor"};
    tabla_parsing["factor"]["ParentesisAbre"] = {"ParentesisAbre", "expression", "ParentesisCierra"};
    tabla_parsing["factor"]["KW_CARGA"]      = {"ml_func"};
    tabla_parsing["factor"]["KW_CREA"]       = {"ml_func"};
    tabla_parsing["factor"]["KW_PREDICE"]    = {"ml_func"};

    // --- ml_func ---
    tabla_parsing["ml_func"]["KW_CARGA"]   = {"KW_CARGA", "ParentesisAbre", "CadenaCaracteres", "ParentesisCierra"};
    tabla_parsing["ml_func"]["KW_CREA"]    = {"KW_CREA", "Id"};
    tabla_parsing["ml_func"]["KW_PREDICE"] = {"KW_PREDICE", "Id"};
}

LexerToken lex() {
  if (token_actual < parserTokens.size()) {
    return parserTokens[token_actual];
  }
  return LexerToken("EOF", "", -1);
}

void advance() {
  if (token_actual < parserTokens.size()) {
    token_actual++;
  }
}

int parsear() {
  inicializar_tabla_ll1();
  token_actual = 0;
  error_sintactico = false;
  int errores_detectados = 0;

  std::stack<std::string> pila_sintactica;
  std::stack<ASTNode*> pila_arbol;

  pila_sintactica.push("EOF");

  ASTNode* raiz = new ASTNode("statements");
  pila_sintactica.push("statements");
  pila_arbol.push(raiz);

  while (!pila_sintactica.empty()) {
    std::string cima = pila_sintactica.top();
    LexerToken tok = lex();
    std::string token_actual_tipo = tok.tipo;

    if (cima == "EOF" && token_actual_tipo == "EOF") {
      pila_sintactica.pop();
      break;
    }

    if (no_terminales.find(cima) == no_terminales.end()) {
      // Símbolo Terminal
      if (cima == token_actual_tipo || cima == tok.lexema) {
        ASTNode* nodo_actual = pila_arbol.top();
        pila_arbol.pop();
        pila_sintactica.pop();

        nodo_actual->valor = tok.tipo + " (" + tok.lexema + ")";
        advance();
      } else {
        std::cout << "[ERROR SINTACTICO] Se esperaba '" << cima << "' en linea " << tok.linea << " pero se encontro '" << tok.lexema << "'" << std::endl;
        error_sintactico = true;
        errores_detectados++;

        // Limpiamos los terminales fallidos de la pila para intentar reanudarse
        pila_sintactica.pop();
        pila_arbol.pop();
      }
    } else {
      // Símbolo No Terminal
      ASTNode* nodo_actual = pila_arbol.top();

      std::string clave_entrada = token_actual_tipo;
      if (tabla_parsing[cima].find(tok.lexema) != tabla_parsing[cima].end()) {
        clave_entrada = tok.lexema;
      }

      if (tabla_parsing[cima].find(clave_entrada) != tabla_parsing[cima].end()) {
        pila_sintactica.pop();
        pila_arbol.pop();

        std::vector<std::string> produccion = tabla_parsing[cima][clave_entrada];

        if (produccion.size() == 1 && produccion[0] == "epsilon") {
          nodo_actual->hijos.push_back(new ASTNode("epsilon"));
        } else {
          std::vector<ASTNode*> nuevos_nodos;
          for (const std::string& simb : produccion) {
             ASTNode* nuevo_hijo = new ASTNode(simb);
             nodo_actual->hijos.push_back(nuevo_hijo);
             nuevos_nodos.push_back(nuevo_hijo);
          }
          for (int i = nuevos_nodos.size() - 1; i >= 0; --i) {
            pila_sintactica.push(produccion[i]);
            pila_arbol.push(nuevos_nodos[i]);
          }
        }
      } else {
        std::cout << "[ERROR SINTACTICO] No existe regla en la tabla predictiva para No-Terminal '" << cima
                  << "' con token '" << tok.lexema << "' (tipo: " << tok.tipo << ", linea " << tok.linea << ")" << std::endl;
        error_sintactico = true;
        errores_detectados++;

        // --- ESTRATEGIA: LIMPIAR PILA HASTA UN STATEMENTS O EOF Y DESCARTAR TOKENS HASTA ';' ---
        while (!pila_sintactica.empty() && pila_sintactica.top() != "statements" && pila_sintactica.top() != "EOF") {
          pila_sintactica.pop();
          if (!pila_arbol.empty()) pila_arbol.pop();
        }

        while (tok.tipo != "EOF" && tok.tipo != "SEMIC") {
          advance();
          tok = lex();
        }
        if (tok.tipo == "SEMIC") {
          advance(); // Consumimos el ';' para que la siguiente instrucción empiece limpia
        }

        nodo_actual->valor = cima + " [Error Sintactico Recuperado]";
      }
    }
  }

  if (errores_detectados > 0) {
    std::cout << "\n[INFO] El analisis finalizo con " << errores_detectados << " errores sintacticos.\n";
    return 1;
  }

  std::cout << "=== ARBOL SINTACTICO (LL1 TABLA) ===\n";
  imprimir_arbol(raiz);
  std::cout << "====================================\n";

  return 0;
}
