;试图修改定时器中断
[INSTRSET "i486p"]
[BITS 32]

      MOV AL, 0X34
      OUT 0X43, AL
      MOV AL, 0XFF
      OUT 0X40, AL
      MOV AL, 0XFF
      OUT 0X40, AL
;上述代码的功能与下面代码相当
;     io_out8(PIT_CTRL, 0X34)
;     io_out8(PIT_CNT0, 0XFF)
;     io_out8(PIT_CNT0, 0XFF)

      MOV EDX, 4
      INT 0X40
