// Callbacks do SDL3 em vez de laço próprio, pois no navegador o laço não pode
// bloquear a thread. Assim o mesmo código serve ao nativo e ao WebAssembly
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdlib.h>

#include "app/app.h"
#include "core/image.h"
#include "core/params.h"
#include "core/pipeline.h"

static const char *WINDOW_TITLE = "Object Counter";
static const int WINDOW_WIDTH = 640;
static const int WINDOW_HEIGHT = 480;

// Mesmo tom da moldura da imagem na página
static const Uint8 BACKGROUND_RED = 0x0A;
static const Uint8 BACKGROUND_GREEN = 0x0A;
static const Uint8 BACKGROUND_BLUE = 0x0A;

static const int RGBA_BYTES_PER_PIXEL = 4;

// Nomes para o console, na ordem de PipelineStage
static const char *STAGE_NAMES[STAGE_COUNT] = {
    "Cinza", "Suavizada", "Binária", "Morfologia", "Resultado",
};

// A página abre com a aba Resultado ativa
static const PipelineStage INITIAL_STAGE = STAGE_RESULT;

typedef struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    RgbaImage image;
    Pipeline pipeline;
    PipelineStage stage;
} AppState;

// Usado pelas funções chamadas da página, que não recebem o estado
static AppState *appState = NULL;

#ifndef __EMSCRIPTEN__
// Folga para a janela não cobrir a tela inteira com fotos grandes
static const float SCREEN_FILL_RATIO = 0.9f;

// Reduz a janela proporcionalmente se a foto for maior que a tela
static void fit_window_to_image(SDL_Window *window, int width, int height)
{
    float scale = 1.0f;
    SDL_Rect bounds;
    if (SDL_GetDisplayUsableBounds(SDL_GetDisplayForWindow(window), &bounds)) {
        float scaleX = bounds.w * SCREEN_FILL_RATIO / width;
        float scaleY = bounds.h * SCREEN_FILL_RATIO / height;
        scale = SDL_min(1.0f, SDL_min(scaleX, scaleY));
    }
    SDL_SetWindowSize(window, (int)(width * scale), (int)(height * scale));
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}
#endif

// Streaming permite reescrever os pixels quando a etapa exibida mudar
static SDL_Texture *create_texture(SDL_Renderer *renderer, int width, int height)
{
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);
    if (texture == NULL) {
        SDL_Log("Falha ao criar a textura: %s", SDL_GetError());
    }
    return texture;
}

// A textura não tem formato cinza, então cada valor vira R, G e B iguais
static bool upload_gray(SDL_Texture *texture, const GrayImage *gray)
{
    void *pixels = NULL;
    int pitch = 0;
    if (!SDL_LockTexture(texture, NULL, &pixels, &pitch)) {
        SDL_Log("Falha ao acessar a textura: %s", SDL_GetError());
        return false;
    }

    for (int y = 0; y < gray->height; y++) {
        // O pitch pode ser maior que a largura vezes 4 por alinhamento
        Uint8 *row = (Uint8 *)pixels + (size_t)y * (size_t)pitch;
        const unsigned char *source = gray->pixels + (size_t)y * (size_t)gray->width;
        for (int x = 0; x < gray->width; x++) {
            Uint8 *pixel = row + (size_t)x * RGBA_BYTES_PER_PIXEL;
            pixel[0] = source[x];
            pixel[1] = source[x];
            pixel[2] = source[x];
            pixel[3] = SDL_ALPHA_OPAQUE;
        }
    }

    SDL_UnlockTexture(texture);
    return true;
}

// Desenhar em coordenadas da imagem deixa as marcações futuras alinhadas
// em qualquer tamanho de janela
static void resize_display(AppState *state, int width, int height)
{
    SDL_SetRenderLogicalPresentation(state->renderer, width, height,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
#ifdef __EMSCRIPTEN__
    // Na web o canvas guarda o tamanho real e o CSS reduz a exibição
    SDL_SetWindowSize(state->window, width, height);
#else
    fit_window_to_image(state->window, width, height);
#endif
}

// Processa e prepara tudo antes de mexer no estado, assim uma falha
// mantém a imagem anterior na tela
static bool replace_image(AppState *state, RgbaImage *loaded)
{
    int originalWidth = loaded->width;
    int originalHeight = loaded->height;
    if (!rgba_image_limit_size(loaded, WORK_MAX_SIDE)) {
        SDL_Log("Memória insuficiente para reduzir a imagem.");
        rgba_image_free(loaded);
        return false;
    }

    Pipeline pipeline;
    if (!pipeline_process(&pipeline, loaded)) {
        SDL_Log("Memória insuficiente para processar a imagem.");
        rgba_image_free(loaded);
        return false;
    }

    SDL_Texture *texture = create_texture(state->renderer, loaded->width, loaded->height);
    if (texture == NULL ||
        !upload_gray(texture, pipeline_stage_image(&pipeline, state->stage))) {
        if (texture != NULL) {
            SDL_DestroyTexture(texture);
        }
        pipeline_free(&pipeline);
        rgba_image_free(loaded);
        return false;
    }

    if (state->texture != NULL) {
        SDL_DestroyTexture(state->texture);
    }
    pipeline_free(&state->pipeline);
    rgba_image_free(&state->image);
    state->texture = texture;
    state->pipeline = pipeline;
    state->image = *loaded;

    resize_display(state, state->image.width, state->image.height);
    SDL_Log("Imagem carregada com %d x %d pixels, processada em %d x %d", originalWidth,
            originalHeight, state->image.width, state->image.height);
    SDL_Log("Limiar de Otsu %d, fundo %s", state->pipeline.threshold,
            state->pipeline.inverted ? "claro (imagem invertida)" : "escuro");
    SDL_Log("Componentes rotulados %d, descartados por área %d, objetos contados %d",
            state->pipeline.labeledCount,
            state->pipeline.labeledCount - state->pipeline.objectCount,
            state->pipeline.objectCount);
    return true;
}

bool app_set_display_stage(int stage)
{
    if (appState == NULL || stage < 0 || stage >= STAGE_COUNT) {
        return false;
    }

    appState->stage = (PipelineStage)stage;
    SDL_Log("Etapa exibida %s", STAGE_NAMES[stage]);

    // Sem imagem só guarda a escolha, que vale para o próximo upload
    if (appState->texture == NULL) {
        return true;
    }

    return upload_gray(appState->texture,
                       pipeline_stage_image(&appState->pipeline, appState->stage));
}

bool app_load_image_from_memory(const unsigned char *data, size_t size)
{
    if (appState == NULL) {
        SDL_Log("A aplicação ainda não terminou de iniciar.");
        return false;
    }

    RgbaImage loaded;
    if (!rgba_image_load_from_memory(&loaded, data, size)) {
        SDL_Log("Falha ao decodificar a imagem (%s)", image_last_error());
        return false;
    }
    return replace_image(appState, &loaded);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    // No navegador o SDL bloquearia as teclas da página toda e os campos
    // do painel não receberiam texto. No nativo a dica é ignorada
    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Falha ao inicializar o SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // calloc zera os ponteiros, então SDL_AppQuit libera com segurança
    // mesmo se a inicialização falhar no meio
    AppState *state = calloc(1, sizeof(AppState));
    if (state == NULL) {
        SDL_Log("Falha ao alocar o estado da aplicação.");
        return SDL_APP_FAILURE;
    }
    *appstate = state;
    appState = state;
    state->stage = INITIAL_STAGE;

    if (!SDL_CreateWindowAndRenderer(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                                     &state->window, &state->renderer)) {
        SDL_Log("Falha ao criar a janela e o renderizador: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // No nativo a foto vem pela linha de comando, na web virá do upload
    if (argc > 1) {
        RgbaImage loaded;
        if (!rgba_image_load_from_file(&loaded, argv[1])) {
            SDL_Log("Falha ao carregar a imagem %s (%s)", argv[1], image_last_error());
            return SDL_APP_FAILURE;
        }
        if (!replace_image(state, &loaded)) {
            return SDL_APP_FAILURE;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    (void)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

#ifndef __EMSCRIPTEN__
    // Na web quem troca a etapa são as abas, assim a aba ativa não fica errada
    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key >= SDLK_1 &&
        event->key.key < SDLK_1 + STAGE_COUNT) {
        app_set_display_stage((int)(event->key.key - SDLK_1));
    }
#endif

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    AppState *state = appstate;

    SDL_SetRenderDrawColor(state->renderer, BACKGROUND_RED, BACKGROUND_GREEN,
                           BACKGROUND_BLUE, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(state->renderer);

    if (state->texture != NULL) {
        SDL_RenderTexture(state->renderer, state->texture, NULL, NULL);
    }

    SDL_RenderPresent(state->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)result;

    AppState *state = appstate;
    if (state != NULL) {
        pipeline_free(&state->pipeline);
        rgba_image_free(&state->image);
        // A textura depende do renderizador, que depende da janela
        if (state->texture != NULL) {
            SDL_DestroyTexture(state->texture);
        }
        if (state->renderer != NULL) {
            SDL_DestroyRenderer(state->renderer);
        }
        if (state->window != NULL) {
            SDL_DestroyWindow(state->window);
        }
        free(state);
    }
    appState = NULL;
    // O SDL chama SDL_Quit sozinho depois deste callback
}
