# make native   executável local para depurar
# make web      WebAssembly da entrega
# make serve    servidor local da versão web
# make clean    apaga a pasta build
# Rodar sempre pelo terminal MSYS2 MINGW64

APP_NAME = counter

SOURCES = src/app/main.c

BUILD_DIR = build
NATIVE_DIR = $(BUILD_DIR)/native
WEB_DIR = $(BUILD_DIR)/web

CC = gcc

# O emsdk_env.sh falha no MSYS2, então apontamos direto para o emcc
# Outro caminho pode ser passado assim make web EMSDK=/d/emsdk
EMSDK ?= /c/emsdk
EMCC ?= $(EMSDK)/upstream/emscripten/emcc

COMMON_FLAGS = -std=c11 -Wall -Wextra -O2

# pkg-config evita caminhos fixos do SDL3 de cada máquina
NATIVE_CFLAGS = $(COMMON_FLAGS) $(shell pkg-config --cflags sdl3)
# Sem -mwindows para o console mostrar as mensagens de erro
NATIVE_LIBS = $(filter-out -mwindows,$(shell pkg-config --libs sdl3)) -lm

# O emcc baixa e compila o port do SDL3 na primeira vez e guarda em cache
WEB_FLAGS = $(COMMON_FLAGS) --use-port=sdl3

PORT = 8000

.PHONY: all native web serve clean

all: native

native: $(NATIVE_DIR)/$(APP_NAME)

$(NATIVE_DIR)/$(APP_NAME): $(SOURCES) | $(NATIVE_DIR)
	$(CC) $(NATIVE_CFLAGS) $(SOURCES) -o $@ $(NATIVE_LIBS)

web: $(WEB_DIR)/index.html

$(WEB_DIR)/index.html: $(SOURCES) | $(WEB_DIR)
	$(EMCC) $(WEB_FLAGS) $(SOURCES) -o $@

# O navegador não carrega WebAssembly por file://
serve: web
	python -m http.server $(PORT) --directory $(WEB_DIR)

$(NATIVE_DIR) $(WEB_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
