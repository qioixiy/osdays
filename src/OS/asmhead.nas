;haribote-os boot asm
;TAB=4

BOTPAK	EQU 0X00280000
DSKCAC	EQU 0X00100000
DSKCAC0 EQU 0X00008000

;�й�BOOT_INFO
CYLS	EQU 0X0FF0	;�趨������
LEDS 	EQU 0X0FF1
VMODE	EQU 0X0FF2	;������ɫ��Ŀ����Ϣ
SCRNX	EQU 0X0FF4	;�ֱ��ʵ�X
SCRNY	EQU 0X0FF6	;�ֱ��ʵ�Y
VRAM	EQU 0X0FF8 	;ͼ�񻺳����Ŀ�ʼλ��

;�������ļ�����0X4200�������ڴ��λ��Ϊ0X8000+0X4200=0XC200
	ORG 0XC200	;����Ҫװ�ص��ڴ�λ��
	MOV BX, 0X4105	;vga�Կ��� 320��200��8λ��ɫ
	MOV AX, 0X4F02
	INT 0X10
	
	MOV BYTE [VMODE], 8	;��¼����ģʽ
	MOV WORD [SCRNX], 1024
	MOV WORD [SCRNY], 768
	MOV DWORD [VRAM], 0Xe0000000;VRAM��0xa000-0xafff��64K

;��biosȡ�ü����ϸ���LEDָʾ�Ƶ�״̬
	MOV AH, 0X02
	INT 0X16	;keyboard bios
	MOV [LEDS], AL

;PIC�ر������жϣ�����AT���ݻ��Ĺ�����Ҫ��ʼ��PIC
;������CKI֮ǰ���У�������ʱ�����������PIC��ʼ��
	MOV AL, 0XFF
	OUT 0X21, AL
	NOP		;�������ִ����Щ�����޷���������
	OUT 0XA1, AL
	CLI		;��ֹCPU������ж�

;Ϊ����CPU�ܹ�����1MB���ϵ��ڴ�ռ䣬�趨A20GATE
	CALL WAITKBDOUT
	MOV AL, 0XD1
	OUT 0X64, AL
	CALL WAITKBDOUT
	MOV AL, 0XDF	;ENABLE A20
	OUT 0X60, AL
	CALL WAITKBDOUT

;�л�������ģʽ
[INSTRSET "i486p"]		;Ҫʹ��486ָ���˵��
	  LGDT [GDTR0]		;�趨��ʱ��GDT
	  MOV EAX, CR0
	  AND EAX, 0X7FFFFFFF	;����bit31Ϊ0����ֹ��
	  OR  EAX, 0X00000001	;����bit0Ϊ1���л�������ģʽ
	  MOV CR0, EAX
	  JMP PIPELINEFLUSH

PIPELINEFLUSH:
	MOV AX, 1*8		;�ɶ�д�Ķ�
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX

;backpack�Ĵ���
	MOV ESI, bootpack	;ת��Դ
	MOV EDI, BOTPAK		;ת��Ŀ�ĵ�
	MOV ECX, 512*1024/4
	CALL memcpy

;�����������մ�����������λ��ȥ

;���ȴ�����������ʼ
	MOV ESI, 0X7C00		;ת��Դ
	MOV EDI, DSKCAC		;ת��Ŀ�ĵ�
	MOV ECX, 512/4
	CALL memcpy

;����ʣ�µ�����
	MOV ESI, DSKCAC0+512	;ת��Դ
	MOV EDI, DSKCAC+512	;ת��Ŀ�ĵ�
	MOV ECX, 0
	MOV CL, BYTE [CYLS]
	IMUL ECX, 512*18*2/4	;���������任Ϊ�ֽ���/4����Ϊ��32λģʽ4�ֽ�
	SUB ECX, 512/4		;����IPL����
	CALL memcpy

;asmhead�� ˵����Page157
;������asmhead��ɵĹ���������ȫ����ϣ��Ժ�ͽ���bootpack����

;ϵͳ���ڴ�ֲ���P158
;0x00000000-0x000fffff	������ʱ����õ�����BIOS��VRAM�����ݣ������Ϊ���ˣ�1M��
;0x00100000-0x00267fff	���ڱ����������ݣ���1440KB��
;0x00268000-0x0026f7ff	�գ�30KB��
;0x0026f800-0x0026ffff	IDT��2KB��
;0x00270000-0x0027ffff	GDT��64KB��
;0x00280000-0x002fffff	bootpack.hrb��512KB��
;0x00300000-0x003fffff	ջ��������;��1M��
;0x00400000-...		��

;bootpack������
	MOV EBX, BOTPAK
	MOV ECX, [EBX+16]
	ADD ECX, 3		;ECX += 3
	SHR ECX, 2		;ECX /=4
	JZ  SKIP 		;û��Ҫ���͵Ķ���ʱ
	MOV ESI, [EBX+20]	;ת��Դ
	ADD ESI, EBX
	MOV EDI, [EBX+12]	;ת��Ŀ�ĵ�
	CALL 	 memcpy

SKIP:
	MOV ESP, [EBX+12]	;ջ��ʼֵ
	JMP DWORD 2*8:0X0000001B;��ת��bootpack��ڴ�

WAITKBDOUT:			;
	IN AL, 0X64
	AND AL, 0X02
	IN  AL, 0X60		;�ն���Ϊ��������ݽ��յĻ������е���������
	JNZ WAITKBDOUT		;���AND�Ľ������0����ת��WAITKBOOT
	RET

memcpy:
	MOV EAX, [ESI]
	ADD ESI, 4
	MOV [EDI], EAX
	ADD EDI, 4
	SUB ECX, 1
	JNZ memcpy		;�����������������0������ת��memcpy
	RET

	ALIGNB 16
GDT0:
	RESB 8			;NULL SELECTOR
	DW   0XFFFF, 0X0000, 0X9200, 0X00CF	;���Զ�д��segment(��)32λ�� seg1
	DW   0XFFFF, 0X0000, 0X9A28, 0X0047	;����ִ�е�segment(��)32λ��bootpack�ã� seg2
	DW   0
GDTR0:
	DW 8*3-1
	DD GDT0
	
	ALIGNB	16
bootpack: