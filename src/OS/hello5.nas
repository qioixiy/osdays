[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "hello5.nas"]

      GLOBAL _HariMain

[SECTION .text]
_HariMain:
      MOV EDX, 2
      MOV EBX, MSG
      INT 0X40		;ϵͳ����
      MOV EDX, 4
      INT 0X40		;ϵͳ����
      
[SECTION .data]
MSG:
      DB "hello5.nas", 0x0a, 0
