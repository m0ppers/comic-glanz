#include "comic-ranz.h"

#include "ranz.h"

#ifndef AMIGA
#include "stdio.h"
#endif

#define BLUR_RADIUS 1

static uint8_t temp[81920];

void plot(uint8_t *image, int16_t x, int16_t y) {
  int32_t x32 = (int32_t)x;
  int32_t y32 = (int32_t)y;

  int32_t index = y32 * 320 + x32;

  image[index] = 255;
}

// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#Algorithm_for_integer_arithmetic
void draw_line(uint8_t *image, int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  // printf("DRAWING LINE %d,%d -> %d,%d\n", x0, y0, x1, y1);
  int16_t dx = (x1 - x0);
  if (dx < 0) {
    dx = -dx;
  }
  int16_t dy = y1 - y0;
  if (dy > 0) {
    dy = -dy;
  }
  int16_t sx = x0 < x1 ? 1 : -1;
  int16_t sy = y0 < y1 ? 1 : -1;
  int16_t err = dx + dy, e2; /* error value e_xy */

  for (;;) { /* loop */
    plot(image, x0, y0);
    if (x0 == x1 && y0 == y1)
      break;
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    } /* e_xy+e_x > 0 */
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    } /* e_xy+e_y < 0 */
  }
}

// http://members.chello.at/easyfilter/bresenham.html
void draw_quadratic_bezier(uint8_t *image, int16_t x0, int16_t y0, int16_t x1,
                           int16_t y1, int16_t x2, int16_t y2) {
  int16_t sx = x2 - x1, sy = y2 - y1;
  long xx = x0 - x1, yy = y0 - y1, xy;          /* relative values for checks */
  int32_t dx, dy, err, cur = xx * sy - yy * sx; /* curvature */

  // assert(xx * sx <= 0 && yy * sy <= 0); /* sign of gradient must not change
  // */

  if (sx * (long)sx + sy * (long)sy >
      xx * xx + yy * yy) { /* begin with longer part */
    x2 = x0;
    x0 = sx + x1;
    y2 = y0;
    y0 = sy + y1;
    cur = -cur; /* swap P0 P2 */
  }
  if (cur != 0) { /* no straight line */
    xx += sx;
    xx *= sx = x0 < x2 ? 1 : -1; /* x step direction */
    yy += sy;
    yy *= sy = y0 < y2 ? 1 : -1; /* y step direction */
    xy = 2 * xx * yy;
    xx *= xx;
    yy *= yy;                /* differences 2nd degree */
    if (cur * sx * sy < 0) { /* negated curvature? */
      xx = -xx;
      yy = -yy;
      xy = -xy;
      cur = -cur;
    }
    dx = 4 * sy * cur * (x1 - x0) + xx - xy; /* differences 1st degree */
    dy = 4 * sx * cur * (y0 - y1) + yy - xy;
    xx += xx;
    yy += yy;
    err = dx + dy + xy; /* error 1st step */
    do {
      plot(image, x0, y0); /* plot curve */
      if (x0 == x2 && y0 == y2)
        return;          /* last pixel -> curve finished */
      y1 = 2 * err < dx; /* save value for test of y step */
      if (2 * err > dy) {
        x0 += sx;
        dx -= xy;
        err += dy += yy;
      } /* x step */
      if (y1) {
        y0 += sy;
        dy -= xy;
        err += dx += xx;
      }                /* y step */
    } while (dy < dx); /* gradient negates -> algorithm fails */
  }
  draw_line(image, x0, y0, x2, y2); /* plot remaining part to end */
}

// https://blackpawn.com/texts/blur/default.html
void blur_h(uint8_t *source, uint8_t *dest) {
  // ignore edge of image so loop is easier (no content there anyway)
  int index = 321;
  for (int y = 1; y < 255; y++) {
    for (int x = 1; x < 319; x++) {
      int16_t total = 0;
      for (int kx = -BLUR_RADIUS; kx <= BLUR_RADIUS; kx++) {
        total += source[index + kx];
      }
      dest[index] = total / 3;
      index++;
    }
    index += 2;
  }
}

void blur_v(uint8_t *source, uint8_t *dest) {
  int index = 321;
  for (int y = 1; y < 255; y++) {
    for (int x = 1; x < 319; x++) {
      int16_t total = 0;
      int blur_index = index - 320;
      for (int ky = -BLUR_RADIUS; ky <= BLUR_RADIUS; ky++) {
        total += source[blur_index];
        blur_index += 320;
      }
      dest[index] = total / 3;
      // dest[index] = source[index];
      index++;
    }
    index += 2;
  }
}

void blur(uint8_t *image) {
  for (int i = 0; i < 81920; i++) {
    temp[i] = 0;
  }
  blur_h(image, &temp[0]);
  blur_v(&temp[0], image);
}

void create_ranz(uint8_t *image) {
  int16_t x, dx = 0;
  int16_t y, dy = 0;

  uint8_t *start = (uint8_t *)&ranz_bin;
  uint8_t *pos = start;
  while (pos != (start + ranz_bin_len)) {
    unsigned char instruction = *(pos++);
    if (instruction == 'm') {
      x = *((int8_t *)pos++) + x;
      y = *((int8_t *)pos++) + y;
      // printf("M %d %d\n", x, y);
    } else if (instruction == 'q') {
      uint8_t numqs = *(pos++);
      for (uint8_t i = 0; i < numqs; i++) {
        int16_t cx = x + *((int8_t *)pos++);
        int16_t cy = y + *((int8_t *)pos++);
        dx = x + *((int8_t *)pos++);
        dy = y + *((int8_t *)pos++);
        draw_quadratic_bezier(image, x, y, cx, cy, dx, dy);
        x = dx;
        y = dy;
      }
    } else if (instruction == 'l') {
      uint8_t numls = *(pos++);
      for (uint8_t i = 0; i < numls; i++) {
        dx = *((int8_t *)pos++) + x;
        dy = *((int8_t *)pos++) + y;
        draw_line(image, x, y, dx, dy);
        x = dx;
        y = dy;
      }
    } else if (instruction == 'h') {
      uint8_t numhs = *(pos++);
      for (uint8_t i = 0; i < numhs; i++) {
        dx = *((int8_t *)pos++) + x;
        draw_line(image, x, y, dx, y);
        x = dx;
      }
    } else if (instruction == 'v') {
      uint8_t numvs = *(pos++);
      for (uint8_t i = 0; i < numvs; i++) {
        dy = *((int8_t *)pos++) + y;
        // printf("V %d %d %d\n", x, y, dy);
        draw_line(image, x, y, x, dy);
        y = dy;
      }
    } else {
      // printf("Kaputt! %c %d\n", instruction, pos - start);
      return;
    }
  }

  blur(image);

  int index = 0;
  for (y = 0; y < 256; y++) {
    uint8_t palette_index = 2;
    uint8_t num = 0;
    uint8_t step = 10;
    uint8_t draw_white = 0;
    for (x = 0; x < 320; x++) {
      if (image[index] > 0) {
        if (draw_white) {
          draw_white = 0;
        } else {
          draw_white = 1;
        }
        if (image[index] > 100) {
          image[index] = 32;
        } else if (image[index] > 80) {
          image[index] = 0;
        } else if (image[index] > 40) {
          image[index] = palette_index + 32;
        } else {
          image[index] = palette_index;
        }
      } else if (draw_white) {
        image[index] = 1;
      } else {
        image[index] = palette_index;
      }
      num++;
      if (num == step) {
        palette_index++;
        if (palette_index > 5 && palette_index < 26) {
          step = 11;
        } else {
          step = 10;
        }
        num = 0;
      }
      index++;
    }
  }
}