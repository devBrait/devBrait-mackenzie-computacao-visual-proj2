#ifndef CORE_LABELING_H
#define CORE_LABELING_H

#include <stdbool.h>

#include "image.h"

// Um objeto candidato, ou seja, um grupo de pixels brancos conectados
typedef struct Component {
    int area;

    // Caixa que envolve o componente, com os limites incluídos
    int minX;
    int minY;
    int maxX;
    int maxY;

    double centroidX;
    double centroidY;

    // Quantas peças o componente representa, mais de 1 quando estão encostadas
    int pieces;
} Component;

typedef struct ComponentList {
    Component *items;
    int count;
} ComponentList;

// Encontra os componentes brancos com vizinhança 8.
// Em caso de falha a lista fica vazia
bool label_components(const GrayImage *binary, ComponentList *list);
void component_list_free(ComponentList *list);

// Tira da lista os componentes com área menor que minArea
void remove_small_components(ComponentList *list, double minArea);

// Mediana das áreas, ou zero se a lista estiver vazia
bool median_component_area(const ComponentList *list, double *median);

// Estima quantas peças cada componente tem pela área de uma peça isolada.
// A referência é a mediana dos componentes entre isolatedMin e isolatedMax
// vezes a mediana geral. Acima de touchingFactor vezes a referência, o
// componente vale round(área / referência) peças
bool estimate_touching_pieces(ComponentList *list, double isolatedMin, double isolatedMax,
                              double touchingFactor, double *referenceArea, int *total);

#endif
