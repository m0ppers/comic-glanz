#include "comic-ranz.h"

void plot(uint8_t *image, int16_t x, int16_t y, uint8_t v) {
  int32_t x32 = (int32_t)x;
  int32_t y32 = (int32_t)y;

  int32_t index = y32 * 320 + x32;

  image[index] = v;
}

// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#Algorithm_for_integer_arithmetic
void draw_line(uint8_t *image, int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  int16_t dx = x1 - x0;
  int16_t dy = y1 - y0;
  int16_t D = 2 * dy - dx;
  int16_t y = y0;

  for (int16_t x = x0; x <= x1; x++) {
    plot(image, x, y, 0);
    if (D > 0) {
      y = y + 1;
      D = D - 2 * dx;
    }
    D = D + 2 * dy;
  }
}

void create_ranz(uint8_t *image) {
  int index = 0;
  for (int y = 0; y < 256; ++y) {
    for (int x = 0; x < 320; ++x) {
      // image[index++] = 0;
      image[index++] = y / 8;
    }
  }

  draw_line(image, 00, 30, 319, 80);
}