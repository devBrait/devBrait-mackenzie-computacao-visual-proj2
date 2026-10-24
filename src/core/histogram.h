#ifndef CORE_HISTOGRAM_H
#define CORE_HISTOGRAM_H

#include <stdbool.h>
#include <stddef.h>

#include "image.h"

#define HISTOGRAM_LEVELS 256

// Quantos pixels existem de cada nível de cinza
void compute_histogram(const GrayImage *image, size_t histogram[HISTOGRAM_LEVELS]);

// Limiar que melhor separa as duas classes do histograma, pelo método de Otsu
int otsu_threshold(const size_t histogram[HISTOGRAM_LEVELS]);

// Pixels acima do limiar viram 255 e os demais viram 0
void binarize(const GrayImage *source, GrayImage *destination, int threshold);

// Deixa os objetos brancos. Se a borda for na maioria branca, o fundo está
// branco e a imagem é invertida. Devolve true quando inverteu
bool fix_polarity(GrayImage *binary);

#endif
