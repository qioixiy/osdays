;haribote.nas
;TAB=4
;编译后的文件放在0X4200处
;在内存的位置为0X8000+0X4200=0XC200
	ORG 0XC200	;程序要装载的内存位置

	MOV AL, 0X13	;vga显卡， 320×200×8位彩色
	MOV AH, 0X00
	INT 0X10

fin:
	HLT
	JMP fin
