[FORMAT "WCOFF"]	;生成对象文件的格式
[INSTRSET "i486p"]	;表示使用I486兼容指令集
[BITS 32] 		;生成32模式的指令
[FILE "a_nask.nas"]	;源文件名信息

      GLOBAL _api_putchar
[SECTION .text]
_api_putchar:			;void api_putchar(int c);
	      MOV EDX, 1
	      MOV AL, [ESP+4]	;c
	      INT 0X40
	      RET
	
