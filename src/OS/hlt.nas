[BITS 32]
      MOV AL, 'H'
      CALL 2*8:0XCCB		;长跳转，指定段和偏移量
      MOV AL, 'E'
      CALL 2*8:0XCCB		;长跳转，指定段和偏移量
      MOV AL, 'L'
      CALL 2*8:0XCCB
      MOV AL, 'L'
      CALL 2*8:0XCCB
      MOV AL, 'O'
      CALL 2*8:0XCCB
      
      RETF