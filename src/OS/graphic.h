#ifndef _GARPHIC_H
#define _GARPHIC_H

//naskinfo.nas
void io_hlt(void);//��ͣϵͳ
void io_cli(void);//�ر��ж�
int io_in8(int port);
void io_out8(int port, int data);
int  io_load_eflags(void);//���ر�־λ
void io_store_eflags(int eflags);//��ԭ��־λ

//graphic.h
void init_palette(void);//�趨��ɫ��
void init_screen8(unsigned char *vram, int xsize, int ysize);//��ʼ������
void init_mouse_cursor8(char *mouse, char bc);//׼�����ָ�롾16��16��
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c,int x0, int y0, int x1, int y1);
void putblock8_8(char *vram, int vxsize, int pxsize,
		 int pysize, int px0, int py0, char *buf, int bxsize);
void putfont8(unsigned char *vram, //display a char
	      int xsize, int x, int y, 
	      char color, char *font);
void putfont8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *str);//put string

#define COL8_000000 0
#define COL8_FF0000 1
#define COL8_00FF00 2
#define COL8_FFFF00 3
#define COL8_0000FF 4
#define COL8_FF00FF 5
#define COL8_00FFFF 6
#define COL8_FFFFFF 7
#define COL8_C6C6C6 8
#define COL8_840000 9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15

#endif
