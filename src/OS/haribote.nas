;haribote.nas
;TAB=4
;�������ļ�����0X4200��
;���ڴ��λ��Ϊ0X8000+0X4200=0XC200
	ORG 0XC200	;����Ҫװ�ص��ڴ�λ��

	MOV AL, 0X13	;vga�Կ��� 320��200��8λ��ɫ
	MOV AH, 0X00
	INT 0X10

fin:
	HLT
	JMP fin
