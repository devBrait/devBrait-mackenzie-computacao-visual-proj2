# Contador de Objetos

Projeto 2 de Computação Visual, Universidade Presbiteriana Mackenzie, 7º semestre.

Você envia a foto de alguns objetos sobre um fundo liso e o programa devolve quantos são, com uma caixa e um número em cima de cada um. Escrevemos todo o processamento em C, do zero e sem OpenCV. Ele roda no navegador, compilado para WebAssembly.

- **Versão online:** https://devbrait.github.io/devBrait-mackenzie-computacao-visual-proj2/
- **Repositório:** https://github.com/devBrait/devBrait-mackenzie-computacao-visual-proj2
- **Release:** https://github.com/devBrait/devBrait-mackenzie-computacao-visual-proj2/releases/tag/v1.0.0
- **Vídeo:** https://www.youtube.com/watch?v=nPJU1PNZEHY

## Integrantes

| Nome | RA |
|---|---|
| Enzo Ponte Gamberi | 10389931 |
| Luís Henrique Ribeiro Fernandes | 10420079 |
| Raphael Grizante da Silva | 10416979 |
| Vinícius Brait Lorimier | 10420046 |

## Como usar

1. Abra a versão online, baixe o `web.zip` da [release](https://github.com/devBrait/devBrait-mackenzie-computacao-visual-proj2/releases/tag/v1.0.0) ou compile com `make web && make serve` (veja [Como compilar](#como-compilar)). O `web.zip` precisa de um servidor local, por exemplo `python -m http.server 8000` na pasta do `index.html`.
2. Clique em **Escolher imagem** ou arraste uma foto PNG, JPG ou BMP para a página.
3. A aba **Resultado** mostra a foto com cada objeto marcado e numerado, e o painel mostra a contagem total.
4. As abas **Cinza**, **Suavizada**, **Binária** e **Morfologia** mostram cada etapa do processamento, para ver em qual delas um resultado errado começou.
5. **Remover imagem** volta ao estado inicial.

O console do navegador (F12) mostra o limiar escolhido, o contraste e quantos componentes cada filtro descartou. Na versão nativa essas linhas saem no terminal.

## Pipeline

Cada etapa mora num arquivo de `src/core/`, escrito em C puro e sem nenhuma dependência do SDL.

| Etapa | Arquivo | O que faz |
|---|---|---|
| 1. Carregamento | `image.c` | Decodifica a foto com o stb_image em RGBA |
| 2. Redução | `image.c` | Fotos com lado maior que 1024 px são reduzidas por média de blocos. Assim os parâmetros em pixels valem igual para qualquer foto |
| 3. Cinza | `image.c` | Luminância Rec. 709 |
| 4. Suavização | `filter.c` | Gaussiano separável e borda replicada |
| 5. Limiar | `histogram.c` | Histograma de 256 níveis e método de Otsu, que escolhe o limiar de maior variância entre as classes |
| 6. Contraste mínimo | `histogram.c` | Se a diferença entre as médias de fundo e objeto for menor que 30, a imagem não tem objetos (evita contar o ruído de uma foto toda preta ou toda branca) |
| 7. Polaridade | `histogram.c` | O fundo é a cor que ocupa a maior parte da borda da imagem. Se ele for branco, a imagem é invertida para os objetos ficarem brancos |
| 8. Morfologia | `morphology.c` | Abertura (apaga pontinhos) e fechamento (tapa furos), com elemento 3x3 e 2 passadas cada |
| 9. Rotulagem | `labeling.c` | Busca em largura com vizinhança 8. Cada componente guarda área, caixa e centroide |
| 10. Filtro de área | `labeling.c` | Descarta componentes menores que 0,02% da imagem e depois os menores que 0,25 × a área mediana |
| 11. Peças encostadas | `labeling.c` | Veja no próximo tópico |
| 12. Marcação | `src/app/main.c` | Desenha caixa e número de cada objeto com o SDL3 |

### Peças encostadas

1. A **área de referência** de uma peça é a mediana das áreas dos componentes com cara de peça sozinha, os que ficam entre 0,5 e 1,5 vezes a mediana geral.
2. Um componente com mais de **1,75 ×** essa área pode ser um grupo de peças encostadas.
3. Esse componente é recortado e **erodido** um número de vezes igual a 0,4 × o raio de uma peça. Peças encostadas se ligam por um "pescoço" fino, que some antes das próprias peças.
4. Cada pedaço que sobra conta como uma peça. Se sobrar um pedaço só, é um objeto grande inteiro (um parafuso no meio de porcas, por exemplo) e conta 1.

Um grupo aparece na imagem com a faixa de números das peças dele, por exemplo `7-8`.

## Parâmetros

Os valores ajustáveis estão todos em `src/core/params.h`, cada um com um comentário sobre o efeito dele. Para testar outro valor, edite o arquivo e rode o `make` de novo.

## Resultados

As fotos de teste estão em `images/`. O número no fim de cada nome é a contagem real, feita por nós olhando a foto.

| Imagem | Real | Contado |
|---|---|---|
| `cafe_16.jpg` | 16 | 16 |
| `comprimidos_4.jpg` | 4 | 4 |
| `parafusos_10.jpg` | 10 | 10 |
| `remedios_9.jpg` | 9 | 9 |
| `remedios_retos_9.jpg` | 9 | 9 |
| `fundo_preto_0.jpg` | 0 | 0 |
| `fundo_branco_0.png` | 0 | 0 |

As 7 imagens saíram com a contagem exata.

### Limitações

Numa foto de cápsulas com sombra dura, testada durante o desenvolvimento, o programa contou 8 objetos onde havia 12. O pipeline separa fundo e objeto só pelo brilho, então a sombra escura entra na mesma mancha da peça e acaba grudando vizinhas. Com luz difusa isso some.

Cor clara sobre fundo claro também atrapalha, e rosa claro em papel branco, por exemplo, perde parte da área na binarização. Peças empilhadas escondem área, e a erosão não consegue separá-las. Em PNG recortado, os pixels transparentes guardam uma cor qualquer por baixo da transparência, e o programa lê essa cor como ruído. Já um objeto encostado na moldura pode inverter a polaridade se ocupar a maior parte da borda.

Para tirar uma boa foto de teste, fotografe de cima, com luz difusa e fundo liso, escolha objetos que contrastem em brilho com o fundo e deixe pelo menos um dedo de distância entre eles.

## Como compilar

Desenvolvemos no Windows, com MSYS2 e emsdk. Os comandos `make` rodam no terminal **MSYS2 MINGW64**.

### Dependências

1. Instale o [MSYS2](https://www.msys2.org/) e, no terminal MINGW64, rode:
   ```
   pacman -S make mingw-w64-x86_64-gcc mingw-w64-x86_64-sdl3 mingw-w64-x86_64-pkgconf
   ```
2. Instale o [emsdk](https://emscripten.org/docs/getting_started/downloads.html) em `C:\emsdk`, com a versão 6.0.13. Rode estes no PowerShell ou no cmd, porque o MSYS2 apaga as barras invertidas de `C:\emsdk`:
   ```
   git clone https://github.com/emscripten-core/emsdk.git C:\emsdk
   cd C:\emsdk
   emsdk install 6.0.13
   emsdk activate 6.0.13
   ```
   Se o emsdk estiver em outra pasta, passe o caminho no make, por exemplo `make web EMSDK=/d/emsdk`.

### Alvos do Makefile

| Comando | O que faz |
|---|---|
| `make native` | Executável nativo em `build/native/counter.exe`, útil para depurar |
| `make web` | Versão WebAssembly em `build/web/` |
| `make serve` | Servidor local em `http://127.0.0.1:8000` |
| `make clean` | Apaga a pasta `build/` |

### Problemas conhecidos

Se o navegador forçar HTTPS no `localhost`, abra `http://127.0.0.1:8000`. Depois de recompilar, recarregue com Ctrl+F5, senão a página pode continuar com o CSS antigo.

### Versão nativa

```
make native
./build/native/counter.exe images/cafe_16.jpg
```

As teclas 1 a 5 trocam a etapa exibida (Cinza, Suavizada, Binária, Morfologia e Resultado).

## Estrutura

```
src/core/      processamento, C puro sem SDL
  params.h       todas as constantes
  image.c        carregamento, redução e cinza
  filter.c       Gaussiano
  histogram.c    histograma, Otsu, contraste, binarização e polaridade
  morphology.c   erosão, dilatação, abertura e fechamento
  labeling.c     rotulagem, filtros de área e peças encostadas
  pipeline.c     executa todas as etapas e guarda o resultado de cada uma
src/app/
  main.c         janela SDL3, textura, marcações e números
  web_api.c      funções chamadas pela página
web/           página (index.html, style.css, app.js)
third_party/   stb_image.h
images/        fotos de teste
```

Nativo e web usam o mesmo `main.c`. Ele é escrito com os callbacks do SDL3 (`SDL_MAIN_USE_CALLBACKS`), já que o navegador não deixa um laço infinito travar a página. Cada push na `main` dispara o GitHub Actions, que compila a versão web e publica no GitHub Pages.

## Referências

- GONZALEZ, R. C.; WOODS, R. E. **Processamento Digital de Imagens**. 3. ed. São Paulo: Pearson Prentice Hall, 2010.
- OTSU, N. A Threshold Selection Method from Gray-Level Histograms. **IEEE Transactions on Systems, Man, and Cybernetics**, v. 9, n. 1, p. 62-66, 1979.
- WU, K.; OTOO, E.; SUZUKI, K. Optimizing Two-Pass Connected-Component Labeling Algorithms. **Pattern Analysis and Applications**, v. 12, n. 2, p. 117-135, 2009.
- ITU-R. **Recommendation BT.709**: Parameter values for the HDTV standards for production and international programme exchange.
- BARRETT, S. **stb_image**. Disponível em: https://github.com/nothings/stb
- HUTTON, T. **sdl-canvas-wasm**: minimal example of animating the HTML5 canvas from C++ using SDL through WebAssembly. Disponível em: https://github.com/timhutton/sdl-canvas-wasm
- NISSAN, S. **sdl-wasm**: compiling C/SDL to WebAssembly and binding it to an HTML5 canvas. Disponível em: https://github.com/shlomnissan/sdl-wasm
- **SDL3 Wiki**. Disponível em: https://wiki.libsdl.org/SDL3
- **Emscripten Documentation**. Disponível em: https://emscripten.org/docs
