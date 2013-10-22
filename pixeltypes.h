#ifndef PIXELTYPES
#define PIXELTYPES
#include <stdint.h>
#include <stdlib.h>
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} pixel_t;
typedef struct  {
    pixel_t *pixels;
    size_t width;
    size_t height;
} bitmap_t;
#endif