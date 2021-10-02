#include "comic-ranz.h"

#include "ranz.h"

#include <stdlib.h>
#ifdef AMIGA
#define roundf(x) (x)
#else
#include "math.h"
#include "stdio.h"
#endif

void plot(uint8_t *image, int16_t x, int16_t y, uint8_t v) {
  int32_t x32 = (int32_t)x;
  int32_t y32 = (int32_t)y;

  int32_t index = y32 * 320 + x32;

  image[index] = v;
}

// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#Algorithm_for_integer_arithmetic
void draw_line(uint8_t *image, int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  // printf("DRAWING LINE %d,%d -> %d,%d\n", x0, y0, x1, y1);
  int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int16_t err = dx + dy, e2; /* error value e_xy */

  for (;;) { /* loop */
    plot(image, x0, y0, 0);
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
  long xx = x0 - x1, yy = y0 - y1, xy;         /* relative values for checks */
  double dx, dy, err, cur = xx * sy - yy * sx; /* curvature */

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
    dx = 4.0 * sy * cur * (x1 - x0) + xx - xy; /* differences 1st degree */
    dy = 4.0 * sx * cur * (y0 - y1) + yy - xy;
    xx += xx;
    yy += yy;
    err = dx + dy + xy; /* error 1st step */
    do {
      plot(image, x0, y0, 0); /* plot curve */
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

void create_ranz(uint8_t *image) {
  int index = 0;
  for (int y = 0; y < 256; ++y) {
    for (int x = 0; x < 320; ++x) {
      // image[index++] = 0;
      image[index++] = y / 8;
    }
  }
  // draw_line(image, 50, 50, 100, 50);
  // draw_line(image, 100, 60, 50, 60);
  // draw_line(image, 50, 70, 100, 75);
  // draw_line(image, 50, 85, 100, 80);
  // draw_line(image, 50, 50, 100, 100);

  uint8_t *start = (uint8_t *)&ranz_bin;
  uint8_t *pos = start;
  float x = 0.f;
  float y = 0.f;
  while (pos != (start + ranz_bin_len)) {
    unsigned char instruction = *pos;
    printf("instruction: %c\n", instruction);

    if (instruction == 'M') {
      pos++;
      x = *(float *)pos;
      pos += 4;
      y = *(float *)pos;
      pos += 4;
      printf("MMMM %f,%f\n", x, y);

      // plot(image, (int16_t)x, (int16_t)y, 0);
    } else if (instruction == 'm') {
      pos++;
      x = *(float *)pos + x;
      pos += 4;
      y = *(float *)pos + y;
      pos += 4;
      printf("MMMM %f,%f\n", x, y);

      // plot(image, (int16_t)x, (int16_t)y, 0);
    } else if (instruction == 'q') {
      pos++;
      uint8_t numqs = *(uint8_t *)pos;
      pos++;
      for (uint8_t i = 0; i < numqs; i++) {
        float cxf = *(float *)pos + x;
        int16_t cx = (int16_t)roundf(cxf);
        pos += 4;
        float cyf = *(float *)pos + y;
        int16_t cy = (int16_t)roundf(cyf);
        pos += 4;
        float dxf = *(float *)pos + x;
        int16_t dx = (int16_t)roundf(dxf);
        pos += 4;
        float dyf = *(float *)pos + y;
        int16_t dy = (int16_t)roundf(dyf);
        pos += 4;
        // printf("%f %f Q %f %f %f %f\n", x, y, cxf, cyf, dxf, dyf);
        // draw_line(image, (int16_t)roundf(x), (int16_t)roundf(y), dx, dy);
        draw_quadratic_bezier(image, (int16_t)roundf(x), (int16_t)roundf(y), cx,
                              cy, dx, dy);
        // plot(image, (int16_t)dx, (int16_t)dy, 0);
        x = dxf;
        y = dyf;
      }
    } else if (instruction == 'l') {
      pos++;
      float dxf = *(float *)pos + x;
      int16_t dx = (int16_t)roundf(dxf);
      pos += 4;
      float dyf = *(float *)pos + y;
      int16_t dy = (int16_t)roundf(dyf);
      pos += 4;
      draw_line(image, (int16_t)roundf(x), (int16_t)roundf(y), dx, dy);
      x = dxf;
      y = dyf;
    } else if (instruction == 'L') {
      pos++;
      float dxf = *(float *)pos;
      int16_t dx = (int16_t)roundf(dxf);
      pos += 4;
      float dyf = *(float *)pos;
      int16_t dy = (int16_t)roundf(dyf);
      pos += 4;
      draw_line(image, (int16_t)roundf(x), (int16_t)roundf(y), dx, dy);
      x = dxf;
      y = dyf;
    } else {
      printf("Kaputt! %c %d\n", instruction, pos - start);
      return;
    }
  }

  // draw_quadratic_bezier(image, 52, 57, -4, -6, -4)
}