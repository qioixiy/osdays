;naskfunc
;tab=4

[FORMAT "WCOFF"]	;����Ŀ���ļ���ģʽ
[INSTRSET "i486p"]
[BITS 32]		;����32λģʽ�õĻ�������
[FILE "naskfunc.nas"]	;����Ŀ���ļ�����Ϣ

;�����а����ĺ�����
      GLOBAL _io_hlt, _io_cli,_io_sti,_io_stihlt
      GLOBAL _io_in8, _io_in16, _io_in32
      GLOBAL _io_out8, _io_out16, _io_out32
      GLOBAL _io_load_eflags, _io_store_eflags

;������ʵ�ʵĺ���
[SECTION .text]		;Ŀ���ļ���д����Щ֮����д����
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
	MOV ECX, [ESP+4];[ESP+4]�д�ŵ��ǵ�ַ���������ECX
	MOV AL, [ESP+8]	;[ESP+8]�д�ŵ������ݣ��������AL
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
	CMP AX, 1*8	;�жϵ�ǰ�Ƿ����ں�̬
	JNE .from_app
;���������ں���
	MOV EAX, ESP
	PUSH SS		;�����ж�ʱ��SS
	PUSH EAX	;�����ж�ʱ��ESP
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
	;�жϷ�����Ӧ�ó��������
	MOV EAX, 1*8
	MOV DS, AX		;�Ƚ���DS�趨Ϊ����ϵͳ��
	MOV ECX, [0xfe4]	;����ϵͳ��ESP
	ADD ECX, -8
	MOV [ECX+4], SS		;�����ж�ʱ��SS
	MOV [ECX],ESP		;�����ж�ʱ��ESP
	MOV SS, AX 			
	MOV ES, AX
	MOV ESP, ECX
	CALL _inthandler20
	POP ECX
	POP EAX
	MOV SS, AX		;��SS���Ӧ�ó�����
	MOV ESP, ECX		;��ESP���Ӧ�ó�����
	POPAD
	POP DS
 	POP ES
	IRETD

_asm_inthandler21:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX, SS
	CMP AX, 1*8	;�жϵ�ǰ�Ƿ����ں�̬
	JNE .from_app
;���������ں���
	MOV EAX, ESP
	PUSH SS		;�����ж�ʱ��SS
	PUSH EAX	;�����ж�ʱ��ESP
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
	;�жϷ�����Ӧ�ó��������
	MOV EAX, 1*8
	MOV DS, AX		;�Ƚ���DS�趨Ϊ����ϵͳ��
	MOV ECX, [0xfe4]	;����ϵͳ��ESP
	ADD ECX, -8
	MOV [ECX+4], SS		;�����ж�ʱ��SS
	MOV [ECX],ESP		;�����ж�ʱ��ESP
	MOV SS, AX 			
	MOV ES, AX
	MOV ESP, ECX
	CALL _inthandler21
	POP ECX
	POP EAX
	MOV SS, AX		;��SS���Ӧ�ó�����
	MOV ESP, ECX		;��ESP���Ӧ�ó�����
	POPAD
	POP DS
 	POP ES
	IRETD

_asm_inthandler27:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX, SS
	CMP AX, 1*8	;�жϵ�ǰ�Ƿ����ں�̬
	JNE .from_app
;���������ں���
	MOV EAX, ESP
	PUSH SS		;�����ж�ʱ��SS
	PUSH EAX	;�����ж�ʱ��ESP
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
	;�жϷ�����Ӧ�ó��������
	MOV EAX, 1*8
	MOV DS, AX		;�Ƚ���DS�趨Ϊ����ϵͳ��
	MOV ECX, [0xfe4]	;����ϵͳ��ESP
	ADD ECX, -8
	MOV [ECX+4], SS		;�����ж�ʱ��SS
	MOV [ECX],ESP		;�����ж�ʱ��ESP
	MOV SS, AX 			
	MOV ES, AX
	MOV ESP, ECX
	CALL _inthandler27
	POP ECX
	POP EAX
	MOV SS, AX		;��SS���Ӧ�ó�����
	MOV ESP, ECX		;��ESP���Ӧ�ó�����
	POPAD
	POP DS
 	POP ES
	IRETD

_asm_inthandler2c:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX, SS
	CMP AX, 1*8	;�жϵ�ǰ�Ƿ����ں�̬
	JNE .from_app
;���������ں���
	MOV EAX, ESP
	PUSH SS		;�����ж�ʱ��SS
	PUSH EAX	;�����ж�ʱ��ESP
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
	;�жϷ�����Ӧ�ó��������
	MOV EAX, 1*8
	MOV DS, AX		;�Ƚ���DS�趨Ϊ����ϵͳ��
	MOV ECX, [0xfe4]	;����ϵͳ��ESP
	ADD ECX, -8
	MOV [ECX+4], SS		;�����ж�ʱ��SS
	MOV [ECX],ESP		;�����ж�ʱ��ESP
	MOV SS, AX 			
	MOV ES, AX
	MOV ESP, ECX
	CALL _inthandler2c
	POP ECX
	POP EAX
	MOV SS, AX		;��SS���Ӧ�ó�����
	MOV ESP, ECX		;��ESP���Ӧ�ó�����
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
	PUSH EDI		; ��������Ҫʹ��EDI��ESI��EBX����ջ
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
	;Ϊ�˷����ͷ��ʼ�ͽ�ֹ�ж�����
	PUSH DS
	PUSH ES
	PUSHAD			;���ڱ����PUSH
	MOV EAX, 1*8
	MOV DS, AX		;�Ƚ���DS�趨Ϊ����ϵͳ��
	MOV ECX, [0XFE4]	;����ϵͳ��ESP����ֵ��ECX�����¹�����
	ADD ECX, -40		;
	MOV [ECX+32], ESP	;����Ӧ�ó����ESP��ʹ�ò���ϵͳ��ջ�ռ�
	MOV [ECX+36], SS	;����Ӧ�ó����SS

;��PUSHAD���ֵ���Ƶ�ϵͳջ��,����Ӧ�ó���ջ�е����ݸ��Ƶ�����ϵͳջ��
	MOV EDX, [ESP]		;��ǰ��ESP��ָ��Ӧ�ó����
	MOV EBX, [ESP+4]
	MOV [ECX], EDX		;���ƴ��ݸ�hrb_api
	MOV [ECX+4], EBX	;���ƴ��ݸ�hrb_api
	MOV EDX, [ESP+8]
	MOV EBX, [ESP+12]
	MOV [ECX+8],EDX		;���ƴ��ݸ�hrb_api
	MOV [ECX+12], EBX	;���ƴ��ݸ�hrb_api
	MOV EDX, [ESP+16]
	MOV EBX, [ESP+20]
	MOV [ECX+16], EDX	;���ƴ��ݸ�hrb_api
	MOV [ECX+20], EBX	;���ƴ��ݸ�hrb_api
	MOV EDX, [ESP+24]
	MOV EBX, [ESP+28]
	MOV [ECX+24], EDX	;���ƴ��ݸ�hrb_api
	MOV [ECX+28], EBX	;���ƴ��ݸ�hrb_api

	MOV ES, AX    		;��ʣ��ĶμĴ���Ҳ��Ϊ����ϵͳ�õ�
	MOV SS, AX		;
	MOV ESP, ECX	

	STI			;�ָ��ж�����
	
	CALL _hrb_api		;����ϵͳapi

	MOV ECX, [ESP+32]	;ȡ��Ӧ�ó����ESP
	MOV EAX, [ESP+36]	;ȡ��Ӧ�ó����SS
	CLI
	MOV SS, AX
	MOV ESP, ECX
	POPAD
	POP ES
	POP DS
	IRETD			;��ָ����Զ�ִ��STI

;APP start
        GLOBAL _start_app
_start_app:			;void start_app(int eip, int cs, int esp, int ds);
	PUSHAD			;��32λ�Ĵ���ȫ������������8���ܹ�32�ֽڣ�
	MOV EAX, [ESP+36]	;Ӧ�ó����õ�EIP
	MOV ECX, [ESP+40]	;Ӧ�ó����õ�CX
	MOV EDX, [ESP+44]	;Ӧ�ó����õ�ESP
	MOV EBX, [ESP+48]	;Ӧ�ó����õ�DS/SS
	MOV [0XFE4], ESP	;����ϵͳ��ESP���������ڴ�0xfe4λ��

	CLI 	     		;���л������н�ֹ�ж�����
	MOV ES, BX
	MOV SS, BX
	MOV DS, BX
	MOV FS, BX
	MOV GS, BX
	MOV ESP, EDX
	STI			;�л���ɺ�ָ��ж�����
	
	PUSH ECX		;����far-call��push(CS)
	PUSH EAX		;����far-call��push(EIP)
	CALL FAR [ESP]		;����Ӧ�ó���

;Ӧ�ó�������󷵻ش˴�
	MOV EAX, 1*8		;����ϵͳ�õ�DS/SS��Ϣ
	CLI			;�ٴν����л�����ֹ�ж����� 
	MOV ES, AX		
	MOV SS, AX
	MOV DS, AX
	MOV FS, AX
	MOV GS, AX
	MOV ESP, [0XFE4]	;�ָ�����ϵͳESP����0xfe4��
	STI			;�л���ɺ�ָ��ж�����
	POPAD			;�ָ�֮ǰ����ļĴ���ֵ
	RET