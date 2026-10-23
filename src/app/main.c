// Callbacks do SDL3 em vez de laço próprio, pois no navegador o laço não pode
// bloquear a thread. Assim o mesmo código serve ao nativo e ao WebAssembly
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdlib.h>

#include "core/image.h"

static const char *WINDOW_TITLE = "Object Counter";
static const int WINDOW_WIDTH = 640;
static const int WINDOW_HEIGHT = 480;

// Mesmo tom da moldura da imagem na página
static const Uint8 BACKGROUND_RED = 0x0A;
static const Uint8 BACKGROUND_GREEN = 0x0A;
static const Uint8 BACKGROUND_BLUE = 0x0A;

static const int RGBA_BYTES_PER_PIXEL = 4;

// Folga para a janela não cobrir a tela inteira com fotos grandes
static const float SCREEN_FILL_RATIO = 0.9f;

typedef struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    RgbaImage image;
} AppState;

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

// Copia os pixels para uma textura e ajusta a janela à imagem
static bool show_image(AppState *state, const RgbaImage *image)
{
    SDL_Texture *texture = SDL_CreateTexture(state->renderer, SDL_PIXELFORMAT_RGBA32,
                                             SDL_TEXTUREACCESS_STATIC, image->width,
                                             image->height);
    if (texture == NULL) {
        SDL_Log("Falha ao criar a textura: %s", SDL_GetError());
        return false;
    }
    if (!SDL_UpdateTexture(texture, NULL, image->pixels, image->width * RGBA_BYTES_PER_PIXEL)) {
        SDL_Log("Falha ao enviar a imagem para a textura: %s", SDL_GetError());
        SDL_DestroyTexture(texture);
        return false;
    }

    if (state->texture != NULL) {
        SDL_DestroyTexture(state->texture);
    }
    state->texture = texture;

    // Desenhar em coordenadas da imagem deixa as marcações futuras alinhadas
    // em qualquer tamanho de janela
    SDL_SetRenderLogicalPresentation(state->renderer, image->width, image->height,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    fit_window_to_image(state->window, image->width, image->height);
    return true;
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

    if (!SDL_CreateWindowAndRenderer(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                                     &state->window, &state->renderer)) {
        SDL_Log("Falha ao criar a janela e o renderizador: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // No nativo a foto vem pela linha de comando, na web virá do upload
    if (argc > 1) {
        if (!rgba_image_load_from_file(&state->image, argv[1])) {
            SDL_Log("Falha ao carregar a imagem %s (%s)", argv[1], image_last_error());
            return SDL_APP_FAILURE;
        }
        SDL_Log("Imagem carregada com %d x %d pixels", state->image.width,
                state->image.height);
        if (!show_image(state, &state->image)) {
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
    // O SDL chama SDL_Quit sozinho depois deste callback
}
