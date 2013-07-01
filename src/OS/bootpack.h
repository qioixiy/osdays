#ifndef _BOOTPACK_H
#define _BOOTPACK_H

#include "graphic.h"
#include "dsctbl.h"
#include "int.h"
#include "keyboard.h"
#include "mouse.h"

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx,scrny;
  char *vram;
};
#define ADR_BOOTINFO 0X00000FF0

#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

#endif
