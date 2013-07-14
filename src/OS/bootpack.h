#ifndef _BOOTPACK_H
#define _BOOTPACK_H

#include "naskfunc.h"
#include "graphic.h"
#include "dsctbl.h"
#include "int.h"
#include "fifo.h"
#include "keyboard.h"
#include "mouse.h"
#include "memory.h"
#include "sheet.h"
#include "window.h"
#include "timer.h"
#include "mtask.h"

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx,scrny;
  char *vram;
};

//file info,定义一个文件的信息
struct FILEINFO {
  unsigned char name[8], ext[3], type;
  char reserve[10];
  unsigned short time, date, clustno;//clustno代表从哪个扇区开始存放文件,磁盘镜像中的地址=clustno * 512 + 0x003e00
  unsigned int size;
};

#define ADR_BOOTINFO 0X00000FF0
#define ADR_DISKIMG 0X00100000

#define KEYCMD_LED 0XED

#endif
