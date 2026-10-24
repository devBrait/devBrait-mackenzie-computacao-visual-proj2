#include "pipeline.h"

#include <string.h>

#include "filter.h"
#include "histogram.h"
#include "morphology.h"
#include "params.h"

bool pipeline_process(Pipeline *pipeline, const RgbaImage *source)
{
    memset(pipeline, 0, sizeof(*pipeline));

    if (!gray_image_alloc(&pipeline->gray, source->width, source->height) ||
        !gray_image_alloc(&pipeline->smoothed, source->width, source->height) ||
        !gray_image_alloc(&pipeline->binary, source->width, source->height) ||
        !gray_image_alloc(&pipeline->morphology, source->width, source->height)) {
        pipeline_free(pipeline);
        return false;
    }

    pipeline->width = source->width;
    pipeline->height = source->height;

    rgba_to_gray(source, &pipeline->gray);

    if (!gaussian_blur(&pipeline->gray, &pipeline->smoothed, GAUSSIAN_SIGMA)) {
        pipeline_free(pipeline);
        return false;
    }

    // Limiar calculado na imagem suavizada, que tem menos ruído no histograma
    size_t histogram[HISTOGRAM_LEVELS];

    compute_histogram(&pipeline->smoothed, histogram);
    pipeline->threshold = otsu_threshold(histogram);
    binarize(&pipeline->smoothed, &pipeline->binary, pipeline->threshold);
    pipeline->inverted = fix_polarity(&pipeline->binary);

    // Abre primeiro para a sujeira sumir antes que o fechamento a junte aos objetos
    size_t count = (size_t)source->width * (size_t)source->height;

    memcpy(pipeline->morphology.pixels, pipeline->binary.pixels, count);

    if (!morphology_open(&pipeline->morphology, OPENING_ITERATIONS) ||
        !morphology_close(&pipeline->morphology, CLOSING_ITERATIONS)) {
        pipeline_free(pipeline);
        return false;
    }

    if (!label_components(&pipeline->morphology, &pipeline->components)) {
        pipeline_free(pipeline);
        return false;
    }

    pipeline->labeledCount = pipeline->components.count;

    double noiseArea = NOISE_AREA_FRACTION * (double)count;
    double median = 0.0;

    remove_small_components(&pipeline->components, noiseArea);

    if (!median_component_area(&pipeline->components, &median)) {
        pipeline_free(pipeline);
        return false;
    }

    remove_small_components(&pipeline->components, MIN_AREA_MEDIAN_FACTOR * median);
    pipeline->objectCount = pipeline->components.count;

    return true;
}

void pipeline_free(Pipeline *pipeline)
{
    if (pipeline == NULL) {
        return;
    }

    gray_image_free(&pipeline->gray);
    gray_image_free(&pipeline->smoothed);
    gray_image_free(&pipeline->binary);
    gray_image_free(&pipeline->morphology);
    component_list_free(&pipeline->components);

    pipeline->width = 0;
    pipeline->height = 0;
    pipeline->threshold = 0;
    pipeline->inverted = false;
    pipeline->labeledCount = 0;
    pipeline->objectCount = 0;
}

const GrayImage *pipeline_stage_image(const Pipeline *pipeline, PipelineStage stage)
{
    switch (stage) {
    case STAGE_SMOOTHED:
        return &pipeline->smoothed;

    case STAGE_BINARY:
        return &pipeline->binary;

    case STAGE_MORPHOLOGY:
        return &pipeline->morphology;

    case STAGE_GRAY:
    default:
        return &pipeline->gray;
    }
}
