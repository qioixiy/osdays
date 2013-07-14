[BITS 32]
      MOV AL, 'A'
      CALL 2*8:0XCCB		;长跳转，指定段和偏移量
      RETF