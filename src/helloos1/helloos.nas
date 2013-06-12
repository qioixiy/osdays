;hello-os
;tab=4

;以下这段是标准fat12格式软盘专用的代码
	DB 0XEB, 0X4E, 0X90
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
	DB "FAT12   "    ;磁盘格式的名称（8字节）
	RESB 18	       ;先空出18字节

;程序主体
	DB 0XB8, 0X00, 0X00, 0X8E, 0XD0, 0XBC, 0X00, 0X7C
	DB 0X8E, 0XD8, 0X8E, 0XC0, 0XBE, 0X74, 0X7C, 0X8A
	DB 0X04, 0X83, 0XC6, 0X01, 0X3C, 0X00, 0X74, 0X09
	DB 0XB4, 0X0E, 0XBB, 0X0F, 0X00, 0XCD, 0X10, 0XEB
	DB 0XEE, 0XF4, 0XEB, 0XFD


;信息显示部分
	DB 0X0A, 0X0A	;两个换行符
	DB "HELLO, WORLD"
	DB 0X0A		;换行符
	DB 0

	RESB 0X1FE-$	;填写0x00，直到0x001fe
	DB 0X55, 0XAA

;以下是启动区以外部分的输出
	DB 0XF0, 0XFF, 0XFF, 0X00, 0X00, 0X00, 0X00, 0X00
	RESB 4600
	DB 0XF0, 0XFF, 0XFF, 0X00, 0X00, 0X00, 0X00, 0X00
	RESB 1469432	