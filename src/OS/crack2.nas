[INSTRSET "i486p"]
[BITS 32]
      MOV EAX, 1*8	;os�õĶκ�
      MOV DS, AX	;�������DS
      MOV BYTE [0X102600], 0
      RETF
