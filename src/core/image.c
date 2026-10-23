#include "image.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

// Só os formatos aceitos pela página, o que deixa o wasm menor
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
// Caminhos com acento no Windows chegam em UTF8 pelo SDL
#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static const int RGBA_CHANNELS = 4;

// Pesos da luminância Rec. 709, o olho é bem mais sensível ao verde
static const double LUMA_RED = 0.2125;
static const double LUMA_GREEN = 0.7154;
static const double LUMA_BLUE = 0.0721;

// Converte o resultado do stb e deixa a struct zerada em caso de falha
static bool finish_load(RgbaImage *image, unsigned char *pixels, int width, int height)
{
    if (pixels == NULL) {
        image->width = 0;
        image->height = 0;
        image->pixels = NULL;
        return false;
    }
    image->width = width;
    image->height = height;
    image->pixels = pixels;
    return true;
}

bool rgba_image_load_from_memory(RgbaImage *image, const unsigned char *data, size_t size)
{
    // O stb recebe o tamanho como int
    if (data == NULL || size == 0 || size > INT_MAX) {
        stbi__err("invalid buffer", "Buffer vazio ou grande demais");
        return finish_load(image, NULL, 0, 0);
    }

    int width = 0;
    int height = 0;
    int channelsInFile = 0;
    // Pedir 4 canais converte cinza e RGB para RGBA com alfa opaco
    unsigned char *pixels = stbi_load_from_memory(data, (int)size, &width, &height,
                                                  &channelsInFile, RGBA_CHANNELS);
    return finish_load(image, pixels, width, height);
}

bool rgba_image_load_from_file(RgbaImage *image, const char *path)
{
    int width = 0;
    int height = 0;
    int channelsInFile = 0;
    unsigned char *pixels = stbi_load(path, &width, &height, &channelsInFile, RGBA_CHANNELS);
    return finish_load(image, pixels, width, height);
}

void rgba_image_free(RgbaImage *image)
{
    if (image == NULL) {
        return;
    }
    // A memória veio do stb, então volta por ele
    stbi_image_free(image->pixels);
    image->pixels = NULL;
    image->width = 0;
    image->height = 0;
}

bool rgba_image_limit_size(RgbaImage *image, int maxSide)
{
    int largestSide = image->width > image->height ? image->width : image->height;
    if (largestSide <= maxSide) {
        return true;
    }

    double scale = (double)maxSide / largestSide;
    int width = (int)(image->width * scale + 0.5);
    int height = (int)(image->height * scale + 0.5);
    // Uma foto muito estreita não pode virar zero pixels
    width = width < 1 ? 1 : width;
    height = height < 1 ? 1 : height;

    unsigned char *pixels = malloc((size_t)width * (size_t)height * RGBA_CHANNELS);
    if (pixels == NULL) {
        return false;
    }

    // Cada pixel novo é a média do bloco de pixels originais que ele cobre.
    // Assim todo pixel original entra na conta e pontinhos não somem ao acaso
    for (int y = 0; y < height; y++) {
        int sourceTop = (int)((long long)y * image->height / height);
        int sourceBottom = (int)((long long)(y + 1) * image->height / height);
        for (int x = 0; x < width; x++) {
            int sourceLeft = (int)((long long)x * image->width / width);
            int sourceRight = (int)((long long)(x + 1) * image->width / width);

            unsigned long sum[4] = {0, 0, 0, 0};
            for (int sy = sourceTop; sy < sourceBottom; sy++) {
                const unsigned char *pixel =
                    image->pixels + ((size_t)sy * image->width + sourceLeft) * RGBA_CHANNELS;
                for (int sx = sourceLeft; sx < sourceRight; sx++) {
                    for (int c = 0; c < RGBA_CHANNELS; c++) {
                        sum[c] += pixel[c];
                    }
                    pixel += RGBA_CHANNELS;
                }
            }

            unsigned long count = (unsigned long)(sourceBottom - sourceTop) *
                                  (unsigned long)(sourceRight - sourceLeft);
            unsigned char *target = pixels + ((size_t)y * width + x) * RGBA_CHANNELS;
            for (int c = 0; c < RGBA_CHANNELS; c++) {
                target[c] = (unsigned char)((sum[c] + count / 2) / count);
            }
        }
    }

    // O stb libera com free, então o buffer novo pode seguir pelo mesmo caminho
    stbi_image_free(image->pixels);
    image->pixels = pixels;
    image->width = width;
    image->height = height;
    return true;
}

const char *image_last_error(void)
{
    const char *reason = stbi_failure_reason();
    return reason != NULL ? reason : "erro desconhecido";
}

bool gray_image_alloc(GrayImage *image, int width, int height)
{
    image->width = 0;
    image->height = 0;
    image->pixels = NULL;

    // Evita estouro na multiplicação em wasm, onde size_t tem 32 bits
    if (width <= 0 || height <= 0 || (size_t)width > SIZE_MAX / (size_t)height) {
        return false;
    }
    image->pixels = malloc((size_t)width * (size_t)height);
    if (image->pixels == NULL) {
        return false;
    }
    image->width = width;
    image->height = height;
    return true;
}

void gray_image_free(GrayImage *image)
{
    if (image == NULL) {
        return;
    }
    free(image->pixels);
    image->pixels = NULL;
    image->width = 0;
    image->height = 0;
}

void rgba_to_gray(const RgbaImage *source, GrayImage *destination)
{
    size_t count = (size_t)source->width * (size_t)source->height;
    const unsigned char *rgba = source->pixels;

    // O alfa é ignorado, só a cor entra na conta
    for (size_t i = 0; i < count; i++) {
        double luma = LUMA_RED * rgba[0] + LUMA_GREEN * rgba[1] + LUMA_BLUE * rgba[2];
        // Os pesos somam 1, então o resultado já cabe em 0 a 255
        destination->pixels[i] = (unsigned char)(luma + 0.5);
        rgba += RGBA_CHANNELS;
    }
}
