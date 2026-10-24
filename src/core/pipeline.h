#ifndef CORE_PIPELINE_H
#define CORE_PIPELINE_H

#include <stdbool.h>

#include "image.h"

// Mesma ordem das abas da página e das teclas 1 a 5 do nativo
typedef enum PipelineStage {
    STAGE_GRAY,
    STAGE_SMOOTHED,
    STAGE_BINARY,
    STAGE_MORPHOLOGY,
    STAGE_RESULT,
    STAGE_COUNT
} PipelineStage;

// Guarda o resultado de cada etapa para a interface poder exibir qualquer uma
typedef struct Pipeline {
    int width;
    int height;
    GrayImage gray;
    GrayImage smoothed;
    GrayImage binary;

    // Guardados para o console mostrar o que o Otsu decidiu
    int threshold;
    bool inverted;
} Pipeline;

// Aloca os buffers e roda todas as etapas sobre a imagem de origem.
// Em caso de falha o pipeline fica vazio
bool pipeline_process(Pipeline *pipeline, const RgbaImage *source);
void pipeline_free(Pipeline *pipeline);

// Imagem em tons de cinza de uma etapa. Etapas ainda não implementadas
// devolvem o cinza
const GrayImage *pipeline_stage_image(const Pipeline *pipeline, PipelineStage stage);

#endif
