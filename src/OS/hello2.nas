[INSTRSET "i486p"]

[BITS 32]
      MOV EDX, 2
      MOV EBX, MSG
      INT 0X40		;ϵͳ����
      MOV EDX, 4
      INT 0X40		;ϵͳ����
      
MSG:
      DB "hello2.nas",0