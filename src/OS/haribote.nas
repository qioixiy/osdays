;haribote.nas
;TAB=4

;有关BOOT_INFO
CYLS	EQU 0X0FF0	;设定启动区
LEDS 	EQU 0X0FF1
VMODE	EQU 0X0FF2	;关于颜色数目的信息
SCRNX	EQU 0X0FF4	;分辨率的X
SCRNY	EQU 0X0FF6	;分辨率的Y
VRAM	EQU 0X0FF8 	;图像缓冲区的开始位置

;编译后的文件放在0X4200处，在内存的位置为0X8000+0X4200=0XC200
	ORG 0XC200	;程序要装载的内存位置
	MOV AL, 0X13	;vga显卡， 320×200×8位彩色
	MOV AH, 0X00
	INT 0X10
	
	MOV BYTE [VMODE], 8	;记录画面模式
	MOV WORD [SCRNX], 320
	MOV WORD [SCRNY], 200
	MOV DWORD [VRAM], 0X000A0000;VRAM从0xa000-0xafff的64K

;用bios取得键盘上各种LED指示灯的状态
	MOV AH, 0X02
	INT 0X16	;keyboard bios
	MOV [LEDS], AL
fin:
	HLT
	JMP fin
