#include "histogram.h"

static const unsigned char WHITE = 255;
static const unsigned char BLACK = 0;

void compute_histogram(const GrayImage *image, size_t histogram[HISTOGRAM_LEVELS])
{
    size_t count = (size_t)image->width * (size_t)image->height;

    for (int level = 0; level < HISTOGRAM_LEVELS; level++) {
        histogram[level] = 0;
    }

    for (size_t i = 0; i < count; i++) {
        histogram[image->pixels[i]]++;
    }
}

int otsu_threshold(const size_t histogram[HISTOGRAM_LEVELS])
{
    double total = 0.0;
    double totalSum = 0.0;

    for (int level = 0; level < HISTOGRAM_LEVELS; level++) {
        total += (double)histogram[level];
        totalSum += (double)level * (double)histogram[level];
    }

    // Peso e soma do fundo crescem a cada limiar testado, assim cada teste
    // custa só algumas contas em vez de percorrer o histograma de novo
    double backgroundWeight = 0.0;
    double backgroundSum = 0.0;
    double bestVariance = -1.0;
    int bestThreshold = 0;

    for (int level = 0; level < HISTOGRAM_LEVELS; level++) {
        backgroundWeight += (double)histogram[level];
        backgroundSum += (double)level * (double)histogram[level];

        double foregroundWeight = total - backgroundWeight;

        if (backgroundWeight == 0.0) {
            continue;
        }

        // Daqui em diante todo pixel ficaria no fundo
        if (foregroundWeight == 0.0) {
            break;
        }

        double backgroundMean = backgroundSum / backgroundWeight;
        double foregroundMean = (totalSum - backgroundSum) / foregroundWeight;
        double difference = backgroundMean - foregroundMean;

        // Variância entre as classes, quanto maior mais separadas elas estão
        double variance = backgroundWeight * foregroundWeight * difference * difference;

        if (variance > bestVariance) {
            bestVariance = variance;
            bestThreshold = level;
        }
    }

    return bestThreshold;
}

void binarize(const GrayImage *source, GrayImage *destination, int threshold)
{
    size_t count = (size_t)source->width * (size_t)source->height;

    for (size_t i = 0; i < count; i++) {
        destination->pixels[i] = source->pixels[i] > threshold ? WHITE : BLACK;
    }
}

bool fix_polarity(GrayImage *binary)
{
    int width = binary->width;
    int height = binary->height;
    size_t borderPixels = 0;
    size_t whiteBorderPixels = 0;

    // Linha de cima e linha de baixo inteiras
    for (int x = 0; x < width; x++) {
        whiteBorderPixels += binary->pixels[x] == WHITE;
        borderPixels++;

        if (height > 1) {
            whiteBorderPixels += binary->pixels[(size_t)(height - 1) * width + x] == WHITE;
            borderPixels++;
        }
    }

    // Colunas laterais sem os cantos, que já foram contados
    for (int y = 1; y < height - 1; y++) {
        const unsigned char *row = binary->pixels + (size_t)y * width;

        whiteBorderPixels += row[0] == WHITE;
        borderPixels++;

        if (width > 1) {
            whiteBorderPixels += row[width - 1] == WHITE;
            borderPixels++;
        }
    }

    // O fundo encosta na moldura e os objetos normalmente não
    if (whiteBorderPixels * 2 <= borderPixels) {
        return false;
    }

    size_t count = (size_t)width * (size_t)height;

    for (size_t i = 0; i < count; i++) {
        binary->pixels[i] = binary->pixels[i] == WHITE ? BLACK : WHITE;
    }

    return true;
}
