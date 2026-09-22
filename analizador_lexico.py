#!/usr/bin/env python3
"""Analizador léxico de Ajolote basado en una matriz de transición.

El reconocimiento se realiza con el autómata definido por ``MATRIZ``.
La función delta es ``delta(estado, columna)`` y no existen expresiones
regulares ni analizadores independientes para los tokens.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional


# Columnas del alfabeto de entrada de la matriz.
DELIMITADOR = 0
DIGITO = 1
PUNTO = 2
SIGNO = 3
EXPONENTE = 4
LETRA = 5
GUION_BAJO = 6
OPERADOR = 7
IGUAL = 8
COMILLAS = 9
PARENTESIS_ABRE = 10
PARENTESIS_CIERRA = 11
MAYOR = 12
MENOR = 13
EXCLAMACION = 14
COMA = 15
PUNTOYCOMA = 16


# Estados aceptores codificados en la matriz.
ACEPTOR_ENTERO = 100
ACEPTOR_REAL = 200
ACEPTOR_CIENTIFICO = 300
ACEPTOR_IDENTIFICADOR = 400
ACEPTOR_OPERADOR = 500
ACEPTOR_ASIGNACION = 600
ACEPTOR_CADENA = 700
ACEPTOR_IGUALDAD = 800
ACEPTOR_PARENTESIS_ABRE = 900
ACEPTOR_PARENTESIS_CIERRA = 1000
ACEPTOR_MAYOR = 1100
ACEPTOR_MENOR = 1200
ACEPTOR_DIFERENTE = 1300
ACEPTOR_COMA = 1400
ACEPTOR_PUNTOYCOMA = 1500


# -1 representa una transición inválida. Los estados 0..9 son los del
# autómata base solicitado: inicio, entero, punto, real, exponente,
# signo del exponente, científico, identificador, igualdad y cadena.
MATRIZ = [
    [0, 1, -5, 500, 7, 7, -5, 500, 8, 9, 900, 1000, 1100, 1200, -6, 1400, 1500, 10, 7, 17, 37, 40, 58, 61, 69, 76, 80, 7, 7, 7, 94, 98, 7, 7, 103],  # q0
    [100, 1, 2, -3, 4, -1, -1, -1, -1, -1, -1, -1, -6, -6, -6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1],  # q1
    [-2, 3, -2, -2, -2, -2, -2, -2, -2, -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2],  # q2
    [200, 3, -2, -2, 4, -2, -2, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2],  # q3
    [-3, 6, -3, 5, -3, -3, -3, -1, -2, -2, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3],  # q4
    [-3, 6, -3, -3, -3, -3, -3, -1, -1, -1, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3],  # q5
    [300, 6, -3, -3, -3, -3, -3, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3],  # q6
    [400, 7, -4, -6, 7, 7, 7, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q7
    [600, -1, -1, -1, -1, -1, -1, -1, 800, -1, -1, -6, -6, -6, -6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1],  # q8
    [9, 9, 9, 9, 9, 9, 9, 9, 9, 700, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9],  # q9
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 7, 7, 7, 7],  # q10
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 12, 7, 7, 7, 7],  # q11
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 13, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q12
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 14, 7, 7, 7, 7, 7, 7, 7],  # q13
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 15, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q14
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 16, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q15
    [1600, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q16
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 18, 7, 7, 7, 7, 7, 7, 32, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q17
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 19, 7, 7, 7, 7, 7, 7, 7, 7, 23, 29, 7, 7, 7, 7],  # q18
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 20, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q19
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 21, 7, 7, 7, 7, 7, 7, 7],  # q20
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 22, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q21
    [1700, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q22
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 24, 7, 7],  # q23
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 25, 7],  # q24
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 26, 7, 7, 7, 7],  # q25
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 27, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q26
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 28, 7, 7, 7, 7],  # q27
    [1800, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q28
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 30, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q29
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 31, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q30
    [1900, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q31
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 33, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q32
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 34, 7, 7, 7, 7],  # q33
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 35, 7, 7, 7, 7],  # q34
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 36, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q35
    [2000, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q36
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 38, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q37
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 39, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q38
    [2100, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q39
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 41, 7, 7, 7, 51, 7, 7, 7],  # q40
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 42, 7, 7],  # q41
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 43, 7, 7, 7, 7, 7, 7, 46, 7, 7, 7, 7, 7, 7],  # q42
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 44, 7, 7, 7, 7],  # q43
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 45, 7, 7, 7, 7, 7, 7],  # q44
    [2200, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q45
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 47, 7, 7, 7, 7, 7, 7, 7],  # q46
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 48, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q47
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 49, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q48
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 50, 7, 7, 7],  # q49
    [2300, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q50
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 52, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q51
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 53, 7, 7, 7, 7],  # q52
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 54, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q53
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 55, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q54
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 56, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q55
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 57, 7, 7, 7, 7],  # q56
    [2400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q57
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 59, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q58
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 60, 7, 7, 7, 7, 7, 7, 7],  # q59
    [2500, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q60
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 62, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q61
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 63, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 66, 7, 7, 7],  # q62
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 64, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q63
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 65, 7, 7, 7, 7],  # q64
    [2600, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q65
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 67, 7, 7],  # q66
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 68, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q67
    [2700, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q68
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 70, 7, 7, 7, 7, 7, 7, 7],  # q69
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 71, 7, 7, 7, 7, 72, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q70
    [2800, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q71
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 73, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q72
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 74, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q73
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 75, 7, 7, 7, 7, 7, 7],  # q74
    [2900, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q75
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 77, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q76
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 78, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q77
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 79, 7, 7, 7, 7],  # q78
    [3000, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q79
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 81, 7, 7, 7, 7, 7, 7, 7, 7, 88, 7],  # q80
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 82, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q81
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 83, 7, 7, 7, 7, 7, 7, 7],  # q82
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 84, 7, 7],  # q83
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 85, 7, 7, 7, 7],  # q84
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 86, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q85
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 87, 7, 7, 7],  # q86
    [3100, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q87
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 89, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q88
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 90, 7, 7, 7],  # q89
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 91, 7, 7],  # q90
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 92, 7, 7, 7, 7],  # q91
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 93, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q92
    [3200, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q93
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 95, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q94
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 96, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q95
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 97, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q96
    [3300, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q97
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 99, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q98
    [3400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 101, 7, 7, 7, 7, 7, 7, 7],  # q99
    [7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q100
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 102, 7, 7, 7, 7, 7, 7],  # q101
    [3500, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q102
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 104, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q103
    [400, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 105, 7, 7, 7, 7],  # q104
    [3600, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7],  # q105
]

COLUMNAS_LETRAS_CLAVE = {'a': 17, 'b': 18, 'c': 19, 'd': 20, 'e': 21, 'f': 22, 'h': 23, 'i': 24, 'l': 25, 'm': 26, 'n': 27, 'o': 28, 'p': 29, 'r': 30, 's': 31, 't': 32, 'u': 33, 'v': 34}
ESTADOS_TRIE = [10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 101, 102, 103, 104, 105]
RESERVADAS = {
    "cadena": "KW_cadena",
    "carac": "KW_carac",
    "inicio": "KW_inicio",
    "entero": "KW_entero",
    "real": "KW_real",
    "hacer": "KW_hacer",
    "escribir": "KW_escribir",
    "leer": "KW_leer",
    "mientras": "KW_mientras",
    "hasta": "KW_hasta",
    "inc": "KW_inc",
    "dec": "KW_dec",
    "ver": "KW_ver",
    "si": "KW_si",
    "entonces": "KW_entonces",
    "sino": "KW_sino",
    "fin": "KW_fin",
    # Palabras utilizadas por el archivo de ejemplo existente.
    "arranca": "KW_arranca",
    "muestra": "KW_muestra",
    "capturar": "KW_capturar",
    "cierra": "KW_cierra",
}


TIPOS_ACEPTORES = {
    ACEPTOR_ENTERO: "NUM_ENTERO",
    ACEPTOR_REAL: "NUM_REAL",
    ACEPTOR_CIENTIFICO: "NUM_NOTACION_CIENTIFICA",
    ACEPTOR_IDENTIFICADOR: "IDENTIFICADOR",
    ACEPTOR_OPERADOR: "OP_ARITMETICO",
    ACEPTOR_ASIGNACION: "OP_ASIGNACION",
    ACEPTOR_CADENA: "CADENA",
    ACEPTOR_IGUALDAD: "OP_IGUALDAD",
    ACEPTOR_PARENTESIS_ABRE: "PARENTESIS_ABRE",
    ACEPTOR_PARENTESIS_CIERRA: "PARENTESIS_CIERRA",
    ACEPTOR_MAYOR: "OP_MAYOR",
    ACEPTOR_MENOR: "OP_MENOR",
    ACEPTOR_DIFERENTE: "OP_DIFERENTE",
    ACEPTOR_COMA: "COMA",
    ACEPTOR_PUNTOYCOMA: "PUNTOYCOMA",
    1600: "KW_arranca",
    1700: "KW_cadena",
    1800: "KW_capturar",
    1900: "KW_carac",
    2000: "KW_cierra",
    2100: "KW_dec",
    2200: "KW_entero",
    2300: "KW_entonces",
    2400: "KW_escribir",
    2500: "KW_fin",
    2600: "KW_hacer",
    2700: "KW_hasta",
    2800: "KW_inc",
    2900: "KW_inicio",
    3000: "KW_leer",
    3100: "KW_mientras",
    3200: "KW_muestra",
    3300: "KW_real",
    3400: "KW_si",
    3500: "KW_sino",
    3600: "KW_ver",
}

@dataclass
class Token:
    tipo: str
    valor: str
    linea: int


class LexError(Exception):
    def __init__(self, mensaje: str, linea: int):
        super().__init__(f"Línea {linea}: {mensaje}")
        self.linea = linea


def _obtener_columna_base(caracter: str) -> int:
    if caracter in " \t\r\n":
        return DELIMITADOR
    if caracter.isdigit():
        return DIGITO
    if caracter == ".":
        return PUNTO
    if caracter in "+-":
        return SIGNO
    if caracter in "eE":
        return EXPONENTE
    if caracter.isalpha():
        return LETRA
    if caracter == "_":
        return GUION_BAJO
    if caracter in "*/%":
        return OPERADOR
    if caracter == "=":
        return IGUAL
    if caracter == '"':
        return COMILLAS
    if caracter == "(":
        return PARENTESIS_ABRE
    if caracter == ")":
        return PARENTESIS_CIERRA
    if caracter == ">":
        return MAYOR
    if caracter == "<":
        return MENOR
    if caracter == "!":
        return EXCLAMACION
    if caracter == ",":
        return COMA
    if caracter == ";":
        return PUNTOYCOMA
    return -1


def obtener_columna(caracter: str, estado: int = 0) -> int:
    if caracter in COLUMNAS_LETRAS_CLAVE and (estado == 0 or estado in ESTADOS_TRIE):
        return COLUMNAS_LETRAS_CLAVE[caracter]
    return _obtener_columna_base(caracter)


def delta(estado: int, columna: int) -> int:
    """Función delta del autómata: δ(q, a) = MATRIZ[q][columna]."""
    if estado < 0 or estado >= len(MATRIZ):
        return -1
    if columna < 0 or columna >= len(MATRIZ[estado]):
        return -1
    return MATRIZ[estado][columna]


def es_aceptor(estado: int) -> bool:
    return estado != 0 and estado % 100 == 0


def tipo_lexema(estado: int, lexema: str) -> str:
    return TIPOS_ACEPTORES[estado]


def tokenizar(texto: str) -> List[Token]:
    tokens: List[Token] = []
    estado = 0
    lexema = ""
    linea = 1
    linea_inicio = 1
    indice = 0

    while indice < len(texto):
        caracter = texto[indice]
        columna = obtener_columna(caracter, estado)
        siguiente = delta(estado, columna)

        if columna == -1:
            if es_aceptor(delta(estado, DELIMITADOR)):
                aceptado = delta(estado, DELIMITADOR)
                tokens.append(Token(tipo_lexema(aceptado, lexema), lexema, linea_inicio))
                estado, lexema = 0, ""
                continue
            raise LexError(f"Carácter no reconocido: '{caracter}'", linea)

        if siguiente < 0:
            aceptado = delta(estado, DELIMITADOR)
            if es_aceptor(aceptado):
                tokens.append(Token(tipo_lexema(aceptado, lexema), lexema, linea_inicio))
                estado, lexema = 0, ""
                continue
            fragmento = lexema + caracter
            raise LexError(f"Token inválido: '{fragmento}'", linea)

        if columna == DELIMITADOR and es_aceptor(siguiente):
            tokens.append(Token(tipo_lexema(siguiente, lexema), lexema, linea_inicio))
            estado, lexema = 0, ""
            continue

        if estado == 9 and columna != COMILLAS:
            estado = siguiente
            lexema += caracter
            if caracter == "\n":
                linea += 1
            indice += 1
            continue

        if estado == 0 and columna == DELIMITADOR:
            if caracter == "\n":
                linea += 1
            indice += 1
            continue

        if estado == 9 and columna == COMILLAS:
            lexema += caracter
            tokens.append(Token(TIPOS_ACEPTORES[ACEPTOR_CADENA], lexema, linea_inicio))
            estado, lexema = 0, ""
            indice += 1
            continue

        estado = siguiente
        if columna != DELIMITADOR:
            if not lexema:
                linea_inicio = linea
            lexema += caracter
        indice += 1

        if es_aceptor(estado):
            tokens.append(Token(tipo_lexema(estado, lexema), lexema, linea_inicio))
            estado, lexema = 0, ""

    if lexema:
        aceptado = delta(estado, DELIMITADOR)
        if not es_aceptor(aceptado):
            raise LexError(f"Token incompleto: '{lexema}'", linea_inicio)
        tokens.append(Token(tipo_lexema(aceptado, lexema), lexema, linea_inicio))

    return tokens


def leer_archivo(ruta: str) -> str:
    path = Path(ruta)
    if not path.exists():
        raise FileNotFoundError(f"No se encontró el archivo: {ruta}")
    return path.read_text(encoding="utf-8")


def imprimir_tokens(tokens: List[Token]) -> None:
    print(f"{'LINEA':<8} {'TIPO':<28} {'VALOR'}")
    print("-" * 64)
    for token in tokens:
        print(f"{token.linea:<8} {token.tipo:<28} {token.valor}")


def main(argv: Optional[List[str]] = None) -> int:
    args = sys.argv[1:] if argv is None else argv
    archivo = args[0] if args else "ejemplo.nexo"
    try:
        imprimir_tokens(tokenizar(leer_archivo(archivo)))
        return 0
    except (FileNotFoundError, LexError) as exc:
        print(f"[ERROR] {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
