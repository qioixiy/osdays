[FORMAT "WCOFF"]	;���ɶ����ļ��ĸ�ʽ
[INSTRSET "i486p"]	;��ʾʹ��I486����ָ�
[BITS 32] 		;����32ģʽ��ָ��
[FILE "a_nask.nas"]	;Դ�ļ�����Ϣ

      GLOBAL _api_putchar
      GLOBAL _api_end
      GLOBAL _api_putstr0
;C����֧��
[SECTION .text]
_api_putchar:	;void api_putchar(int c);
      MOV EDX, 1
      MOV AL, [ESP+4]	;c
      INT 0X40
      RET
	
_api_end:	;void api_end(void);
      MOV EDX, 4
      INT 0X40     
	
_api_putstr0:	;api_putstr0(char *s)
	PUSH EBX
	MOV EDX, 2
	MOV EBX, [ESP+8]	;s
	INT 0X40
	POP EBX
	RET