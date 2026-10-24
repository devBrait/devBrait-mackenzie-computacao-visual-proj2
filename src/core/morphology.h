#ifndef CORE_MORPHOLOGY_H
#define CORE_MORPHOLOGY_H

#include <stdbool.h>

#include "image.h"

// Operações sobre a imagem binária, com elemento estruturante 3x3.
// Todas alteram a imagem no lugar e falham só por falta de memória

// Erosões seguidas de dilatações. Apaga pontinhos e ligações finas
bool morphology_open(GrayImage *binary, int iterations);

// Dilatações seguidas de erosões. Fecha furos e rachaduras pequenas
bool morphology_close(GrayImage *binary, int iterations);

#endif
