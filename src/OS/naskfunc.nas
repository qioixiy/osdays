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
_io_sti:		;void io_str(void)
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
	