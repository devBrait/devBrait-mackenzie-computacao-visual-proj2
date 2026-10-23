#ifndef CORE_IMAGE_H
#define CORE_IMAGE_H

#include <stdbool.h>
#include <stddef.h>

// Sempre 4 bytes por pixel na ordem R, G, B, A
typedef struct RgbaImage {
    int width;
    int height;
    unsigned char *pixels;
} RgbaImage;

// Um byte por pixel, 0 é preto e 255 é branco
typedef struct GrayImage {
    int width;
    int height;
    unsigned char *pixels;
} GrayImage;

// Na web os bytes do arquivo chegam do JS já em memória
bool rgba_image_load_from_memory(RgbaImage *image, const unsigned char *data, size_t size);
bool rgba_image_load_from_file(RgbaImage *image, const char *path);
void rgba_image_free(RgbaImage *image);

// Reduz a imagem no lugar até o lado maior caber em maxSide.
// Imagens menores ficam como estão
bool rgba_image_limit_size(RgbaImage *image, int maxSide);

// Motivo da última falha de carregamento
const char *image_last_error(void);

bool gray_image_alloc(GrayImage *image, int width, int height);
void gray_image_free(GrayImage *image);

// O destino precisa ter o mesmo tamanho da origem
void rgba_to_gray(const RgbaImage *source, GrayImage *destination);

#endif
