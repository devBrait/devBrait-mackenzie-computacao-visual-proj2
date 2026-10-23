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

// Na web os bytes do arquivo chegam do JS já em memória
bool rgba_image_load_from_memory(RgbaImage *image, const unsigned char *data, size_t size);
bool rgba_image_load_from_file(RgbaImage *image, const char *path);
void rgba_image_free(RgbaImage *image);

// Motivo da última falha de carregamento
const char *image_last_error(void);

#endif
