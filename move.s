		AREA move, CODE, READONLY
		EXPORT setMove
			
player	RN	0
mode	RN	1
dir		RN	2
py		RN	3
px		RN 	4
			
setMove PROC
	
		MOV r12, sp
		PUSH {r4-r8,r10-r11,lr}		
		LDRB px, [r12]
		
		LSL	player, #4
		ADDS r0, player, mode
		LSL r0, #4
		ADDS r0, r0, dir
		LSL r0, #8
		ADDS r0, r0, py
		LSL r0, #8
		ADDS r0, r0, px
		
		POP {r4-r8,r10-r11,pc}
		
        ENDP