;naskfunc
;tab=4

[FORMAT "WCOFF"]	;����Ŀ���ļ���ģʽ
[INSTRSET "i486p"]
[BITS 32]		;����32λģʽ�õĻ�������
[FILE "naskfunc.nas"]	;����Ŀ���ļ�����Ϣ

      GLOBAL _io_hlt	;�����а����ĺ�����
      GLOBAL _write_mem8

;������ʵ�ʵĺ���
[SECTION .text]		;Ŀ���ļ���д����Щ֮����д����
_io_hlt: 		;void _io_hlt(void)
	HLT
	RET

_write_mem8:		;void write_mem8(int addr, int data)
	MOV ECX, [ESP+4];[ESP+4]�д�ŵ��ǵ�ַ���������ECX
	MOV AL, [ESP+8]	;[ESP+8]�д�ŵ������ݣ��������AL
	MOV [ECX], AL
	RET
	