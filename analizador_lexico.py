class Token:
    def __init__(self, tipo, lexema):
        self.tipo = tipo
        self.lexema = lexema


# ---------------------------------------------------------------
# Clasifica un caracter en la columna que le corresponde dentro
# de la matriz de transiciones (igual que el switch del boceto)
# Devuelve -1 si el simbolo no pertenece al lenguaje
# ---------------------------------------------------------------
def obtener_columna(c):
    if c in (' ', '\t', '\n', ''): # delimitadores 
        return 0
    if c.isdigit(): # digitos
        return 1
    if c == '.': # punto
        return 2
    if c in ('+', '-'): # signos
        return 3
    if c in ('E', 'e'): # notacion cientifica
        return 4
    if c.isalpha(): # letras
        return 5
    if c == '_': # guion bajo
        return 6
    return -1

# ---------------------------------------------------------------
# Traduce el estado de aceptacion (q) al nombre del token,
# tal como en el switch de "q" del boceto
# ---------------------------------------------------------------
def obtener_tipo_token(q):
    if q == 100:
        return "NumEntero"
    if q == 200:
        return "NumReal"
    if q == 300:
        return "NumNotacionCientifica"
    if q == 400:
        return "Id"
    return "Token"

#---------------------------------------------------------------
# Maneja los errores de lexico, mostrando el lexema que no pudo ser reconocido
#---------------------------------------------------------------
def error_lexico(lexema, error_code=-1):
    print(f"Error lexico en: {lexema}")
    if error_code == -1:
        print("Error de formato en el numero real o notacion cientifica")
    if error_code == -2:
        print("Error de formato en el numero real")
    if error_code == -3:   
        print("El exponente debe ser un numero entero")
    if error_code == -4:
        print("No se permiten puntos despues de un identificador")
    if error_code == -5:
        print("Identificador no valido")

# ---------------------------------------------------------------
# Identificar simbolos lexicos:
#   matriz  -> matriz de transiciones [estado][columna] = nuevo estado
#              (usa -1 en la matriz para marcar "transicion invalida")
#   cadena  -> codigo fuente a tokenizar
# ---------------------------------------------------------------
def busca_simbolos(matriz, cadena) -> list[Token]:
    tokens = []
    q = 0
    lexema_actual = ""

    for i in range(0, len(cadena)):
        col = obtener_columna(cadena[i])

        if col == -1:
            error_lexico(cadena[i], -1)
            continue

        nuevo_estado = matriz[q][col]

        # invalido
        if nuevo_estado < 0:
            if lexema_actual:
                error_lexico(lexema_actual + cadena[i], nuevo_estado)
            q = 0
            lexema_actual = ""
            continue

        q = nuevo_estado

        # los delimitadores no forman parte del lexema
        if col != 0:
            lexema_actual += cadena[i]

        # estado de aceptacion alcanzado -> se cierra el token
        if q % 100 == 0 and q != 0:
            tokens.append(Token(obtener_tipo_token(q), lexema_actual))
            q = 0
            lexema_actual = ""

    return tokens


# ---------------------------------------------------------------
# Ejemplo de uso con una matriz minima de prueba (solo reconoce
# enteros). Sustituye esta matriz por la que diseñes en tu tabla
# de simbolos (la del segundo boceto).
# ---------------------------------------------------------------
if __name__ == "__main__":
    matriz = [
        [0,    1,  -5,  0,  7,   7,  -5],   # estado 0
        [100,  1,  2,  100,  4,   -1,  -1], # estado 1
        [-2,   3, -2, -2, -2, -2, -2], # estado 2
        [200,   3, -2, 200, 4, -2, -2],  # estado 3
        [-3, 6, -3, 5, -3, -3, -3], # estado 4
        [-3, 6, -3, -3, -3, -3, -3], # estado 5
        [300, 6, -3, 300, -3, -3, -3], # estado 6
        [400, 7, -4, 400, 7, 7, 7] # estado 7
    ]

    cadena = input("Introduce una linea: ")
    cadena = cadena.strip() + " "  # Agrega un espacio al final para procesar el último token

    tokens = busca_simbolos(matriz, cadena)

    for t in tokens:
        print(t.lexema, "->", t.tipo)
