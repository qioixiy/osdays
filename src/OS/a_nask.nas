[FORMAT "WCOFF"]	;生成对象文件的格式
[INSTRSET "i486p"]	;表示使用I486兼容指令集
[BITS 32] 		;生成32模式的指令
[FILE "a_nask.nas"]	;源文件名信息

      GLOBAL _api_putchar
      GLOBAL _api_end
      GLOBAL _api_putstr0
      GLOBAL _api_openwin
      GLOBAL _api_putstrwin
      GLOBAL _api_boxfilwin
;C语言支持
[SECTION .text]
_api_putchar:	;void api_putchar(int c);
      MOV EDX, 1
      MOV AL, [ESP+4]	;c
      INT 0X40
      RET
	
_api_end:	;void api_end(void);
      MOV EDX, 4
      INT 0X40     
	
_api_putstr0:	;void api_putstr0(char *s)
	PUSH EBX
	MOV EDX, 2
	MOV EBX, [ESP+8]	;s
	INT 0X40
	POP EBX
	RET

_api_openwin:	;int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title)
	PUSH EDI
	PUSH ESI
	PUSH EBX
	MOV EDX, 5
	MOV EBX, [ESP+16]	;buf
	MOV ESI, [ESP+20]	;xsize
	MOV EDI, [ESP+24]	;ysize
	MOV EAX, [ESP+28]	;col_inv
	MOV ECX, [ESP+32]	;title
	
	INT 0X40
	POP EBX
	POP ESI
	POP EDI
	RET	

_api_putstrwin:	;int api_putstrwin(int win, int x, int y, int col, int len, char *str)
	PUSH EDI
	PUSH ESI
	PUSH EBP
	PUSH EBX
	MOV EDX, 6
	MOV EBX, [ESP+20]	;win
	MOV ESI, [ESP+24]	;x
	MOV EDI, [ESP+28]	;y
	MOV EAX, [ESP+32]	;col
	MOV ECX, [ESP+36]	;len
	MOV EBP, [ESP+40]	;&str
	
	INT 0X40
	POP EBX
	POP EBP
	POP ESI
	POP EDI
	RET	

_api_boxfilwin:	;int api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col)
	PUSH EDI
	PUSH ESI
	PUSH EBP
	PUSH EBX
	MOV EDX, 7
	MOV EBX, [ESP+20]	;win
	MOV EAX, [ESP+24]	;x0
	MOV ECX, [ESP+28]	;y0
	MOV ESI, [ESP+32]	;x1
	MOV EDI, [ESP+36]	;y1
	MOV EBP, [ESP+40]	;col
	
	INT 0X40
	POP EBX
	POP EBP
	POP ESI
	POP EDI
	RET	
	
;malloc start
	GLOBAL _api_initmalloc
	GLOBAL _api_malloc
	GLOBAL _api_free
_api_initmalloc:	;void api_initmalloc(void)
	PUSH EBX
	MOV EDX, 8
	MOV EBX, [CS:0X0020]	;malloc内存空间的地址
	MOV EAX, EBX
	ADD EAX, 32*1024	;加上32KB
	MOV ECX, [CS:0X0000]	;数据段的大小
	SUB ECX, EAX
	INT 0X40
	POP EBX
	RET
	
_api_malloc:		;char *api_malloc(int size)
	PUSH EBX
	MOV EDX, 9
	MOV EBX, [CS:0X0020]
	MOV ECX, [ESP+8]	;size
	INT 0X40
	POP EBX
	RET

_api_free:		;void api_free(char *addr, int size)
	PUSH EBX
	MOV EDX, 10
	MOV EBX, [CS:0X0020]
	MOV EAX, [ESP+8]	;addr	
	MOV ECX, [ESP+12]	;size
	INT 0X40
	POP EBX
	RET
;malloc end

	GLOBAL _api_point
_api_point:		;void api_point(int win, int x, int y, int col)
	PUSH EDI
	PUSH ESI
	PUSH EBX
	MOV EDX, 11	
	MOV EBX, [ESP+16];win
	MOV ESI, [ESP+20];x
	MOV EDI, [ESP+24];y
	MOV EAX, [ESP+28];col
	INT 0X40
	POP EBX
	POP ESI
	POP EDI
	RET

	GLOBAL _api_refreshwin
_api_refreshwin:	;void api_refreshwin(int win, int x0, int y0, int x1, int y1)
	PUSH EDI
	PUSH ESI
	PUSH EBX
	MOV EDX, 12	
	MOV EBX, [ESP+16];win
	MOV EAX, [ESP+20];x0
	MOV ECX, [ESP+24];y0
	MOV ESI, [ESP+28];x1
	MOV EDI, [ESP+32];Y1
	INT 0X40
	POP EBX
	POP ESI
	POP EDI
	RET

	GLOBAL _api_linewin
_api_linewin:		;void api_linewin(int win, int x0, int y0, int x1, int y1, int col)
	PUSH EDI
	PUSH ESI
	PUSH EBP
	PUSH EBX
	MOV EDX, 13	
	MOV EBX, [ESP+20];win
	MOV EAX, [ESP+24];x0
	MOV ECX, [ESP+28];y0
	MOV ESI, [ESP+32];x1
	MOV EDI, [ESP+36];y1
	MOV EBP, [ESP+40];col
	INT 0X40
	POP EBX
	POP EBP
	POP ESI
	POP EDI
	RET

	GLOBAL _api_closewin
_api_closewin:		;void api_closewin(int win)
	PUSH EBX
	MOV EDX, 14
	MOV EBX, [ESP+8];win
	INT 0X40
	POP EBX
	RET

	GLOBAL _api_getkey
_api_getkey:		;int api_getkey(int mode)
	MOV EDX, 15
	MOV EAX, [ESP+4];mode
	INT 0X40
	RET
	
;TIMER
	GLOBAL _api_alloctimer
	GLOBAL _api_inittimer
	GLOBAL _api_settimer
	GLOBAL _api_freetimer
_api_alloctimer:	;int api_alloctimer
	MOV EDX, 16
	INT 0X40
	RET
_api_inittimer:		;void api_inittimer(int timer, int data)
	PUSH EBX
	MOV EDX, 17
	MOV EBX, [ESP+8];timer
	MOV EAX, [ESP+12];data
	INT 0X40
	POP EBX
	RET
_api_settimer:		;void api_settimer(int timer, int time)
	PUSH EBX
	MOV EDX, 18
	MOV EBX, [ESP+8];timer
	MOV EAX, [ESP+12];time
	INT 0X40
	POP EBX
	RET
_api_freetimer:		;void api_freetimer(int timer)
	PUSH EBX
	MOV EDX, 19
	MOV EBX, [ESP+8];timer
	INT 0X40
	POP EBX
	RET
