;haribote-os boot asm
;TAB=4

BOTPAK	EQU 0X00280000
DSKCAC	EQU 0X00100000
DSKCAC0 EQU 0X00008000

;有关BOOT_INFO
CYLS	EQU 0X0FF0	;设定启动区
LEDS 	EQU 0X0FF1
VMODE	EQU 0X0FF2	;关于颜色数目的信息
SCRNX	EQU 0X0FF4	;分辨率的X
SCRNY	EQU 0X0FF6	;分辨率的Y
VRAM	EQU 0X0FF8 	;图像缓冲区的开始位置

;编译后的文件放在0X4200处，在内存的位置为0X8000+0X4200=0XC200
	ORG 0XC200	;程序要装载的内存位置
	MOV BX, 0X4105	;vga显卡， 320×200×8位彩色
	MOV AX, 0X4F02
	INT 0X10
	
	MOV BYTE [VMODE], 8	;记录画面模式
	MOV WORD [SCRNX], 1024
	MOV WORD [SCRNY], 768
	MOV DWORD [VRAM], 0Xe0000000;VRAM从0xa000-0xafff的64K

;用bios取得键盘上各种LED指示灯的状态
	MOV AH, 0X02
	INT 0X16	;keyboard bios
	MOV [LEDS], AL

;PIC关闭所有中断，根据AT兼容机的规格，如果要初始化PIC
;必须在CKI之前进行，否则有时会挂起，随后进行PIC初始化
	MOV AL, 0XFF
	OUT 0X21, AL
	NOP		;如果连续执行有些机型无法正常运行
	OUT 0XA1, AL
	CLI		;禁止CPU级别的中断

;为了让CPU能够访问1MB以上的内存空间，设定A20GATE
	CALL WAITKBDOUT
	MOV AL, 0XD1
	OUT 0X64, AL
	CALL WAITKBDOUT
	MOV AL, 0XDF	;ENABLE A20
	OUT 0X60, AL
	CALL WAITKBDOUT

;切换到保护模式
[INSTRSET "i486p"]		;要使用486指令的说明
	  LGDT [GDTR0]		;设定临时的GDT
	  MOV EAX, CR0
	  AND EAX, 0X7FFFFFFF	;设置bit31为0，禁止×
	  OR  EAX, 0X00000001	;设置bit0为1，切换到保护模式
	  MOV CR0, EAX
	  JMP PIPELINEFLUSH

PIPELINEFLUSH:
	MOV AX, 1*8		;可读写的段
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX

;backpack的传送
	MOV ESI, bootpack	;转送源
	MOV EDI, BOTPAK		;转送目的地
	MOV ECX, 512*1024/4
	CALL memcpy

;磁盘数据最终传到它本来的位置去

;首先从启动扇区开始
	MOV ESI, 0X7C00		;转送源
	MOV EDI, DSKCAC		;转送目的地
	MOV ECX, 512/4
	CALL memcpy

;所有剩下的数据
	MOV ESI, DSKCAC0+512	;转送源
	MOV EDI, DSKCAC+512	;转送目的地
	MOV ECX, 0
	MOV CL, BYTE [CYLS]
	IMUL ECX, 512*18*2/4	;从柱面数变换为字节数/4，因为是32位模式4字节
	SUB ECX, 512/4		;减掉IPL部分
	CALL memcpy

;asmhead， 说明见Page157
;必须由asmhead完成的工作，到此全部完毕，以后就交由bootpack继续

;系统的内存分布：P158
;0x00000000-0x000fffff	在启动时多次用到，有BIOS，VRAM等内容，但最后为空了（1M）
;0x00100000-0x00267fff	用于保存软盘内容，（1440KB）
;0x00268000-0x0026f7ff	空（30KB）
;0x0026f800-0x0026ffff	IDT（2KB）
;0x00270000-0x0027ffff	GDT（64KB）
;0x00280000-0x002fffff	bootpack.hrb（512KB）
;0x00300000-0x003fffff	栈和其他用途（1M）
;0x00400000-...		空

;bootpack的启动
	MOV EBX, BOTPAK
	MOV ECX, [EBX+16]
	ADD ECX, 3		;ECX += 3
	SHR ECX, 2		;ECX /=4
	JZ  SKIP 		;没有要传送的东西时
	MOV ESI, [EBX+20]	;转送源
	ADD ESI, EBX
	MOV EDI, [EBX+12]	;转送目的地
	CALL 	 memcpy

SKIP:
	MOV ESP, [EBX+12]	;栈初始值
	JMP DWORD 2*8:0X0000001B;跳转到bootpack入口处

WAITKBDOUT:			;
	IN AL, 0X64
	AND AL, 0X02
	IN  AL, 0X60		;空读，为了清除数据接收的缓存区中的垃圾数据
	JNZ WAITKBDOUT		;如果AND的结果不是0就跳转到WAITKBOOT
	RET

memcpy:
	MOV EAX, [ESI]
	ADD ESI, 4
	MOV [EDI], EAX
	ADD EDI, 4
	SUB ECX, 1
	JNZ memcpy		;如果减法运算结果不是0，就跳转到memcpy
	RET

	ALIGNB 16
GDT0:
	RESB 8			;NULL SELECTOR
	DW   0XFFFF, 0X0000, 0X9200, 0X00CF	;可以读写的segment(段)32位， seg1
	DW   0XFFFF, 0X0000, 0X9A28, 0X0047	;可以执行的segment(段)32位（bootpack用） seg2
	DW   0
GDTR0:
	DW 8*3-1
	DD GDT0
	
	ALIGNB	16
bootpack: