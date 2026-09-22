# nexocompiler
compilador especifico hecho para IA

## Analizador léxico de Ajolote

El proyecto incluye un analizador léxico que reconoce:
- enteros
- reales
- notación científica
- identificadores
- palabras reservadas del lenguaje Ajolote
- operadores y delimitadores básicos

El analizador de Python conserva el autómata finito y su matriz de
transición. Cada carácter se convierte en una columna del alfabeto y el
estado siguiente se obtiene exclusivamente mediante la función delta:
`delta(estado, columna)`.

La matriz está declarada explícitamente en el código como una lista de filas
(`MATRIZ`); no se genera durante la ejecución. Las palabras reservadas tienen
una transición por cada letra y sus estados finales producen los tokens
correspondientes.

### Uso

```bash
python analizador_lexico.py ejemplo.nexo
```

Si no se pasa archivo, el programa usa `ejemplo.nexo` por defecto.
