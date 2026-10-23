#include "pipeline.h"

#include <string.h>

bool pipeline_process(Pipeline *pipeline, const RgbaImage *source)
{
    memset(pipeline, 0, sizeof(*pipeline));

    if (!gray_image_alloc(&pipeline->gray, source->width, source->height)) {
        pipeline_free(pipeline);
        return false;
    }
    pipeline->width = source->width;
    pipeline->height = source->height;

    rgba_to_gray(source, &pipeline->gray);
    return true;
}

void pipeline_free(Pipeline *pipeline)
{
    if (pipeline == NULL) {
        return;
    }
    gray_image_free(&pipeline->gray);
    pipeline->width = 0;
    pipeline->height = 0;
}
