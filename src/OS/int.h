#ifndef _INT_H
#define _INT_H

//GDT°¢IDT
#define ADR_IDT		0x0026f800
#define LIMIT_IDT	0x000007ff
#define ADR_GDT		0x00270000
#define LIMIT_GDT	0x0000ffff
#define ADR_BOTPAK	0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e

//÷–∂œIO…Ë÷√
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

//KEYBOARD BUFFER
struct KEYBUF{
  unsigned char data[32];
  int next_r, next_w, len;
};

/* int.h */
void init_pic(void);
int inthandler0d(int *esp);
void inthandler20(int *esp);
void inthandler21(int *esp);
void inthandler27(int *esp);
void inthandler2c(int *esp);
//naskfunc.nas
void asm_inthandler0d(void);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

extern struct FIFO32 *keyfifo;
extern struct FIFO32 *mousefifo;
extern unsigned int keydata0;
extern unsigned int mousedata0;

#endif
