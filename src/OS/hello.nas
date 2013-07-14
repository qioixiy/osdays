[INSTRSET "i486p"]
[BITS 32]
      MOV ECX, MSG
      MOV EDX, 1
PUTLOOP:
      MOV AL, [CS:ECX]
      CMP AL, 0
      JE FIN
      INT 0X40
      ADD ECX, 1
      JMP PUTLOOP
FIN:
      RETF
MSG:
      DB "hello",0