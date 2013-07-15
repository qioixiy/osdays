[INSTRSET "i486p"]

[BITS 32]
      MOV EDX, 2
      MOV EBX, MSG
      INT 0X40		;系统调用
      MOV EDX, 4
      INT 0X40		;系统调用
      
MSG:
      DB "hello2.nas",0