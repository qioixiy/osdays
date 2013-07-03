#ifndef _DSCTBL_H
#define _DSCTBL_H

//about GDT and IDT
struct SEGMENT_DESCRIPTOR {
  short limit_low, base_low;//limit:段的大小，base段的起始地址
  char base_mid, access_right;//access_right 段的管理属性
  char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
  short offset_low, selector;
  char dw_count, access_right;
  short offset_high;
};

#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e

void init_gdtidt(void);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR * gd, int offset, int selector, int ar);

#endif

