;naskfunc
;tab=4

[FORMAT "WCOFF"]	;制作目标文件的模式
[INSTRSET "i486p"]
[BITS 32]		;制作32位模式用的机器语言
[FILE "naskfunc.nas"]	;制作目标文件的信息

;程序中包含的函数名
      GLOBAL _io_hlt, _io_cli,_io_sti,_io_stihlt
      GLOBAL _io_in8, _io_in16, _io_in32
      GLOBAL _io_out8, _io_out16, _io_out32
      GLOBAL _io_load_eflags, _io_store_eflags

;以下是实际的函数
[SECTION .text]		;目标文件中写了这些之后再写程序
_io_hlt: 		;void _io_hlt(void)
	HLT
	RET
_io_cli:		;void _io_cli(void)
	CLI
	RET
_io_sti:		;void io_sti(void)
	STI
	RET
_io_stihlt:		;void io_stihlt(void)
	STI
	HLT
	RET

_io_in8:		;int io_in8(int port)
	MOV EDX, [ESP+4];port
	MOV EAX, 0
	IN  AL, DX
	RET
_io_in16:		;int io_in16(int port)
	MOV EDX, [ESP+4]
	MOV EAX, 0
	IN AX, DX
	RET
_io_in32:		;int io_in32(int port)
	MOV EDX, [ESP+4]
	IN EAX,DX
	RET

_io_out8:		;void io_out8(int port, int data)
	MOV EDX, [ESP+4];port
	MOV AL, [ESP+8]	;data
	OUT DX, AL
	RET
_io_out16:		;void io_out16(int port, int data)
	MOV EDX, [ESP+4]
	MOV EAX, [ESP+8]
	OUT DX, AX
	RET
_io_out32:		;void io_out32(int port, int data)
	MOV EDX, [ESP+4]
	MOV EAX, [ESP+8]
	OUT DX, EAX
	RET

_io_load_eflags:	;int io_load_eflags(void)
	PUSHFD
	POP EAX
	RET
_io_store_eflags:	;void io_store_eflags(int eflags)
	MOV EAX, [ESP+4]
	PUSH EAX
	POPFD
	RET

GLOBAL _write_mem8
_write_mem8:		;void write_mem8(int addr, int data)
	MOV ECX, [ESP+4];[ESP+4]中存放的是地址，将其读入ECX
	MOV AL, [ESP+8]	;[ESP+8]中存放的是数据，将其读入AL
	MOV [ECX], AL
	RET
	
	GLOBAL _load_gdtr, _load_idtr
_load_gdtr:    		;void load_gdtr(int limit, int addr);
	MOV AX, [ESP+4]		  ;limit
	MOV [ESP+6], AX
	LGDT [ESP+6]
	RET
_load_idtr:	;void load_idtr(int limit, int addr);
	MOV AX, [ESP+4]		  ;limit
	MOV [ESP+6], AX
	LIDT [ESP+6]
	RET

	GLOBAL	_asm_inthandler20, _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
	EXTERN	_inthandler20, _inthandler21, _inthandler27, _inthandler2c
	
_asm_inthandler20:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX, SS
	CMP AX, 1*8	;判断当前是否处于内核态
	JNE .from_app
;当发生在内核中
	MOV EAX, ESP
	PUSH SS		;保存中断时的SS
	PUSH EAX	;保存中断时的ESP
	MOV AX, SS
	MOV DS, AX
	MOV ES, AX
	CALL _inthandler20
	
	ADD ESP, 8
	POPAD
	POP DS
	POP ES
	IRETD
.from_app:
	;中断发生在应用程序过程中
	MOV EAX, 1*8
	MOV DS, AX		;先仅将DS设定为操作系统用
	MOV ECX, [0xfe4]	;操作系统的ESP
	ADD ECX, -8
	MOV [ECX+4], SS		;保存中断时的SS
	MOV [ECX],ESP		;保存中断时的ESP
	MOV SS, AX 			
	MOV ES, AX
	MOV ESP, ECX
	CALL _inthandler20
	POP ECX
	POP EAX
	MOV SS, AX		;将SS设回应用程序用
	MOV ESP, ECX		;将ESP设回应用程序用
	POPAD
	POP DS
 	POP ES
	IRETD

_asm_inthandler21:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX, SS
	CMP AX, 1*8	;判断当前是否处于内核态
	JNE .from_app
;当发生在内核中
	MOV EAX, ESP
	PUSH SS		;保存中断时的SS
	PUSH EAX	;保存中断时的ESP
	MOV AX, SS
	MOV DS, AX
	MOV ES, AX
	CALL _inthandler21
	
	ADD ESP, 8
	POPAD
	POP DS
	POP ES
	IRETD
.from_app:
	;中断发生在应用程序过程中
	MOV EAX, 1*8
	MOV DS, AX		;先仅将DS设定为操作系统用
	MOV ECX, [0xfe4]	;操作系统的ESP
	ADD ECX, -8
	MOV [ECX+4], SS		;保存中断时的SS
	MOV [ECX],ESP		;保存中断时的ESP
	MOV SS, AX 			
	MOV ES, AX
	MOV ESP, ECX
	CALL _inthandler21
	POP ECX
	POP EAX
	MOV SS, AX		;将SS设回应用程序用
	MOV ESP, ECX		;将ESP设回应用程序用
	POPAD
	POP DS
 	POP ES
	IRETD

_asm_inthandler27:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX, SS
	CMP AX, 1*8	;判断当前是否处于内核态
	JNE .from_app
;当发生在内核中
	MOV EAX, ESP
	PUSH SS		;保存中断时的SS
	PUSH EAX	;保存中断时的ESP
	MOV AX, SS
	MOV DS, AX
	MOV ES, AX
	CALL _inthandler27
	
	ADD ESP, 8
	POPAD
	POP DS
	POP ES
	IRETD
.from_app:
	;中断发生在应用程序过程中
	MOV EAX, 1*8
	MOV DS, AX		;先仅将DS设定为操作系统用
	MOV ECX, [0xfe4]	;操作系统的ESP
	ADD ECX, -8
	MOV [ECX+4], SS		;保存中断时的SS
	MOV [ECX],ESP		;保存中断时的ESP
	MOV SS, AX 			
	MOV ES, AX
	MOV ESP, ECX
	CALL _inthandler27
	POP ECX
	POP EAX
	MOV SS, AX		;将SS设回应用程序用
	MOV ESP, ECX		;将ESP设回应用程序用
	POPAD
	POP DS
 	POP ES
	IRETD

_asm_inthandler2c:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX, SS
	CMP AX, 1*8	;判断当前是否处于内核态
	JNE .from_app
;当发生在内核中
	MOV EAX, ESP
	PUSH SS		;保存中断时的SS
	PUSH EAX	;保存中断时的ESP
	MOV AX, SS
	MOV DS, AX
	MOV ES, AX
	CALL _inthandler2c
	
	ADD ESP, 8
	POPAD
	POP DS
	POP ES
	IRETD
.from_app:
	;中断发生在应用程序过程中
	MOV EAX, 1*8
	MOV DS, AX		;先仅将DS设定为操作系统用
	MOV ECX, [0xfe4]	;操作系统的ESP
	ADD ECX, -8
	MOV [ECX+4], SS		;保存中断时的SS
	MOV [ECX],ESP		;保存中断时的ESP
	MOV SS, AX 			
	MOV ES, AX
	MOV ESP, ECX
	CALL _inthandler2c
	POP ECX
	POP EAX
	MOV SS, AX		;将SS设回应用程序用
	MOV ESP, ECX		;将ESP设回应用程序用
	POPAD
	POP DS
 	POP ES
	IRETD

	GLOBAL _load_cr0, _store_cr0
_load_cr0:		;int load_cr0();
	MOV EAX, CR0
	RET

_store_cr0:		;void store_cr0(int cr0)
	MOV EAX, [ESP + 4]
	MOV CR0, EAX
	RET

	GLOBAL _load_tr
_load_tr:		;void load_tr(int tr)
	LTR [ESP+4]	;tr
	RET
	
	GLOBAL _memtest_sub
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
	PUSH EDI		; 过程中需要使用EDI、ESI、EBX先入栈
	PUSH ESI
	PUSH EBX
	MOV ESI,0xaa55aa55	; pat0 = 0xaa55aa55;
	MOV EDI,0x55aa55aa	; pat1 = 0x55aa55aa;
	MOV EAX,[ESP+12+4]	; i = start;
mts_loop:
	MOV EBX,EAX
	ADD EBX,0xffc		; p = i + 0xffc;
	MOV EDX,[EBX]		; old = *p;
	MOV [EBX],ESI		; *p = pat0;
	XOR DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
	CMP EDI,[EBX]		; if (*p != pat1) goto fin;
	JNE mts_fin
	XOR DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
	CMP ESI,[EBX]		; if (*p != pat0) goto fin;
	JNE mts_fin
	MOV [EBX],EDX		; *p = old;
	ADD EAX,0x1000		; i += 0x1000;
	CMP EAX,[ESP+12+8]	; if (i <= end) goto mts_loop;
	JBE mts_loop
	POP EBX
	POP ESI
	POP EDI
	RET
mts_fin:
	MOV [EBX],EDX		; *p = old;
	POP EBX
	POP ESI
	POP EDI
	RET

	GLOBAL _farjmp
_farjmp:			;void farjmp(int eip, int cs)
	JMP FAR [ESP+4]		;eip,cs
	RET
	GLOBAL _farcall
_farcall:			;void farcall(int eip, int cs);
	CALL FAR [ESP+4]	;eip, cs	
	RET


;API
	EXTERN _hrb_api
	GLOBAL _asm_hrb_api
_asm_hrb_api:
	;为了方便从头开始就禁止中断请求
	PUSH DS
	PUSH ES
	PUSHAD			;用于保存的PUSH
	MOV EAX, 1*8
	MOV DS, AX		;先仅将DS设定为操作系统用
	MOV ECX, [0XFE4]	;操作系统的ESP，赋值给ECX供以下过程用
	ADD ECX, -40		;
	MOV [ECX+32], ESP	;保存应用程序的ESP，使用操作系统的栈空间
	MOV [ECX+36], SS	;保存应用程序的SS

;将PUSHAD后的值复制到系统栈中,即将应用程序栈中的数据复制到操作系统栈中
	MOV EDX, [ESP]		;当前的ESP是指向应用程序的
	MOV EBX, [ESP+4]
	MOV [ECX], EDX		;复制传递给hrb_api
	MOV [ECX+4], EBX	;复制传递给hrb_api
	MOV EDX, [ESP+8]
	MOV EBX, [ESP+12]
	MOV [ECX+8],EDX		;复制传递给hrb_api
	MOV [ECX+12], EBX	;复制传递给hrb_api
	MOV EDX, [ESP+16]
	MOV EBX, [ESP+20]
	MOV [ECX+16], EDX	;复制传递给hrb_api
	MOV [ECX+20], EBX	;复制传递给hrb_api
	MOV EDX, [ESP+24]
	MOV EBX, [ESP+28]
	MOV [ECX+24], EDX	;复制传递给hrb_api
	MOV [ECX+28], EBX	;复制传递给hrb_api

	MOV ES, AX    		;将剩余的段寄存器也设为操作系统用的
	MOV SS, AX		;
	MOV ESP, ECX	

	STI			;恢复中断请求
	
	CALL _hrb_api		;调用系统api

	MOV ECX, [ESP+32]	;取出应用程序的ESP
	MOV EAX, [ESP+36]	;取出应用程序的SS
	CLI
	MOV SS, AX
	MOV ESP, ECX
	POPAD
	POP ES
	POP DS
	IRETD			;此指令会自动执行STI

;APP start
        GLOBAL _start_app
_start_app:			;void start_app(int eip, int cs, int esp, int ds);
	PUSHAD			;将32位寄存器全部保存起来，8个总共32字节？
	MOV EAX, [ESP+36]	;应用程序用的EIP
	MOV ECX, [ESP+40]	;应用程序用的CX
	MOV EDX, [ESP+44]	;应用程序用的ESP
	MOV EBX, [ESP+48]	;应用程序用的DS/SS
	MOV [0XFE4], ESP	;操作系统用ESP，保存在内存0xfe4位置

	CLI 	     		;在切换过程中禁止中断请求
	MOV ES, BX
	MOV SS, BX
	MOV DS, BX
	MOV FS, BX
	MOV GS, BX
	MOV ESP, EDX
	STI			;切换完成后恢复中断请求
	
	PUSH ECX		;用于far-call的push(CS)
	PUSH EAX		;用于far-call的push(EIP)
	CALL FAR [ESP]		;调用应用程序

;应用程序结束后返回此处
	MOV EAX, 1*8		;操作系统用的DS/SS信息
	CLI			;再次进行切换，禁止中断请求 
	MOV ES, AX		
	MOV SS, AX
	MOV DS, AX
	MOV FS, AX
	MOV GS, AX
	MOV ESP, [0XFE4]	;恢复操作系统ESP，从0xfe4中
	STI			;切换完成后恢复中断请求
	POPAD			;恢复之前保存的寄存器值
	RET