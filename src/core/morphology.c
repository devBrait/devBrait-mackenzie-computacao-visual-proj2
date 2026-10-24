#include "morphology.h"

#include <string.h>

static const unsigned char WHITE = 255;
static const unsigned char BLACK = 0;

// Repete o pixel da borda, assim a moldura não apaga nem cria objetos
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

// Se algum vizinho 3x3 tiver a cor spread, o pixel recebe essa cor.
// Com preto é a erosão e com branco é a dilatação
static void spread_3x3(const GrayImage *source, GrayImage *destination, unsigned char spread)
{
    int width = source->width;
    int height = source->height;
    unsigned char other = spread == WHITE ? BLACK : WHITE;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char result = other;

            for (int dy = -1; dy <= 1 && result == other; dy++) {
                size_t row = (size_t)clamp_index(y + dy, height) * (size_t)width;

                for (int dx = -1; dx <= 1; dx++) {
                    if (source->pixels[row + (size_t)clamp_index(x + dx, width)] == spread) {
                        result = spread;
                        break;
                    }
                }
            }

            destination->pixels[(size_t)y * (size_t)width + (size_t)x] = result;
        }
    }
}

// Cada passada lê a imagem e escreve no temporário, que depois volta para a imagem
static void repeat_spread(GrayImage *image, GrayImage *temp, unsigned char spread,
                          int iterations)
{
    size_t count = (size_t)image->width * (size_t)image->height;

    for (int i = 0; i < iterations; i++) {
        spread_3x3(image, temp, spread);
        memcpy(image->pixels, temp->pixels, count);
    }
}

bool morphology_open(GrayImage *binary, int iterations)
{
    GrayImage temp;

    if (!gray_image_alloc(&temp, binary->width, binary->height)) {
        return false;
    }

    repeat_spread(binary, &temp, BLACK, iterations);
    repeat_spread(binary, &temp, WHITE, iterations);

    gray_image_free(&temp);

    return true;
}

bool morphology_erode(GrayImage *binary, int iterations)
{
    GrayImage temp;

    if (!gray_image_alloc(&temp, binary->width, binary->height)) {
        return false;
    }

    repeat_spread(binary, &temp, BLACK, iterations);

    gray_image_free(&temp);

    return true;
}

bool morphology_close(GrayImage *binary, int iterations)
{
    GrayImage temp;

    if (!gray_image_alloc(&temp, binary->width, binary->height)) {
        return false;
    }

    repeat_spread(binary, &temp, WHITE, iterations);
    repeat_spread(binary, &temp, BLACK, iterations);

    gray_image_free(&temp);

    return true;
}
