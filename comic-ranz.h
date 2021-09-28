#pragma once

#include <stdbool.h>
#include <stdint.h>

void draw_line(uint8_t *image, float x0, float y0, float x1, float y1);
void create_palette(uint16_t *palette);
void create_ranz(uint8_t *image);