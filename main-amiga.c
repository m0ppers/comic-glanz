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

// clang-format off
#define DIWSTRT 0x08e
#define DIWSTOP 0x090
#define DDFSTRT 0x092
#define DDFSTOP 0x094
#define BPL1PTH 0x0e0
#define BPL1PTL 0x0e2
#define BPLCON0 0x100
#define BPL1MOD 0x108
#define BPL2MOD 0x10a
#define COLOR00 0x180
#define FMODE   0x1fc
// clang-format on

#define PRA_FIR0_BIT (1 << 6)
#define BPLCON0_COMPOSITE_COLOR (1 << 9)

static uint16_t palette[32];
static uint8_t image[81920];

#define COPLIST_IDX_BPL_SETUP (17)

// clang-format off
static UWORD __chip coplist[] = {
  FMODE, 0x0, // AGA => OCS compat
  BPLCON0, 0x1200,
  DIWSTRT, 0x2c81,
  DIWSTOP, 0x2cc1,
	DDFSTRT, 0x38,
	DDFSTOP, 0xd0,
  BPL1MOD, 0x0,
  BPL2MOD, 0x0,
  BPL1PTH, 0x0,
  BPL1PTL, 0x0,
  // palette
  0x180, 0xa0f,
  0x182, 0x000,
  0xffff, 0xfffe
};
// clang-format on

static uint8_t __chip buffer[81920];

#define PRA_FIR0_BIT (1 << 6)

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

  ULONG addr = (ULONG)(&buffer);
  coplist[COPLIST_IDX_BPL_SETUP] = (addr >> 16) & 0xffff;
  coplist[COPLIST_IDX_BPL_SETUP + 2] = (addr)&0xffff;

  custom.cop1lc = (ULONG)coplist;
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
