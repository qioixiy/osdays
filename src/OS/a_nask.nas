[FORMAT "WCOFF"]	;���ɶ����ļ��ĸ�ʽ
[INSTRSET "i486p"]	;��ʾʹ��I486����ָ�
[BITS 32] 		;����32ģʽ��ָ��
[FILE "a_nask.nas"]	;Դ�ļ�����Ϣ

      GLOBAL _api_putchar
      GLOBAL _api_end

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
	