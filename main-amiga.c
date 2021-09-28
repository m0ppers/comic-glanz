#include "comic-ranz.h"

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <hardware/custom.h>

struct Custom custom;
struct ExecBase *SysBase;
struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;

// void __nocommandline(){}; /* Disable commandline parsing  */
// void __initlibraries(){}; /* Disable auto-library-opening */

/*
 * declare memory mapped chip areas as volatile to ensure
 * the compiler does not optimize away the access.
 */
volatile UBYTE *ciaa_pra = (volatile UBYTE *)0xbfe001;
volatile UBYTE *custom_vhposr = (volatile UBYTE *)0xdff006;

#define TASK_PRIORITY 20
#define BPLCON0 0x100
#define COLOR00 0x180
#define FMODE 0x1fc
#define PRA_FIR0_BIT (1 << 6)
#define BPLCON0_COMPOSITE_COLOR (1 << 9)

static uint16_t palette[32];
static uint8_t image[81920];

// clang-format off
// static UWORD __chip coplist[128] = {
//   FMODE, 0x0, // AGA => OCS compat
//   BPLCON0, 0x200,
//   COLOR00, 0xf00,
//   0xffff, 0xfffe
// };
// clang-format on

static uint8_t __chip buffer[81920];

#define PRA_FIR0_BIT (1 << 6)
#define COLOR_WHITE (0xfff)
#define COLOR_RED (0xf00)
#define COLOR_WBENCH_BG (0x05a)
#define TOP_POS (0x40)
#define BOTTOM_POS (0xf0)

void init_display(void) {
  LoadView(NULL); // clear display, reset hardware registers
  WaitTOF();      // 2 WaitTOFs to wait for 1. long frame and
  WaitTOF();      // 2. short frame copper lists to finish (if interlaced)
}

void reset_display(void) {
  LoadView(GfxBase->ActiView);
  WaitTOF();
  WaitTOF();
  custom.cop1lc = (ULONG)GfxBase->copinit;
  RethinkDisplay();
}

int main(int argc, char **argv) {
  SysBase = *((struct ExecBase **)4UL);
  GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L);
  if (!GfxBase) {
    return 1;
  }
  IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
  if (!IntuitionBase) {
    return 1;
  }
  // MathBase = (struct MathBase*) OpenLibrary("math.library", 0);
  // if (!MathBase) {
  //   return 1;
  // }
  for (int i = 0; i < 81920; i++) {
    buffer[i] = 0xff;
  }

  create_palette(&palette[0]);
  create_ranz(&image[0]);

  Forbid();
  SetTaskPri(FindTask(NULL), TASK_PRIORITY);
  init_display();

  // save old interrupt state
  uint16_t intenar = custom.intenar;
  // disable all interrupts
  custom.intena = 0x7fff;
  custom.fmode = 0x0;
  custom.bplcon0 = 0x1200;
  custom.bplpt[0] = &buffer[0];

  for (int i = 0; i < 32; i++) {
    custom.color[i] = palette[i];
  }
  // custom.cop1lc = (ULONG) coplist;
  while ((*ciaa_pra & PRA_FIR0_BIT) != 0) {
  }
  reset_display();
  // reset old state
  custom.intena = intenar | 0xc000;
  Permit();

  CloseLibrary((struct Library *)IntuitionBase);
  CloseLibrary((struct Library *)GfxBase);
  return 0;
}
