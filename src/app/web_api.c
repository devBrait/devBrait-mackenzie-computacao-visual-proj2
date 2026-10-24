// Funções chamadas pelo app.js, compiladas só na versão web
#include <emscripten/emscripten.h>

#include "app/app.h"

// O JS copia o arquivo para a memória do wasm e passa o endereço
EMSCRIPTEN_KEEPALIVE int api_load_image(const unsigned char *data, int size)
{
    if (size < 0) {
        return 0;
    }
    return app_load_image_from_memory(data, (size_t)size) ? 1 : 0;
}

// Recebe o índice da aba clicada, na mesma ordem de PipelineStage
EMSCRIPTEN_KEEPALIVE int api_set_display_stage(int stage)
{
    return app_set_display_stage(stage) ? 1 : 0;
}

// Lido pelo JS depois de cada upload para mostrar a contagem na página
EMSCRIPTEN_KEEPALIVE int api_get_count(void)
{
    return app_get_count();
}
