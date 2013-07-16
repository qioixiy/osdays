;调用修改过的API
[INSTRSET "i486p"]
[BITS 32]
      MOV EDX, 12345678
      INT 0X40
      MOV EDX, 4
      INT 0X40