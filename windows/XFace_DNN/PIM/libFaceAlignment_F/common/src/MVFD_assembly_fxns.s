	@===========================================================================
    @ARM
    @REQUIRE8
    @PRESERVE8
    
    .global MVFD_IsFaceIRMB_pxl_buf_gen
    .global MVFD_IsFaceIRMBConf
    .global MVFD_IsFaceSIMD8PConf
    .global MVFD_integral_image_gen_part_arm
    .global MVFD_ImageTranspose_arm
    .global MVFD_ImageTranspose_neon
    .global MVFD_IsFaceIRMB
    .global MVFD_IsFaceMCT5x5
    
    @AREA FACE_DETECTION, CODE, READONLY, ALIGN=4
    	.text
        
@void MVFD_IsFaceIRMB_pxl_buf_gen(unsigned char *irmb2_buf,int *ffpos,unsigned char *code_image_buf,int fnum);

		@r0 - irmb2_buf
		@r1 - ffpos
		@r2 - code_image_buf
		@r3 - fnum

.align 4
MVFD_IsFaceIRMB_pxl_buf_gen:
 		PUSH     {r4-r9,r14}              
		MOV		r14, r2
		ADD		r14,r14,#1
MVFD_IsFaceIRMB_pxl_buf_gen_loopback:
		LDM 	R1!,{R4-R5}
		SUBS 	r3,r3,#1
		LDM 	R1!,{R6-R7}
		LDRB	R4 ,[R0,R4]
		LDM 	R1!,{R8-R9}
		LDRB	R5 ,[R0,R5]
		LDRB	R6 ,[R0,R6]
		LDRB	R7 ,[R0,R7]
		LDRB	R8 ,[R0,R8]
		LDRB	R9 ,[R0,R9]

		STRB	R4,[r2],#2
		STRB	R5,[r14],#2
		STRB	R6,[r2],#2
		STRB	R7,[r14],#2
		STRB	R8,[r2],#2
		STRB	R9,[r14],#2
		
		BNE		MVFD_IsFaceIRMB_pxl_buf_gen_loopback
		
		POP     {r4-r9,r15} 
		@BX       lr  





@int MVFD_IsFaceIRMBConf(unsigned short *lut,const short *irmb2_table,unsigned short *code_image_buf,intfnum_bkp);
        	@r0 - lut
		@r1 - irmb2_table
		@r2 - code_image_buf
		@r3 - fnum
MVFD_IsFaceIRMBConf:
		PUSH     {r4-r9,r14} 
					
		AND r14,r3,#3
		ASR	R4,R3,#2
		MOV 	R9, #0
		MOV	R3, #0xFE
		ORR	R3,R3, #0xFF00
		LSL	R3,R3,#1
MVFD_IsFaceIRMBConf_loopback:

		LDRH r5,[r2],#2
		SUBS r4,r4,#1
		LDRH r6,[r2],#2
		AND 	R5,r3,R5, LSL #2
		LDRH r7,[r2],#2
		AND 	R6,r3,R6, LSL #2
		LDRH r8,[r2],#2
		AND 	R7,r3,R7, LSL #2
		LDRH	R5 ,[R1,R5]
		AND 	R8,r3,R8, LSL #2

		LDRH	R6 ,[R1,R6]
		LDRH	R7 ,[R1,R7]
		LDRH	R8 ,[R1,R8]
		
		LSL 	R5,R5,#1
		LSL 	R6,R6,#1
		LSL 	R7,R7,#1
		LSL 	R8,R8,#1
		
		LDRH	R5 ,[R0,R5]
		ADD		R0, R0, #0x5A0
		ADD 	R9, R9, R5
				
		LDRH	R6 ,[R0,R6]
		ADD		R0, R0, #0x5A0
		ADD 	R9, R9, R6
		
		LDRH	R7 ,[R0,R7]
		ADD		R0, R0, #0x5A0
		ADD 	R9, R9, R7
		
		LDRH	R8 ,[R0,R8]
		ADD		R0, R0, #0x5A0
		ADD 	R9, R9, R8
		
		BNE	MVFD_IsFaceIRMBConf_loopback
		
		CMP r14, #0
		BEQ 	MVFD_IsFaceIRMBConf_end
MVFD_IsFaceIRMBConf_loopback2:
		SUBS 	r14,r14,#1
		LDRH 	r5,[r2],#2
		AND 	R5,r3,R5, LSL #2
		LDRH	R5 ,[R1,R5]
		LSL 	R5,R5,#1
		LDRH	R5 ,[R0,R5]
		ADD		R0, R0, #0x5A0
		ADD 	R9, R9, R5
		BNE	MVFD_IsFaceIRMBConf_loopback2
MVFD_IsFaceIRMBConf_end:
		MOV 	R0,R9
		POP     {r4-r9,r15} 
        
        
@int MVFD_IsFaceSIMD8PConf(lut,(unsigned char *)code_image_buf,fnum);
	@r0 - lut
	@r1 - code_image_buf
	@r2 - fnum
.align 4
MVFD_IsFaceSIMD8PConf:
        PUSH     {r4-r9,r14} 
					
	AND 	R14,R2,#3
	ASR	R4,R2,#2
        MOV 	R9, #0
	MOV	R2, #0xFF
	LSL	R2,R2,#1
MVFD_IsFaceSIMD8PConf_loopback:
        @LDRB 	R5,[R1], #1
        @LDRB 	R6,[R1], #1
        @LDRB 	R7,[R1], #1
        @LDRB 	R8,[R1], #1
        
        LDR	R8,[R1], #4
        SUBS	R4,R4,#1
	
	AND	R5,R2,R8,LSL #1
	AND	R6,R2,R8,LSR #7
	AND	R7,R2,R8,LSR #0xF
	AND	R8,R2,R8,LSR #0x17
        
        LDRH	R5 ,[R0,R5]
        ADD	R0, R0, #0x200
        ADD 	R9, R9, R5
        
        LDRH	R6 ,[R0,R6]
        ADD	R0, R0, #0x200
        ADD 	R9, R9, R6
        
        LDRH	R7 ,[R0,R7]
        ADD	R0, R0, #0x200
        ADD 	R9, R9, R7
        
        LDRH	R8 ,[R0,R8]
        ADD	R0, R0, #0x200
        ADD 	R9, R9, R8
        
        BNE	MVFD_IsFaceSIMD8PConf_loopback
        
        CMP 	r14, #0
	BEQ 	MVFD_IsFaceSIMD8PConf_end
        
MVFD_IsFaceSIMD8PConf_loopback2:
	SUBS 	r14,r14,#1
	LDRB 	r5,[r1], #1
	LSL 	R5,R5,#1
	LDRH	R5 ,[R0,R5]
        ADD		R0, R0, #0x200
        ADD 	R9, R9, R5
	BNE		MVFD_IsFaceSIMD8PConf_loopback2
		
MVFD_IsFaceSIMD8PConf_end:
	MOV 	R0,R9
	POP     {r4-r9,r15} 


@void MVFD_integral_image_gen_part_arm(unsigned char * ipos, int * p0, int * p1, int width);
	@ r0 - ipos
	@ r1 - p0
	@ r2 - p1
	@ r3 - width
	
		@for(int x = width; x ; --x)
		@{
		@	s = s + *(ipos++);
		@	*(p0++) = s + *(p1++);
		@}
	
MVFD_integral_image_gen_part_arm:
		PUSH     {r4-r11, r14} 
		
		AND		R4, R3, #7
		ASR 	R3, R3, #3
		
		CMP 	R3, #0
		BEQ		MVFD_integral_image_gen_part_arm_skip_loop1
		
		EOR		R5, R5, R5 @ s = 0
MVFD_integral_image_gen_part_arm_loopback:

		LDRB	R10, [R0], #1
		LDM 	R2!, {R6, R7} 
		LDRB	R11, [R0], #1
		SUBS 	R3, R3, #1
		LDRB	R12, [R0], #1
		LDM 	R2!, {R8, R9}
		LDRB	R14, [R0], #1

		ADD		R5, R5, R10
		ADD     R10, R5, R6
		@STR		R10, [R1]!
	
		ADD		R5, R5, R11
		ADD     R11, R5, R7
		@STR		R11, [R1]!
		STM		R1!,{R10, R11}

		ADD		R5, R5, R12
		ADD     R12, R5, R8
		@STR		R12, [R1]!
		
		ADD		R5, R5, R14
		ADD     R14, R5, R9
		@STR		R13, [R1]!
		STM		R1!,{R12, R14}
		
		LDRB	R10, [R0], #1
		LDM 	R2!, {R6, R7} 
		LDRB	R11, [R0], #1
		LDM 	R2!, {R8, R9}
		LDRB	R12, [R0], #1
		ADD		R5, R5, R10
		ADD     R10, R5, R6
		LDRB	R14, [R0], #1

		@STR		R10, [R1]!
	
		ADD		R5, R5, R11
		ADD     R11, R5, R7
		@STR		R11, [R1]!
		STM		R1!,{R10, R11}

		ADD		R5, R5, R12
		ADD     R12, R5, R8
		@STR		R12, [R1]!
		
		ADD		R5, R5, R14
		ADD     R14, R5, R9
		@STR		R13, [R1]!
		STM		R1!,{R12, R14}
		
		BNE		MVFD_integral_image_gen_part_arm_loopback
MVFD_integral_image_gen_part_arm_skip_loop1:
		
		CMP 	R4, #0
		BEQ		MVFD_integral_image_gen_part_arm_skip_loop2

MVFD_integral_image_gen_part_arm_loopback2:
		SUBS	R4, R4, #1
		LDRB	R6, [R0], #1
		LDR		R7, [R2], #4
		ADD		R5, R5, R6
		ADD     R6,	R5, R7
		STR		R6, [R1], #4
		BNE		MVFD_integral_image_gen_part_arm_loopback2

MVFD_integral_image_gen_part_arm_skip_loop2:

		POP 	{r4-r11,r15}


@void MVFD_ImageTranspose_arm(unsigned char* src_data, int src_w, int src_h, unsigned char* rot_data)
@ Note : In MVFD src_w and src_h are always greater then 40 hence no check is added
	@unsigned char *sp = src_data + src_w*src_h -1;
	@unsigned char *rp_back = rot_data + src_w*src_h -1;
	@for(int y = src_h; y; y--)
	@{
	@	int x = src_w;
	@	unsigned char *rp = rp_back;
	@	while(x--)
	@	{	
	@		*rp = *sp--;
	@		rp -= src_h;
	@	}
	@	rp_back --;
	@}
	
	@ r0 - src_data
	@ r1 - src_w
	@ r2 - src_h
	@ r3 - rot_data
	
MVFD_ImageTranspose_arm:
		PUSH	{r4,r14}
		MUL 	R4, R1, R2
		PUSH	{r5-r10}
		SUB		R4, R4, #1
		
		ADD		R0, R4, R0		@ sp
		ADD		R3, R4, R3		@ rp
		MOV		R4, R2			@src_h
		AND		R9, R1, #7
		ASR 	R1, R1, #3
MVFD_ImageTranspose_arm_Outer_loop:
	
		MOV 	R6, R3
		
		@ Not Required because src_w is always greater then 40
		@CMP 	R1,	#0
		@BEQ		MVFD_ImageTranspose_arm_Skip_Inner_loop1
		MOV		R5, R1		@ Inner Loop Cnt

MVFD_ImageTranspose_arm_Inner_loop1:

		LDRB	R7, [R0], #-1
		SUBS	R5, R5, #1
		STRB	R7,	[R6], - R4
		LDRB	R7, [R0], #-1
		STRB	R7,	[R6], - R4
		LDRB	R7, [R0], #-1
		STRB	R7,	[R6], - R4
		LDRB	R7, [R0], #-1
		STRB	R7,	[R6], - R4
		LDRB	R7, [R0], #-1
		STRB	R7,	[R6], - R4
		LDRB	R7, [R0], #-1
		STRB	R7,	[R6], - R4
		LDRB	R7, [R0], #-1
		STRB	R7,	[R6], - R4
		LDRB	R7, [R0], #-1
		STRB	R7,	[R6], - R4
		
		BNE 	MVFD_ImageTranspose_arm_Inner_loop1
MVFD_ImageTranspose_arm_Skip_Inner_loop1:
		CMP 	R9,	#0
		BEQ		MVFD_ImageTranspose_arm_Skip_Inner_loop2
		MOV		R8, R9
MVFD_ImageTranspose_arm_Inner_loop2:

		LDRB	R7, [R0], #-1
		SUBS	R8, R8, #1
		STRB	R7,	[R6], - R4
		BNE 	MVFD_ImageTranspose_arm_Inner_loop2
MVFD_ImageTranspose_arm_Skip_Inner_loop2:
		SUB 	R3, R3, #1
		SUBS	R2, R2, #1
		BNE		MVFD_ImageTranspose_arm_Outer_loop
		POP	{r5-r10}
		POP	{r4,r15}
		
		
@void MVFD_ImageTranspose_neon(unsigned char *pSrc, int src_w, int src_h, unsigned char *pDes)
@      r0  =  pSrc			r10 = src_w-1
@      r1  =  src_w         r11 = temp  
@      r2  =  src_h         r12 = -src_h
@      r3  =  pDes            
@      r4  =  i = src_h/16              
@      r5  =  j = src_w/16
@      r6  =  ps
@      r7  =  pd
@      r8  =  ps2
@      r9  =  pd2
MVFD_ImageTranspose_neon:
	PUSH     {r4-r12,lr}
	VPUSH    {d8-d15}
	SUB      r10,r1,#1	@ src_w-1
	MOV	 r12,r2	
	LSR      r4,r2,#4       @i=src_h/16
	
MVFD_ImageTranspose_neon_i_loop:
	MOV	r6, r0		@ ps = pSrc
	MOV	r7, r3		@ pd = pDes
	LSR	r5,r1,#4	@ j=src_w/16
		
MVFD_ImageTranspose_neon_j_loop:
		MOV		r8,r6			@ ps2 = ps
		MOV		 r9,r7			@ pd2 = pd
		VLD1.U8	 {q0},[r8],r1	@ Q0 , ps2+=src_w
		VLD1.U8	 {q1},[r8],r1	@ Q1 , ps2+=src_w
		VLD1.U8	 {q2},[r8],r1	@ Q2 , ps2+=src_w
		VLD1.U8	 {q3},[r8],r1	@ Q3 , ps2+=src_w
		VLD1.U8	 {q4},[r8],r1	@ Q4 , ps2+=src_w
		VLD1.U8	 {q5},[r8],r1	@ Q5 , ps2+=src_w
		VLD1.U8	 {q6},[r8],r1	@ Q6 , ps2+=src_w
		VLD1.U8	 {q7},[r8],r1	@ Q7 , ps2+=src_w
		VLD1.U8	 {q8},[r8],r1	@ Q8 , ps2+=src_w
		VLD1.U8	 {q9},[r8],r1	@ Q9 , ps2+=src_w
		VLD1.U8	 {q10},[r8],r1	@ Q10 , ps2+=src_w
		VLD1.U8	 {q11},[r8],r1	@ Q11 , ps2+=src_w
		VLD1.U8	 {q12},[r8],r1	@ Q12 , ps2+=src_w
		VLD1.U8	 {q13},[r8],r1	@ Q13 , ps2+=src_w
		VLD1.U8	 {q14},[r8],r1	@ Q14 , ps2+=src_w
		VLD1.U8	 {q15},[r8]	@ Q15 
		
		VTRN.U8 Q0, Q1
		VTRN.U8 Q2, Q3
		VTRN.U8 Q4, Q5
		VTRN.U8 Q6, Q7
		VTRN.U8 Q8, Q9
		VTRN.U8 Q10, Q11
		VTRN.U8 Q12, Q13
		VTRN.U8 Q14, Q15

		VTRN.U16 Q0, Q2
		VTRN.U16 Q1, Q3
		VTRN.U16 Q4, Q6
		VTRN.U16 Q5, Q7
		VTRN.U16 Q8, Q10
		VTRN.U16 Q9, Q11
		VTRN.U16 Q12, Q14
		VTRN.U16 Q13, Q15

		VTRN.U32 Q0, Q4
		VTRN.U32 Q1, Q5
		VTRN.U32 Q2, Q6
		VTRN.U32 Q3, Q7
		VTRN.U32 Q8, Q12
		VTRN.U32 Q9, Q13
		VTRN.U32 Q10, Q14
		VTRN.U32 Q11, Q15

		VSWP d1,d16
		VSWP d3,d18
		VSWP d5,d20
		VSWP d7,d22
		VSWP d9,d24
		VSWP d11,d26
		VSWP d13,d28
		VSWP d15,d30		

		VST1.U8	 {q0},[r9],r12	@ Q0 , pd2+=src_h
		VST1.U8	 {q1},[r9],r12	@ Q1 , pd2+=src_h
		VST1.U8	 {q2},[r9],r12	@ Q2 , pd2+=src_h
		VST1.U8	 {q3},[r9],r12	@ Q3 , pd2+=src_h
		VST1.U8	 {q4},[r9],r12	@ Q4 , pd2+=src_h
		VST1.U8	 {q5},[r9],r12	@ Q5 , pd2+=src_h
		VST1.U8	 {q6},[r9],r12	@ Q6 , pd2+=src_h
		VST1.U8	 {q7},[r9],r12	@ Q7 , pd2+=src_h
		VST1.U8	 {q8},[r9],r12	@ Q8 , pd2+=src_h
		VST1.U8	 {q9},[r9],r12	@ Q9 , pd2+=src_h
		VST1.U8	 {q10},[r9],r12	@ Q10 , pd2+=src_h
		VST1.U8	 {q11},[r9],r12	@ Q11 , pd2+=src_h
		VST1.U8	 {q12},[r9],r12	@ Q12 , pd2+=src_h
		VST1.U8	 {q13},[r9],r12	@ Q13 , pd2+=src_h
		VST1.U8	 {q14},[r9],r12	@ Q14 , pd2+=src_h
		VST1.U8	 {q15},[r9]	@ Q15 	

		
		ADD		 r7,r9,r2		@ pd=pd2+src_h
		ADD		 r6,r6,#16		@ ps+=16
		SUBS		 r5,r5,#1		@ j--
@		CMP		 r5,#0			@ j>0
		BGT		 MVFD_ImageTranspose_neon_j_loop
		
		ADD		 r0,r0,r1,LSL #4	@ pSrc+=src_w*16
		ADD		 r3,r3,#16		@ pDes+=16
		SUBS		 r4,r4,#1		@ i--
;		CMP		 r4,#0			@ i>0
		BGT		 MVFD_ImageTranspose_neon_i_loop
		
MVFD_ImageTranspose_neon_exit:
		VPOP    {d8-d15}    
		POP      {r4-r12,pc}

@----------------------------------------------------------------------------------------

@int MVFD_IsFaceMCT5x5(unsigned char * mct_5x5_buf,int *ffpos,(unsigned char *)code_image_buf,
@				int fnum_By_16, unsigned short * lut, int width);

	@	R0-	mct_5x5_buf
	@	R1- ffpos
	@	R2- code_image_buf
	@	R3- fnum_By_16
	
MVFD_IsFaceMCT5x5:
		PUSH	{r4-r11,r14}
		VPUSH	{d8-d15}
		
		EOR		R10,R10,R10	@for Confidence
		LDR		R11,[sp, #0x64]	@ lut
		
		LDR		R9,[sp, #0x68] @width
		LSL		R9, R9, #1
MVFD_IsFaceMCT5x5_loop:
		MOV		R12, #0x10
		MOV		R14, R2
		
		MOV		R6, #0x55
		MOV		R7, #0x33
		MOV		R8, #0x0F
		VDUP.8	Q10,	R6
		VDUP.8	Q11,	R7
		VDUP.8	Q12,	R8
		
MVFD_IsFaceMCT5x5_pxl_gen_loopback:
		LDM 	R1!,{R4-R5}
		SUBS 	R12,R12,#2

		ADD		R4, R0, R4
		ADD		R5, R0, R5
		
		LDRB	R6 ,[R4]
		LDRB	R7 ,[R4, #2]
		LDRB	R8 ,[R4, #4]
		STRB	R6,[R14]
		STRB	R7,[R14, #0x10]
		STRB	R8,[R14, #0x20]
		LDRB	R6 ,[R5]
		LDRB	R7 ,[R5, #2]
		LDRB	R8 ,[R5, #4]
		STRB	R6,[R14, #0x1]
		STRB	R7,[R14, #0x11]
		STRB	R8,[R14, #0x21]
		
		ADD		R4, R9, R4
		ADD		R5, R9, R5
		LDRB	R6 ,[R4]
		LDRB	R7 ,[R4, #2]
		LDRB	R8 ,[R4, #4]
		STRB	R6,[R14, #0x30]
		STRB	R7,[R14, #0x40]
		STRB	R8,[R14, #0x50]
		LDRB	R6 ,[R5]
		LDRB	R7 ,[R5, #2]
		LDRB	R8 ,[R5, #4]
		STRB	R6,[R14, #0x31]
		STRB	R7,[R14, #0x41]
		STRB	R8,[R14, #0x51]
		
		ADD		R4, R9, R4
		ADD		R5, R9, R5
		LDRB	R6 ,[R4]
		LDRB	R7 ,[R4, #2]
		LDRB	R8 ,[R4, #4]
		STRB	R6,[R14, #0x60]
		STRB	R7,[R14, #0x70]
		STRB	R8,[R14, #0x80]
		LDRB	R6 ,[R5]
		LDRB	R7 ,[R5, #2]
		LDRB	R8 ,[R5, #4]
		STRB	R6,[R14, #0x61]
		STRB	R7,[R14, #0x71]
		STRB	R8,[R14, #0x81]

		ADD		R14, R14, #2

		BNE		MVFD_IsFaceMCT5x5_pxl_gen_loopback


		MOV 	R14, R2	@ code_image_buf
		VLD1.U8	{Q1, Q2}, [R14]!
		VLD1.U8	{Q3, Q4}, [R14]!
		VLD1.U8	{Q5, Q6}, [R14]!
		VLD1.U8	{Q7, Q8}, [R14]!
		VLD1.U8	{Q9}, [R14]!

		@t1_16x8 = vaddl_u8(vget_low_u8(m1_8x16), vget_low_u8(m2_8x16));
		VADDL.U8	Q0,	D2,	D4	
		@Vector wide add: vaddw_<type>. Vr[i]:=Va[i]+Vb[i]
		@t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m3_8x16));
		VADDW.U8	Q0,	Q0,	D6
		@t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m4_8x16));
		VADDW.U8	Q0,	Q0,	D8
		@t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m5_8x16));
		VADDW.U8	Q0,	Q0,	D10
		@t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m6_8x16));
		VADDW.U8	Q0,	Q0,	D12
		@t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m7_8x16));
		VADDW.U8	Q0,	Q0,	D14
		@t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m8_8x16));
		VADDW.U8	Q0,	Q0,	D16
		@t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m9_8x16));
		VADDW.U8	Q0,	Q0,	D18
		
		@Max value of t can be 2304(256*9)
		@Hard Code Method for Integer division by 9(Tested for values less then 2310)
		@=(B3*7281+7280)/65536   //(t*455+454)/4096 - Old;
		
		@const_7281_16x4 = vdup_n_u16(7281);
		@const_7280_32x4 = vdupq_n_u32(7280);
		MOV 	R6, #0x70
		ORR		R6, #0x1C00
		ADD		R7, R6, #1
		VDUP.U16	D26,	R7
		VDUP.U32	Q14,	R6
		
		@t11_32x4 = vmull_u16(vget_low_u16(t1_16x8), const_7281_16x4);
		VMULL.U16	Q15, D26, D0
		
		@t12_32x4 = vmull_u16(vget_high_u16(t1_16x8), const_7281_16x4);
		VMULL.U16	Q0, D26, D1
		@Load all lanes of vector to the same literal value		
		@Vector add high half: vaddhn_<type>.Vr[i]:=Va[i]+Vb[i]
		@Combining vectors
		@t1_16x8 = vcombine_u16(vaddhn_u32(t11_32x4, const_7280_32x4),vaddhn_u32(t12_32x4, const_7280_32x4));
		VADDHN.U32	D30, Q15, Q14
		VADDHN.U32	D31, Q0, Q14
		
		@Splitting vectors
		@Vector long add: vaddl_<type>. Vr[i]:=Va[i]+Vb[i]
		@t2_16x8 = vaddl_u8(vget_high_u8(m1_8x16), vget_high_u8(m2_8x16));
		VADDL.U8	Q0,	D3,	D5
		@Vector wide add: vaddw_<type>. Vr[i]:=Va[i]+Vb[i]
		@t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m3_8x16));
		VADDW.U8	Q0,	Q0,	D7
		@t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m4_8x16));
		VADDW.U8	Q0,	Q0,	D9
		@t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m5_8x16));
		VADDW.U8	Q0,	Q0,	D11
		@t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m6_8x16));
		VADDW.U8	Q0,	Q0,	D13
		@t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m7_8x16));
		VADDW.U8	Q0,	Q0,	D15
		@t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m8_8x16));
		VADDW.U8	Q0,	Q0,	D17
		@t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m9_8x16));
		VADDW.U8	Q0,	Q0,	D19
		
		@t21_32x4 = vmull_u16(vget_low_u16(t2_16x8), const_7281_16x4);
		VMOV	D27, D1
		VMULL.U16	Q0, D26, D0
		
		@t22_32x4 = vmull_u16(vget_high_u16(t2_16x8), const_7281_16x4);
		VMULL.U16	Q13, D26, D27
		
		@Load all lanes of vector to the same literal value		
		@Vector add high half: vaddhn_<type>.Vr[i]:=Va[i]+Vb[i]
		@Combining vectors
		@t2_16x8 = vcombine_u16(vaddhn_u32(t21_32x4, const_7280_32x4),vaddhn_u32(t22_32x4, const_7280_32x4));
		VADDHN.U32	D0, Q0, Q14
		VADDHN.U32	D1, Q13, Q14
		
		@Vector narrow integer
		@Combining vectors
		@t_8x16 = vcombine_u8(vmovn_u16(t1_16x8),vmovn_u16(t2_16x8));
		VMOVN.U16 D1, Q0
		VMOVN.U16 D0, Q15
		
		
		@Vector compare less-than
		@Vector Compare Less Than takes the value of each element in a vector, and compares it with the
		@value of the corresponding element of a second vector. If the condition is true, the corresponding
		@element in the destination vector is set to all ones. Otherwise, it is set to all zeros.
		@m1_8x16 = vcltq_u8(t_8x16, m1_8x16);
		VCLT.U8		Q1, Q0, Q1
		@m2_8x16 = vcltq_u8(t_8x16, m2_8x16);
		VCLT.U8		Q2, Q0, Q2 
		@m3_8x16 = vcltq_u8(t_8x16, m3_8x16); 
		VCLT.U8		Q3, Q0, Q3
		@m4_8x16 = vcltq_u8(t_8x16, m4_8x16); 
		VCLT.U8		Q4, Q0, Q4
		@m5_8x16 = vcltq_u8(t_8x16, m5_8x16);
		VCLT.U8		Q5, Q0, Q5
		@m6_8x16 = vcltq_u8(t_8x16, m6_8x16); 
		VCLT.U8		Q6, Q0, Q6
		@m7_8x16 = vcltq_u8(t_8x16, m7_8x16); 
		VCLT.U8		Q7, Q0, Q7
		@m8_8x16 = vcltq_u8(t_8x16, m8_8x16); 
		VCLT.U8		Q8, Q0, Q8
		@m9_8x16 = vcltq_u8(t_8x16, m9_8x16);
		VCLT.U8		Q9, Q0, Q9
		
		@Bitwise Select
		@VBSL (Bitwise Select) selects each bit for the destination from the first operand if the
		@corresponding bit of the destination is 1, or from the second operand if the corresponding bit of
		@the destination is 0.
		@m2_8x16 = vbslq_u8(mask1_8x16, m1_8x16, m2_8x16); 
		VBIT	Q2, Q1, Q10
		@m4_8x16 = vbslq_u8(mask1_8x16, m3_8x16, m4_8x16); 
		VBIT	Q4, Q3, Q10
		@m6_8x16 = vbslq_u8(mask1_8x16, m5_8x16, m6_8x16); 
		VBIT	Q6, Q5, Q10
		@m8_8x16 = vbslq_u8(mask1_8x16, m7_8x16, m8_8x16); 
		VBIT	Q8, Q7, Q10

		@m4_8x16 = vbslq_u8(mask2_8x16, m2_8x16, m4_8x16); 
		VBIT	Q4, Q2, Q11
		@m8_8x16 = vbslq_u8(mask2_8x16, m6_8x16, m8_8x16); 
		VBIT	Q8, Q6, Q11

		@pmct_8x16x2.val[0] = vbslq_u8(mask3_8x16, m4_8x16, m8_8x16); 
		VBIT	Q8, Q4, Q12
		
		@Right Shift by a constant
		@pmct_8x16x2.val[1] = vshrq_n_u8(m9_8x16,7);
		VSHR.U8		Q9, Q9, #7
		
		@VST2.8   {Q8,Q9},[R2] @ Storing the 16 16(9)Bit result to code_image_buf
		
		@Multiplying by 2 as LUT is in short format
		VZIP.8		Q8, Q9
		VSHL.U16	Q8, Q8, #1
		VSHL.U16	Q9, Q9, #1
		
		VST1.U16	{Q8, Q9}, [R2]
		
		@code_image_buf	- R2
		@lut	- R11
		@Conf	- R10
		
		MOV		R14	, R2	@ code_image_buf
		MOV		R12, #0x10	@ counter
MVFD_IsFaceMCT5x5_Conf_arm_new_START:	
		LDM R14!,{R5,R6}
		SUBS 	R12,R12,#4
		
		LSR		R7, R5, #16
		LSL		R5, R5, #16
		LSR		R8, R6, #16
		LSL		R6, R6, #16
		LSR		R5, R5, #16
		LSR		R6, R6, #16
	
		LDRH	R5,[R11,R5]
		ADD R11,R11,#1024
		ADD	R10,R10,R5
		
		LDRH	R5,[R11,R7]
		ADD R11,R11,#1024
		ADD	R10,R10,R5
		
		LDRH	R5,[R11,R6]
		ADD R11,R11,#1024
		ADD	R10,R10,R5
		
		LDRH	R5,[R11,R8]
		ADD R11,R11,#1024
		ADD	R10,R10,R5
		
		BNE MVFD_IsFaceMCT5x5_Conf_arm_new_START
		
		SUBS 	R3,R3,#1	@fnum_by_16--
		BNE MVFD_IsFaceMCT5x5_loop
		
		VPOP    {d8-d15}
		MOV		R0, R10
		POP     {R4-R11,r15} 
		
		
		
		
		
@---------------------------------------------------------------------------------------
@int MVFD_IsFaceIRMB(unsigned char * irmb_image_buf,int *ffpos,(unsigned char *)code_image_buf,int fnum_By_16,
@				unsigned short * lut,int * irmb2_table);
	
	@	R0-	irmb_image_buf
	@	R1- ffpos
	@	R2- code_image_buf
	@	R3- fnum_By_16

MVFD_IsFaceIRMB:
 		PUSH     {r4-r11,r14}              
		VPUSH    {d8-d15}

		EOR		R10,R10,R10
		LDR		R11,[sp, #0x64]	@ lut
						
MVFD_IsFaceIRMB_loop:
		MOV		R12, #0x10
		MOV		R14, R2
MVFD_IsFaceIRMB_pxl_gen_loopback:
		LDM 	R1!,{R4-R5}
		SUBS 	R12,R12,#1
		LDM 	R1!,{R6-R7}
		LDRB	R4 ,[R0,R4]
		LDM 	R1!,{R8-R9}
		LDRB	R5 ,[R0,R5]
		LDRB	R6 ,[R0,R6]
		LDRB	R7 ,[R0,R7]
		
		STRB	R5,[R14, #0x10]
		LDRB	R8 ,[R0,R8]
		LDRB	R9 ,[R0,R9]

		
		STRB	R6,[R14, #0x20]
		STRB	R7,[R14, #0x30]
		STRB	R8,[R14, #0x40]
		STRB	R9,[R14, #0x50]
		STRB	R4,[R14],#1
		
		BNE		MVFD_IsFaceIRMB_pxl_gen_loopback
		
		@POP     {r4-r9,r15} 
		@BX       lr  
		
		ADD		R4, R2, #0x10
		ADD		R5, R2, #0x20
		ADD		R6, R2, #0x30
		ADD		R7, R2, #0x40
		ADD		R8, R2, #0x50
		
		VLD1.8   {d0,d1},[R2]		@i0
		VLD1.8   {d18,d19},[R4] 	@i1
		VLD1.8   {d16,d17},[R5]		@i2
		VLD1.8   {d2,d3},[R6]    	@i3
		VLD1.8   {d4,d5},[R7]   	@i4
		VLD1.8   {d6,d7},[R8]		@i5
		VCGE.U8  q11,q0,q9 		@CMP i0 i1
		VCGE.U8  q12,q9,q3  		@CMP i1 i5
		VCGE.U8  q15,q0,q8      	@CMP i0 i2
		VCGE.U8  q5,q8,q1   		@CMP i2 i3
		VMOV.I8  q4,#0x2		@dup Mask1
		VCGE.U8  q13,q0,q1  		@CMP i0 i3
		VCGE.U8  q14,q8,q2 		@CMP i2 i4
		VBIT     q11,q15,q4 		@Bitsel 10
		VBIT     q12,q5,q4 		@Bitsel 11
		VMOV.I8  q10,#0x4   		@dup Mask2
		VSHL.I8  q4,q4,#2		@Sift Mask1
		VCGE.U8  q15,q0,q2 		@CMP i0 i4
		VCGE.U8  q5,q8,q3 		@CMP i2 i5
		VBIT 	 q11,q13,q10		@Bitsel 8
		VBIT	 q12,q14,q10		@Bitsel 9
		VCGE.U8  q6,q0,q3		@CMP i0 i5
		VCGE.U8  q7,q1,q2 		@CMP i3 i4
		VBIT	 q11,q15,q4		@Bitsel 10
		VBIT	 q12,q5,q4		@Bitsel 11
		VSHL.I8  q10,q10,#2		@Shift Mask2
		VCGE.U8  q15,q9,q8 		@CMP i1 i2
		VCGE.U8  q5,q1,q3  		@CMP i3 i5
		VBIT	 q11,q6,q10		@Bitsel 14
		VBIT	 q12,q7,q10		@Bitsel 15
		VSHL.I8  q4,q4,#2		@Sift Mask1
		VCGE.U8  q13,q9,q1		@CMP i1, i3
		VCGE.U8  q14,q2,q3		@CMP i4, i5
		VBIT	 q11,q15,q4		@Bitsel 10
		VBIT	 q12,q5,q4		@Bitsel 11
		VSHL.I8  q10,q10,#2		@Shift Mask2
		VCGE.U8  q6,q9,q2 		@CMP i1, i4
		VBIT	 q12,q14,q10		@Bitsel 9
		VBIT	 q11,q13,q10		@Bitsel 8
		VSHL.I8  q4,q4,#2		@Sift Mask1
		VBIC	 q12,q12,q4		@Bit Clear
		VBIT	 q11,q6,q4		@Bitsel 14
		VST2.8   {q11,q12},[R2]		@Store the lcode
		
		@MVFD_IsFaceIRMBConf
		@r11 - lut
		@r5 - irmb2_table
		@r2 - code_image_buf
		
		LDR		R5,[sp, #0x68] @irmb2_table
		
		MOV		R14, R2

		LDRH 	R7,	[R2,#2]
		LDRH 	R8,	[r14,#4]
		LDRH 	R9,	[r14,#6]
		LDRH 	R6,	[R14],#8
		

		LDR		R7, [R5, R7, LSL #2]
		LDR		R8, [R5, R8, LSL #2]
		LDR		R6, [R5, R6, LSL #2]
		LDR		R9, [R5, R9, LSL #2]


		LSL 	R7,R7,#1
		LSL 	R6,R6,#1
		LSL 	R8,R8,#1
		LSL 	R9,R9,#1
		
		LDRH	R6 ,[R11,R6]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R6
		
		LDRH	R7 ,[R11,R7]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R7
		
		LDRH	R8 ,[R11,R8]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R8
		
		LDRH	R9 ,[R11,R9]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R9
		
		LDRH 	R7,	[r14,#2]
		LDRH 	R8,	[r14,#4]
		LDRH 	R9,	[r14,#6]
		LDRH 	R6,	[R14],#8
		

		LDR		R7, [R5, R7, LSL #2]
		LDR		R8, [R5, R8, LSL #2]
		LDR		R6, [R5, R6, LSL #2]
		LDR		R9, [R5, R9, LSL #2]


		LSL 	R7,R7,#1
		LSL 	R6,R6,#1
		LSL 	R8,R8,#1
		LSL 	R9,R9,#1
		
		LDRH	R6 ,[R11,R6]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R6
		
		LDRH	R7 ,[R11,R7]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R7
		
		LDRH	R8 ,[R11,R8]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R8
		
		LDRH	R9 ,[R11,R9]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R9
		
		LDRH 	R7,	[r14,#2]
		LDRH 	R8,	[r14,#4]
		LDRH 	R9,	[r14,#6]
		LDRH 	R6,	[R14],#8
		

		LDR		R7, [R5, R7, LSL #2]
		LDR		R8, [R5, R8, LSL #2]
		LDR		R6, [R5, R6, LSL #2]
		LDR		R9, [R5, R9, LSL #2]


		LSL 	R7,R7,#1
		LSL 	R6,R6,#1
		LSL 	R8,R8,#1
		LSL 	R9,R9,#1
		
		LDRH	R6 ,[R11,R6]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R6
		
		LDRH	R7 ,[R11,R7]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R7
		
		LDRH	R8 ,[R11,R8]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R8
		
		LDRH	R9 ,[R11,R9]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R9

		LDRH 	R7,	[r14,#2]
		LDRH 	R8,	[r14,#4]
		LDRH 	R9,	[r14,#6]
		LDRH 	R6,	[R14],#8
		
		LDR		R7, [R5, R7, LSL #2]
		LDR		R8, [R5, R8, LSL #2]
		LDR		R6, [R5, R6, LSL #2]
		LDR		R9, [R5, R9, LSL #2]


		LSL 	R7,R7,#1
		LSL 	R6,R6,#1
		LSL 	R8,R8,#1
		LSL 	R9,R9,#1
		
		LDRH	R6 ,[R11,R6]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R6
		
		LDRH	R7 ,[R11,R7]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R7
		
		LDRH	R8 ,[R11,R8]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R8
		
		LDRH	R9 ,[R11,R9]
		ADD		R11, R11, #0x5A0
		ADD 	R10, R10, R9		
		
		SUBS 	R3,R3,#1		@fnum_by_16--
		BNE		MVFD_IsFaceIRMB_loop

		VPOP    {d8-d15}
		MOV		R0, R10
		POP     {R4-R11,r15} 
		
		
	.end
        

