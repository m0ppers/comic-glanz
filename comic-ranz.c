#include "comic-ranz.h"

#ifdef AMIGA
#include <clib/mathffp_protos.h>

#define AAABS SPAbs
#define AAFLOOR SPFloor
#define AAROUND(x) ((x - SPFloor(x)) >= 0.5f ? SPCeil(x) : SPFloor(x))
#else
#include "math.h"

#define AAABS fabs
#define AAFLOOR floor
#define AAROUND roundf
#endif

void plot(uint8_t *image, int x, int y, float a) {
  uint8_t blendColor = image[y * 320 + x];
  if (a <= 0.25f) {
    image[y * 320 + x] = blendColor;
  } else if (a <= 0.5f) {
    image[y * 320 + x] = blendColor + 32;
  } else if (a <= 0.75f) {
    image[y * 320 + x] = 1; //
  } else {
    // black
    image[y * 320 + x] = 33; // "true" black
  }
}

// // integer part of x
// function ipart(x) is
//     return floor(x)

// function round(x) is
//     return ipart(x + 0.5)

// // fractional part of x
// function fpart(x) is
//     return x - floor(x)

// function rfpart(x) is
//     return 1 - fpart(x)

// fractional part of x
float fpart(float x) { return x - AAFLOOR(x); }
float rfpart(float x) { return 1.0f - fpart(x); }

void draw_line(uint8_t *image, float x0, float y0, float x1, float y1) {
  bool steep = AAABS(y1 - y0) > AAABS(x1 - x0);

  if (steep) {
    float t = y0;
    y0 = x0;
    x0 = t;
    t = y1;
    y1 = x1;
    x1 = t;
  }
  if (x0 > x1) {
    float t = x1;
    x1 = x0;
    x0 = t;
    t = y1;
    y1 = y0;
    y0 = t;
  }

  float dx = x1 - x0;
  float dy = y1 - y0;
  float gradient = dy / dx;
  if (dx == 0.0f) {
    gradient = 1.0f;
  }
  // handle first endpoint
  float xend = AAROUND(x0);
  float yend = y0 + gradient * (xend - x0);
  float xgap = rfpart(x0 + 0.5f);
  int xpxl1 = (int)xend; // this will be used in the main loop
  int ypxl1 = (int)yend;
  if (steep) {
    plot(image, ypxl1, xpxl1, rfpart(yend) * xgap);
    plot(image, ypxl1 + 1, xpxl1, fpart(yend) * xgap);
  } else {
    plot(image, xpxl1, ypxl1, rfpart(yend) * xgap);
    plot(image, xpxl1, ypxl1 + 1, fpart(yend) * xgap);
  }
  float intery = yend + gradient; // first y-intersection for the main loop

  // handle second endpoint
  xend = AAROUND(x1);
  yend = y1 + gradient * (xend - x1);
  xgap = fpart(x1 + 0.5f);
  int xpxl2 = (int)xend; // this will be used in the main loop
  int ypxl2 = (int)yend;
  if (steep) {
    plot(image, ypxl2, xpxl2, rfpart(yend) * xgap);
    plot(image, ypxl2 + 1, xpxl2, fpart(yend) * xgap);
  } else {
    plot(image, xpxl2, ypxl2, rfpart(yend) * xgap);
    plot(image, xpxl2, ypxl2 + 1, fpart(yend) * xgap);
  }

  // main loop
  if (steep) {
    for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++) {
      plot(image, (int)intery, x, rfpart(intery));
      plot(image, (int)intery + 1, x, fpart(intery));
      intery = intery + gradient;
    }
  } else {
    for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++) {
      plot(image, x, (int)intery, rfpart(intery));
      plot(image, x, (int)intery + 1, fpart(intery));
      intery = intery + gradient;
    }
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

  draw_line(image, 0.f, 0.f, 200.f, 250.0f);
}