[INSTRSET "i486p"]

[BITS 32]
      MOV EDX, 2
      MOV EBX, MSG
      INT 0X40
      RETF
      
MSG:
      DB "hello",0