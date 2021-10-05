#include "comic-ranz.h"

#include "ranz.h"

#ifdef AMIGA
#define FETCH_I16(pos) *(int16_t *)pos
#else
#define FETCH_I16(pos)                                                         \
  ((int16_t)((*(uint16_t *)pos >> 8) | (*(uint16_t *)pos << 8)))
#include "stdio.h"
#endif

void plot(uint8_t *image, int16_t x, int16_t y, uint8_t v) {
  int32_t x32 = (int32_t)x;
  int32_t y32 = (int32_t)y;

  int32_t index = y32 * 320 + x32;

  image[index] = v;
}

// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#Algorithm_for_integer_arithmetic
void draw_line(uint8_t *image, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
               uint8_t color, int do_mod) {
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
    if (do_mod) {
      plot(image, x0 % 320, y0, color);
    } else {
      plot(image, x0, y0, color);
    }
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
      plot(image, x0, y0, 32); /* plot curve */
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
  draw_line(image, x0, y0, x2, y2, 32, 0); /* plot remaining part to end */
}

void create_ranz(uint8_t *image) {
  int index = 0;
  int16_t x = 0;
  int16_t y = 0;

  for (index = 2; index < 32; index++) {
    int16_t add;
    if (index > 5 && index < 26) {
      add = 11;
    } else {
      add = 10;
    }
    for (int i = 0; i < add; i++) {
      draw_line(image, x + i, 255, x + i + 20, 0, index, 1);
    }
    x += add;
  }

  // draw_line(image, 50, 50, 100, 50);
  // draw_line(image, 100, 60, 50, 60);
  // draw_line(image, 50, 70, 100, 75);
  // draw_line(image, 50, 85, 100, 80);
  // draw_line(image, 50, 50, 100, 100);

  // draw_quadratic_bezier(image, 50, 50, 0, 70, 80, 80);
  // return;

  uint8_t *start = (uint8_t *)&ranz_bin;
  uint8_t *pos = start;
  x = 0;
  y = 0;
  while (pos != (start + ranz_bin_len)) {
    unsigned char instruction = *pos;
    // printf("GEIL %c\n", instruction);
    // if (pos > start + 9) {
    //   return;
    // }
    if (instruction == 'm') {
      pos++;
      x = *((int8_t *)pos++) + x;
      y = *((int8_t *)pos++) + y;
      // printf("M %d %d\n", x, y);
    } else if (instruction == 'q') {
      pos++;
      uint8_t numqs = *(uint8_t *)pos;
      pos++;
      for (uint8_t i = 0; i < numqs; i++) {
        int16_t cx = x + *((int8_t *)pos++);
        int16_t cy = y + *((int8_t *)pos++);
        int16_t dx = x + *((int8_t *)pos++);
        int16_t dy = y + *((int8_t *)pos++);
        // printf("%d %d Q %d %d %d %d\n", x, y, cx, cy, dx, dy);
        // draw_line(image, (int16_t)roundf(x), (int16_t)roundf(y), dx, dy);
        draw_quadratic_bezier(image, x, y, cx, cy, dx, dy);
        // draw_line(image, x, y, dx, dy, 32, 0);
        x = dx;
        y = dy;
      }
    } else if (instruction == 'l') {
      pos++;
      uint8_t numls = *(uint8_t *)pos;
      pos++;
      for (uint8_t i = 0; i < numls; i++) {
        int16_t dx = *((int8_t *)pos++) + x;
        int16_t dy = *((int8_t *)pos++) + y;
        // printf("L %d %d %d %d\n", x, y, dx, dy);
        draw_line(image, x, y, dx, dy, 32, 0);
        x = dx;
        y = dy;
      }
    } else if (instruction == 'h') {
      pos++;
      uint8_t numhs = *(uint8_t *)pos;
      pos++;
      for (uint8_t i = 0; i < numhs; i++) {
        int16_t dx = *((int8_t *)pos++) + x;
        draw_line(image, x, y, dx, y, 32, 0);
        // printf("H %d %d %d\n", x, y, dx);
        x = dx;
      }
    } else if (instruction == 'v') {
      pos++;
      uint8_t numvs = *(uint8_t *)pos;
      pos++;
      for (uint8_t i = 0; i < numvs; i++) {
        int16_t dy = *((int8_t *)pos++) + y;
        // printf("V %d %d %d\n", x, y, dy);
        draw_line(image, x, y, x, dy, 32, 0);
        y = dy;
      }
    } else {
      // printf("Kaputt! %c %d\n", instruction, pos - start);
      return;
    }
  }

  index = 0;
  for (y = 0; y < 256; ++y) {
    int draw_white = 0;
    for (x = 0; x < 320; ++x) {
      if (image[index] == 32) {
        if (draw_white) {
          draw_white = 0;
        } else {
          draw_white = 1;
        }
      } else if (draw_white) {
        image[index] = 1;
      }
      index++;
    }
  }

  // draw_quadratic_bezier(image, 52, 57, -4, -6, -4)
}