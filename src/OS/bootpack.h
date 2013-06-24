#ifndef _BOOTPACK_H
#define _BOOTPACK_H

#include "graphic.h"
#include "dsctbl.h"
#include "int.h"

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx,scrny;
  char *vram;
};
#define ADR_BOOTINFO 0X00000FF0

#endif
