// Callbacks do SDL3 em vez de laço próprio, pois no navegador o laço não pode
// bloquear a thread. Assim o mesmo código serve ao nativo e ao WebAssembly
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdlib.h>

static const char *WINDOW_TITLE = "Object Counter";
static const int WINDOW_WIDTH = 640;
static const int WINDOW_HEIGHT = 480;

// Cores da paleta da interface
static const Uint8 BACKGROUND_RED = 0x0A;
static const Uint8 BACKGROUND_GREEN = 0x0A;
static const Uint8 BACKGROUND_BLUE = 0x0A;
static const Uint8 ACCENT_RED = 0x00;
static const Uint8 ACCENT_GREEN = 0x70;
static const Uint8 ACCENT_BLUE = 0xF3;

static const float RECTANGLE_SCALE = 0.5f;

typedef struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
} AppState;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

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

    // Centralizado para confirmar que toda a área de desenho aparece
    float rectangleWidth = WINDOW_WIDTH * RECTANGLE_SCALE;
    float rectangleHeight = WINDOW_HEIGHT * RECTANGLE_SCALE;
    SDL_FRect rectangle = {
        (WINDOW_WIDTH - rectangleWidth) / 2.0f,
        (WINDOW_HEIGHT - rectangleHeight) / 2.0f,
        rectangleWidth,
        rectangleHeight,
    };
    SDL_SetRenderDrawColor(state->renderer, ACCENT_RED, ACCENT_GREEN, ACCENT_BLUE,
                           SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(state->renderer, &rectangle);

    SDL_RenderPresent(state->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)result;

    AppState *state = appstate;
    if (state != NULL) {
        // O renderizador depende da janela, por isso é destruído antes
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
