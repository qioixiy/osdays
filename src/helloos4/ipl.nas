;hello-os
;tab=4

	ORG 0X7C00	;ָ�������װ�ص�ַ

;���µļ������ڱ�׼fat12��ʽ�Ĵ���
	JMP ENTRY
	DB 0X90
	DB "HELLOIPL"  ;�����������ƿ�����������ַ���
	DW 512	       ;ÿ��������sector���Ĵ�С������Ϊ512�ֽڣ�
	DB 1	       ;�أ�cluster���Ĵ�С������Ϊ��һ������
	DW 1	       ;fat����ʼλ�ã�һ��ӵ�һ��������ʼ��
	DB 2	       ;fat�ĸ���������Ϊ2��
	DW 224	       ;��Ŀ¼�Ĵ�С��һ�����224��
	DW 2880	       ;�ô��̵Ĵ�С������Ϊ2880��
	DB 0XF0	       ;���̵����ࣨ����Ϊ0xf0��
	DW 9	       ;fat�ĳ��ȣ�������9��������
	DW 18	       ;1���ŵ��м�������������Ϊ18��
	DW 2	       ;��ͷ����������2��
	DD 0	       ;��ʹ�÷�����������0
	DD 2880	       ;��дһ�δ��̴�С
	DB 0,0,0X29    ;���岻�����̶�
	DD 0XFFFFFFFF  ;������
	DB "HELLO-OS   ";���̵����ƣ�11�ֽڣ�
	DB "FAT12   "  ;���̸�ʽ�����ƣ�8�ֽڣ�
	RESB 18	       ;�ȿճ�18�ֽ�

;�������
ENTRY:
	MOV AX, 0	;��ʼ���Ĵ���
	MOV SS, AX
	MOV SP, 0X7C00
	MOV DS, AX
 	MOV ES, AX

	MOV SI, MSG

PUTLOOP:
	MOV AL, [SI]
	ADD SI, 1	;��si+1
	CMP AL, 0

	JE FIN
	MOV AH,0X0E	;��ʾһ������
	MOV BX, 15	;ָ���ַ���ɫ
	INT 0X10	;�����Կ�bios
	JMP PUTLOOP

FIN:
	HLT		;��cpuֹͣ���ȴ�ָ��
	JMP FIN		;����ѭ��

MSG:
	DB 0X0A, 0X0A	;��������
	DB "HELLO, WORLD,zxyuan!"
	DB 0X0A		;����
	DB 0

	RESB 0X7DFE-$

	DB 0X55, 0XAA