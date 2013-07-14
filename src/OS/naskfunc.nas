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
	MOV EAX, ESP
	PUSH EAX
	MOV AX, SS
	MOV DS, AX
	MOV ES, AX
	CALL _inthandler20
	POP EAX
	POPAD
	POP DS
	POP ES
	IRETD

_asm_inthandler21:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV EAX, ESP
	PUSH EAX
	MOV AX, SS
	MOV DS, AX
	MOV ES, AX
	CALL _inthandler21
	POP EAX
	POPAD
	POP DS
	POP ES
	IRETD

_asm_inthandler27:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV EAX, ESP
	PUSH EAX
	MOV AX, SS
	MOV DS, AX
	MOV ES, AX
	CALL _inthandler27
	POP EAX
	POPAD
	POP DS
	POP ES
	IRETD

_asm_inthandler2c:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV EAX, ESP
	PUSH EAX
	MOV AX, SS
	MOV DS, AX
	MOV ES, AX
	CALL _inthandler2c
	POP EAX
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
	EXTERN _cons_putchar
	GLOBAL _asm_cons_putchar
_asm_cons_putchar:
	STI			;，由于是通过中断请求调用，自动执行了关中断CLI,所有需要开中断
	PUSHAD			;将全部寄存器值还原
	PUSH 1
	AND EAX, 0XFF		;将高位清零
	PUSH EAX
	PUSH DWORD [0X0FEC]	;读取内存并push该值
	CALL _cons_putchar	;C函数调用
	ADD ESP, 12		;将栈中的数据丢弃
	POPAD
	IRETD	
