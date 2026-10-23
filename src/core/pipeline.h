#ifndef CORE_PIPELINE_H
#define CORE_PIPELINE_H

#include <stdbool.h>

#include "image.h"

// Guarda o resultado de cada etapa para a interface poder exibir qualquer uma
typedef struct Pipeline {
    int width;
    int height;
    GrayImage gray;
} Pipeline;

// Aloca os buffers e roda todas as etapas sobre a imagem de origem.
// Em caso de falha o pipeline fica vazio
bool pipeline_process(Pipeline *pipeline, const RgbaImage *source);
void pipeline_free(Pipeline *pipeline);

#endif
