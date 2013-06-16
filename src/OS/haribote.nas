;编译后的文件放在0X4200处
;在内存的位置为0X8000+0X4200=0XC200
ORG 0XC200
fin:
		HLT
		JMP		fin
