;haribote.nas
;TAB=4

;�й�BOOT_INFO
CYLS	EQU 0X0FF0	;�趨������
LEDS 	EQU 0X0FF1
VMODE	EQU 0X0FF2	;������ɫ��Ŀ����Ϣ
SCRNX	EQU 0X0FF4	;�ֱ��ʵ�X
SCRNY	EQU 0X0FF6	;�ֱ��ʵ�Y
VRAM	EQU 0X0FF8 	;ͼ�񻺳����Ŀ�ʼλ��

;�������ļ�����0X4200�������ڴ��λ��Ϊ0X8000+0X4200=0XC200
	ORG 0XC200	;����Ҫװ�ص��ڴ�λ��
	MOV AL, 0X13	;vga�Կ��� 320��200��8λ��ɫ
	MOV AH, 0X00
	INT 0X10
	
	MOV BYTE [VMODE], 8	;��¼����ģʽ
	MOV WORD [SCRNX], 320
	MOV WORD [SCRNY], 200
	MOV DWORD [VRAM], 0X000A0000;VRAM��0xa000-0xafff��64K

;��biosȡ�ü����ϸ���LEDָʾ�Ƶ�״̬
	MOV AH, 0X02
	INT 0X16	;keyboard bios
	MOV [LEDS], AL
fin:
	HLT
	JMP fin
