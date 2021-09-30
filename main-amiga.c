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

#define NUM_BITPLANES 6

#define TASK_PRIORITY 20

// clang-format off
#define DIWSTRT 0x08e
#define DIWSTOP 0x090
#define DDFSTRT 0x092
#define DDFSTOP 0x094
#define BPL1PTH 0x0e0
#define BPL1PTL 0x0e2
#define BPL2PTH 0x0e4
#define BPL2PTL 0x0e6
#define BPL3PTH 0x0e8
#define BPL3PTL 0x0ea
#define BPL4PTH 0x0ec
#define BPL4PTL 0x0ee
#define BPL5PTH 0x0f0
#define BPL5PTL 0x0f2
#define BPL6PTH 0x0f4
#define BPL6PTL 0x0f6
#define BPLCON0 0x100
#define BPL1MOD 0x108
#define BPL2MOD 0x10a
#define COLOR00 0x180
#define FMODE   0x1fc
// clang-format on

#define PRA_FIR0_BIT (1 << 6)

static uint8_t image[81920];

#define COPLIST_IDX_BPL_SETUP (17)

// clang-format off
static UWORD __chip coplist[] = {
  FMODE, 0x0, // AGA => OCS compat
  BPLCON0, 0x6200, // 6 bitplanes for EHB, Composite Color enable
  DIWSTRT, 0x2c81,
  DIWSTOP, 0x2cc1,
	DDFSTRT, 0x38,
	DDFSTOP, 0xd0,
  BPL1MOD, 0x0,
  BPL2MOD, 0x0,
  BPL1PTH, 0x0,
  BPL1PTL, 0x0,
  BPL2PTH, 0x0,
  BPL2PTL, 0x0,
  BPL3PTH, 0x0,
  BPL3PTL, 0x0,
  BPL4PTH, 0x0,
  BPL4PTL, 0x0,
  BPL5PTH, 0x0,
  BPL5PTL, 0x0,
  BPL6PTH, 0x0,
  BPL6PTL, 0x0,
  #include "palette_amiga.h"
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

  create_ranz(&image[0]);
  for (int i = 0; i < 81920; i++) {
    buffer[i] = 0;
  }

  int plane_size = 320 * 256 / 8;
  for (int i = 0; i < 81920; i++) {
    uint8_t b = 1 << (7 - i % 8);
    // walk through all planes and set bit by bit in every plane
    // STUMPF IST TRUMPF
    int plane_offset = 0;
    int bittest = 1;
    for (int p = 0; p < NUM_BITPLANES; p++) {
      if (image[i] & bittest) {
        buffer[i / 8 + plane_offset] |= b;
      }
      bittest *= 2;
      plane_offset += plane_size;
    }
  }
  ULONG addr = (ULONG)(&buffer);
  int offset = 0;
  for (int i = 0; i < NUM_BITPLANES; i++) {
    coplist[COPLIST_IDX_BPL_SETUP + offset] = (addr >> 16) & 0xffff;
    offset += 2;
    coplist[COPLIST_IDX_BPL_SETUP + offset] = (addr)&0xffff;
    offset += 2;
    addr += plane_size;
  }

  Forbid();
  SetTaskPri(FindTask(NULL), TASK_PRIORITY);
  init_display();

  // save old interrupt state
  uint16_t intenar = custom.intenar;
  // disable all interrupts
  custom.intena = 0x7fff;

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
