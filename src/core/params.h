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

#endif
