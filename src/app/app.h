#ifndef APP_APP_H
#define APP_APP_H

#include <stdbool.h>
#include <stddef.h>

// Decodifica os bytes de um arquivo e passa a exibir essa imagem
bool app_load_image_from_memory(const unsigned char *data, size_t size);

#endif
