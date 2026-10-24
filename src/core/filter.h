#ifndef CORE_FILTER_H
#define CORE_FILTER_H

#include <stdbool.h>

#include "image.h"

// Suaviza com Gaussiano separável e borda replicada.
// O destino precisa ter o mesmo tamanho da origem
bool gaussian_blur(const GrayImage *source, GrayImage *destination, float sigma);

#endif
