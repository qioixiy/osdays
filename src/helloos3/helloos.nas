;hello-os
;tab=4

	ORG 0X7C00	;指明程序的装载地址

;以下的记述用于标准fat12格式的磁盘
	JMP ENTRY
	DB 0X90
	DB 0X90
	DB "HELLOIPL"  ;启动区的名称可以是任意的字符串
	DW 512	       ;每个扇区（sector）的大小（必须为512字节）
	DB 1	       ;簇（cluster）的大小（必须为）一个扇区
	DW 1	       ;fat的启始位置（一般从第一个扇区开始）
	DB 2	       ;fat的个数（必须为2）
	DW 224	       ;根目录的大小（一般设成224）
	DW 2880	       ;该磁盘的大小（必须为2880）
	DB 0XF0	       ;磁盘的种类（必须为0xf0）
	DW 9	       ;fat的长度（必须是9个扇区）
	DW 18	       ;1个磁道有几个扇区（必须为18）
	DW 2	       ;磁头数（必须是2）
	DD 0	       ;不使用分区，必须是0
	DD 2880	       ;重写一次磁盘大小
	DB 0,0,0X29    ;意义不明，固定
	DD 0XFFFFFFFF  ;标卷号码
	DB "HELLO-OS   ";磁盘的名称（11字节）
	DB "FAT12   "  ;磁盘格式的名称（8字节）
	RESB 18	       ;先空出18字节

;程序核心
ENTRY:
	MOV AX, 0	;初始化寄存器
	MOV SS, AX
	MOV SP, 0X7C00
	MOV DS, AX
 	MOV ES, AX

	MOV SI, MSG

PUTLOOP:
	MOV AL, [SI]
	ADD SI, 1	;给si+1
	CMP AL, 0

	JE FIN
	MOV AH,0X0E	;显示一个文字
	MOV BX, 15	;指定字符颜色
	INT 0X10	;调用显卡bios
	JMP PUTLOOP

FIN:
	HLT		;让cpu停止，等待指令
	JMP FIN		;无限循环

MSG:
	DB 0X0A, 0X0A	;换行两次
	DB "HELLO, WORLD"
	DB 0X0A		;换行
	DB 0

	RESB 0X7DFE-$

	DB 0X55, 0XAA
	DB 0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
	RESB 4600
	DB 0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
	RESB 1469432
	