#ifndef CORE_PARAMS_H
#define CORE_PARAMS_H

// Todos os valores ajustáveis do processamento ficam aqui.
// Para testar outro valor basta editar e recompilar

// Lado maior da imagem de trabalho em pixels. Fotos maiores são reduzidas
// para que os parâmetros em pixels tenham o mesmo efeito em qualquer foto
#define WORK_MAX_SIDE 1024

// Desvio padrão do Gaussiano em pixels da imagem de trabalho. Maior apaga
// mais textura do fundo, mas também arredonda as bordas dos objetos
#define GAUSSIAN_SIGMA 1.5f

// Passadas 3x3 de cada operação morfológica. A abertura apaga sujeira de
// até uns 2 pixels de raio e o fechamento tapa furos do mesmo tamanho
#define OPENING_ITERATIONS 2
#define CLOSING_ITERATIONS 2

// Componentes menores que esta fração da imagem são ruído e saem antes
// da mediana, senão muita sujeira puxaria a mediana para baixo
#define NOISE_AREA_FRACTION 0.0002

// Depois do ruído, saem os menores que esta fração da área mediana
#define MIN_AREA_MEDIAN_FACTOR 0.25

// Peça isolada é a que tem entre 0,5 e 1,5 vezes a área mediana. A mediana
// delas é a área de referência de uma peça
#define ISOLATED_MIN_FACTOR 0.5
#define ISOLATED_MAX_FACTOR 1.5

// Componentes maiores que este fator vezes a referência são peças encostadas
#define TOUCHING_FACTOR 1.5

// Cor das caixas e da etiqueta do número na aba Resultado, azul claro #38BDF8
#define MARK_COLOR_RED 0x38
#define MARK_COLOR_GREEN 0xBD
#define MARK_COLOR_BLUE 0xF8

// Espessura da caixa e altura do número em fração do lado maior da imagem,
// assim as marcações ficam do mesmo tamanho na tela em qualquer foto
#define MARK_LINE_FRACTION 0.003
#define MARK_TEXT_FRACTION 0.025

#endif
