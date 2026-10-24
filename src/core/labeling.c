#include "labeling.h"

#include <stdlib.h>

static const unsigned char WHITE = 255;
static const int INITIAL_CAPACITY = 16;

// Garante espaço para mais um componente dobrando a capacidade,
// assim o realloc roda poucas vezes
static bool reserve_component(ComponentList *list, int *capacity)
{
    if (list->count < *capacity) {
        return true;
    }

    int newCapacity = *capacity == 0 ? INITIAL_CAPACITY : *capacity * 2;
    Component *items = realloc(list->items, sizeof(Component) * (size_t)newCapacity);

    if (items == NULL) {
        return false;
    }

    list->items = items;
    *capacity = newCapacity;

    return true;
}

// Busca em largura a partir de start. Cada pixel é marcado ao entrar na fila,
// então entra uma vez só e a fila nunca passa do tamanho da imagem
static Component flood_component(const GrayImage *binary, unsigned char *visited, int *queue,
                                 int start)
{
    int width = binary->width;
    int height = binary->height;
    Component component = {0, width, height, -1, -1, 0.0, 0.0};
    double sumX = 0.0;
    double sumY = 0.0;
    int head = 0;
    int tail = 0;

    queue[tail++] = start;
    visited[start] = 1;

    while (head < tail) {
        int index = queue[head++];
        int x = index % width;
        int y = index / width;

        component.area++;
        sumX += x;
        sumY += y;

        if (x < component.minX) {
            component.minX = x;
        }

        if (x > component.maxX) {
            component.maxX = x;
        }

        if (y < component.minY) {
            component.minY = y;
        }

        if (y > component.maxY) {
            component.maxY = y;
        }

        // Os 8 vizinhos, assim peças ligadas só pela diagonal ficam juntas
        for (int dy = -1; dy <= 1; dy++) {
            int neighborY = y + dy;

            if (neighborY < 0 || neighborY >= height) {
                continue;
            }

            for (int dx = -1; dx <= 1; dx++) {
                int neighborX = x + dx;

                if (neighborX < 0 || neighborX >= width) {
                    continue;
                }

                int neighbor = neighborY * width + neighborX;

                if (!visited[neighbor] && binary->pixels[neighbor] == WHITE) {
                    visited[neighbor] = 1;
                    queue[tail++] = neighbor;
                }
            }
        }
    }

    component.centroidX = sumX / component.area;
    component.centroidY = sumY / component.area;

    return component;
}

bool label_components(const GrayImage *binary, ComponentList *list)
{
    list->items = NULL;
    list->count = 0;

    // Índices em int bastam, a imagem de trabalho tem no máximo WORK_MAX_SIDE de lado
    int count = binary->width * binary->height;
    unsigned char *visited = calloc((size_t)count, 1);
    int *queue = malloc(sizeof(int) * (size_t)count);
    int capacity = 0;

    if (visited == NULL || queue == NULL) {
        free(visited);
        free(queue);
        return false;
    }

    for (int i = 0; i < count; i++) {
        if (visited[i] || binary->pixels[i] != WHITE) {
            continue;
        }

        if (!reserve_component(list, &capacity)) {
            free(visited);
            free(queue);
            component_list_free(list);
            return false;
        }

        list->items[list->count] = flood_component(binary, visited, queue, i);
        list->count++;
    }

    free(visited);
    free(queue);

    return true;
}

void component_list_free(ComponentList *list)
{
    if (list == NULL) {
        return;
    }

    free(list->items);
    list->items = NULL;
    list->count = 0;
}

void remove_small_components(ComponentList *list, double minArea)
{
    int kept = 0;

    // Mantém a ordem, que é a ordem de leitura da imagem
    for (int i = 0; i < list->count; i++) {
        if (list->items[i].area >= minArea) {
            list->items[kept] = list->items[i];
            kept++;
        }
    }

    list->count = kept;
}

static int compare_ints(const void *a, const void *b)
{
    int first = *(const int *)a;
    int second = *(const int *)b;

    return (first > second) - (first < second);
}

bool median_component_area(const ComponentList *list, double *median)
{
    *median = 0.0;

    if (list->count == 0) {
        return true;
    }

    // Ordena uma cópia das áreas para não mexer na ordem da lista
    int *areas = malloc(sizeof(int) * (size_t)list->count);

    if (areas == NULL) {
        return false;
    }

    for (int i = 0; i < list->count; i++) {
        areas[i] = list->items[i].area;
    }

    qsort(areas, (size_t)list->count, sizeof(int), compare_ints);

    int middle = list->count / 2;

    if (list->count % 2 == 1) {
        *median = areas[middle];
    } else {
        *median = (areas[middle - 1] + areas[middle]) / 2.0;
    }

    free(areas);

    return true;
}
