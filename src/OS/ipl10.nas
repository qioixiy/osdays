;hello-os
;tab=4
	CYLS EQU 10	;10������
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

;һ�����̣�80���棬2����ͷ��18������һ��������512�ֽ�
;80*2*18*512�ֽ�=1474560�ֽ�=1440KB=1.44MB
;������
 	MOV AX, 0X0820	;ES:BX=�����ַ,ES*16+BX
	MOV ES, AX	;ES=0X0820
	MOV CH, 0	;����0
	MOV DH, 0	;��ͷ0
	MOV CL, 2	;����2,����1bios�Զ���ȡ

READLOOP:
	MOV SI, 0	;��¼ʧ�ܴ����ļĴ���

;���Զ�ȡ����
RETRY:
	MOV AH, 0X02	;AH=0X02,����
	MOV AL, 1	;1������
	MOV BX, 0	;BX=0,ES*16+BX=0X0820*16+0=0X8200,0X8000-0X81FF��512�ֽ�����������
	MOV DL, 0X00	;A������
	INT 0X13	;���ô���bios
	JNC NEXT	;û�д�Ļ���ת��NEXT
	ADD SI, 1	;ʧ�ܴ�����1
	CMP SI, 5	;�Ƚ�SI��5
	JAE ERROR	;SI>=5,��ת��ERROR
	MOV AH, 0X00
	MOV DL, 0X00	;A������
	INT 0X13	;����������
	JMP RETRY
;��ȡ��һ������
NEXT:
	MOV AX, ES	;���ڴ��ַ����0X200
	ADD AX, 0X0020
	MOV ES, AX	;��Ϊû��ADD ES, 0X20ָ�����ֱ�Ӳ���ES�Ĵ�������������������
	ADD CL, 1
	CMP CL, 18	;���18������û�ж�ȡ�꣬����
	JBE READLOOP
	MOV CL, 1
	ADD DH, 1
	CMP DH, 2	;��ȡ������ͷ
	JB READLOOP	;
	MOV DH, 0
	ADD CH, 1
	CMP CH, CYLS	
	JB READLOOP	;��ȡ��������
	
	MOV [0X0FF0], CH;����CH[CYLS=10]���ڴ�0xff0��
;��������ת��0xc200��ִ��haribote.sys
	JMP 0XC200

ERROR:
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
	DB "LOAD ERROR"
	DB 0X0A		;����
	DB 0

	RESB 0X7DFE-$

	DB 0X55, 0XAA