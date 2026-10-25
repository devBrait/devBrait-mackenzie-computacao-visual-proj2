#include "filter.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// Três sigmas cobrem 99,7% da curva, o resto não muda o resultado
static const float KERNEL_RADIUS_SIGMAS = 3.0f;
static const float MAX_GRAY = 255.0f;

// Repete o pixel da borda, assim a moldura não escurece
static int clamp_index(int index, int size)
{
    if (index < 0) {
        return 0;
    }

    if (index >= size) {
        return size - 1;
    }

    return index;
}

// Pesos normalizados para somar 1 e não mudar o brilho da imagem
static float *build_kernel(float sigma, int radius)
{
    int size = 2 * radius + 1;
    float *kernel = malloc(sizeof(float) * (size_t)size);

    if (kernel == NULL) {
        return NULL;
    }

    float sum = 0.0f;

    for (int i = -radius; i <= radius; i++) {
        float weight = expf(-(float)(i * i) / (2.0f * sigma * sigma));
        kernel[i + radius] = weight;
        sum += weight;
    }

    for (int i = 0; i < size; i++) {
        kernel[i] /= sum;
    }

    return kernel;
}

bool gaussian_blur(const GrayImage *source, GrayImage *destination, float sigma)
{
    int width = source->width;
    int height = source->height;
    size_t count = (size_t)width * (size_t)height;

    if (sigma <= 0.0f) {
        memcpy(destination->pixels, source->pixels, count);
        return true;
    }

    int radius = (int)ceilf(KERNEL_RADIUS_SIGMAS * sigma);
    float *kernel = build_kernel(sigma, radius);

    // Passada intermediária em float para arredondar uma vez só no fim
    float *rows = malloc(sizeof(float) * count);

    if (kernel == NULL || rows == NULL) {
        free(kernel);
        free(rows);
        return false;
    }

    // Duas passadas dão o mesmo resultado do kernel com bem menos contas
    for (int y = 0; y < height; y++) {
        const unsigned char *line = source->pixels + (size_t)y * (size_t)width;
        float *out = rows + (size_t)y * (size_t)width;

        for (int x = 0; x < width; x++) {
            float sum = 0.0f;

            for (int k = -radius; k <= radius; k++) {
                sum += kernel[k + radius] * line[clamp_index(x + k, width)];
            }

            out[x] = sum;
        }
    }

    for (int y = 0; y < height; y++) {
        unsigned char *out = destination->pixels + (size_t)y * (size_t)width;

        for (int x = 0; x < width; x++) {
            float sum = 0.0f;

            for (int k = -radius; k <= radius; k++) {
                size_t row = (size_t)clamp_index(y + k, height);
                sum += kernel[k + radius] * rows[row * (size_t)width + (size_t)x];
            }

            // Erro de arredondamento do float pode passar um pouco de 255
            out[x] = (unsigned char)(fminf(sum, MAX_GRAY) + 0.5f);
        }
    }

    free(kernel);
    free(rows);

    return true;
}
