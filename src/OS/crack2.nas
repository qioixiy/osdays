[INSTRSET "i486p"]
[BITS 32]
      MOV EAX, 1*8	;os用的段号
      MOV DS, AX	;将其存入DS
      MOV BYTE [0X102600], 0
      MOV EDX, 4
      INT 0X40
