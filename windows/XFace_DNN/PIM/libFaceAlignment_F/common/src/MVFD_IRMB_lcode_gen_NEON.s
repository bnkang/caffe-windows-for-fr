	@===========================================================================
    @ARM
    @REQUIRE8
    @PRESERVE8
    
    .global MVFD_IRMB_lcode_Gen_arm
    @AREA FACE_DETECTION, CODE, READONLY, ALIGN=4
 
 @void MVFD_IRMB_lcode_Gen_arm(int *fpos, unsigned char *p, int valid_w, unsigned short **wp)

    .section	.text
    .align	4
 
 
MVFD_IRMB_lcode_Gen_arm: 
			PUSH     {r4-r11}                                                                                      
			SUB      r11,r2,#0x10                                                                                  
			@CMP      r11,#0                                                                                        
			VPUSH    {d8-d15}                                                                                      
			LDR      r6,[r0,#0xc]                                                                                  
			LDR      r12,[r0,#0]                                                                                   
			LDR      r4,[r0,#4]                                                                                    
			ADD      r8,r6,r1                                                                                      
			LDR      r5,[r0,#8]                                                                                    
			ADD      r12,r12,r1                                                                                    
			LDR      r7,[r0,#0x10]                                                                                 
			ADD      r4,r4,r1                                                                                      
			LDR      r6,[r0,#0x14]                                                                                 
			ADD      r5,r5,r1                                                                                      
			ADD      r9,r7,r1                                                                                      
			ADD      r10,r6,r1                                                                                     
			@BLE      MVFD_IRMB_lcode_Gen_arm_skiploop ;0x0000fc38; MVFD_IRMB_lcode_Gen(int*, unsigned char*, int, unsigned short**) + 0x114         
			MOV      r7,r11
			LDR      r6,[r3,#0]

MVFD_IRMB_lcode_Gen_arm_loopback:
			VLD1.8   {d0,d1},[r12]!		@i0
			VLD1.8   {d18,d19},[r4]! 	@i1
			@ADD      r7,r7,#0x10
			SUBS      r7,r7,#0x10
			VLD1.8   {d16,d17},[r5]!	@i2
			@CMP      r7,r11
			VLD1.8   {d2,d3},[r8]!    	@i3
			VLD1.8   {d4,d5},[r9]!   	@i4
			VLD1.8   {d6,d7},[r10]!		@i5
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
			@VST2.8   {d22,d24},[r6]!	@ Store the lcode
			@VST2.8   {d23,d25},[r6]!
			VST2.8   {q11,q12},[r6]!	@ Store the lcode
			BPL      MVFD_IRMB_lcode_Gen_arm_loopback@0x0000fb6c ; MVFD_IRMB_lcode_Gen(int*, unsigned char*, int, unsigned short**) + 0x48          
			STR      r6,[r3,#0]
			RSB		 r7,r7,#0
			ADD 	 r7,r11,r7
MVFD_IRMB_lcode_Gen_arm_skiploop:
			ADD      r1,r1,r2                                                                                      
			LDR      r12,[r0,#0xc]                                                                                 
			SUB      r8,r1,#0x10                                                                                   
			LDR      r1,[r0,#0]                                                                                    
			LDR      r4,[r0,#0x10]                                                                                 
			SUB      r2,r2,r7                                                                                      
			ADD      r6,r1,r8                                                                                      
			LDR      r1,[r0,#4]                                                                                    
			ADD      r12,r12,r8                                                                                    
			ADD      r4,r4,r8                                                                                      
			ADD      r5,r1,r8                                                                                      
			LDR      r1,[r0,#8]                                                                                    
			LDR      r0,[r0,#0x14]                                                                                 
			MVN      r7,#0x1f                                                                                      
			ADD      r1,r1,r8                                                                                      
			ADD      r2,r7,r2,LSL #1                                                                               
			ADD      r0,r0,r8                                                                                      
			LDR      r8,[r3,#0]                                                                                    
			VMOV.I8  q6,#0x2                                                                                       
			ADD      r2,r2,r8                                                                                      
			@STR      r2,[r3,#0]                                                                                    
			VLD1.8   {d2,d3},[r5]                                                                                  
			VLD1.8   {d0,d1},[r6]                                                                                  
			VCGE.U8  q11,q0,q1                                                                                     
			VLD1.8   {d4,d5},[r1]                                                                                  
			VCGE.U8  q12,q0,q2                                                                                     
			VLD1.8   {d6,d7},[r12]                                                                                 
			VLD1.8   {d18,d19},[r0]                                                                                
			@ADD      r0,r2,#0x10                                                                                   
			VBIT     q11,q12,q6                                                                                    
			VLD1.8   {d16,d17},[r4]                                                                                
			VCGE.U8  q10,q1,q9                                                                                     
			VCGE.U8  q12,q2,q3                                                                                     
			VBIT     q10,q12,q6                                                                                    
			VCGE.U8  q12,q0,q3                                                                                     
			VSHL.I8  q6,q6,#1                                                                                      
			VBIT     q11,q12,q6                                                                                    
			VCGE.U8  q12,q2,q8                                                                                     
			VBIT     q10,q12,q6                                                                                    
			VCGE.U8  q12,q0,q8                                                                                     
			VSHL.I8  q6,q6,#1                                                                                      
			VBIT     q11,q12,q6                                                                                    
			VCGE.U8  q12,q2,q9                                                                                     
			VBIT     q10,q12,q6                                                                                    
			VCGE.U8  q12,q0,q9                                                                                     
			VSHL.I8  q6,q6,#1                                                                                      
			VBIF     q12,q11,q6                                                                                    
			VCGE.U8  q11,q3,q8                                                                                     
			VSHL.I8  q0,q6,#1                                                                                      
			VBIF     q11,q10,q6                                                                                    
			VCGE.U8  q6,q1,q2                                                                                      
			VCGE.U8  q10,q3,q9                                                                                     
			VBIF     q6,q12,q0                                                                                     
			VBIF     q10,q11,q0                                                                                    
			VSHL.I8  q2,q0,#1                                                                                      
			VCGE.U8  q0,q1,q3                                                                                      
			VCGE.U8  q1,q1,q8                                                                                      
			VBIT     q6,q0,q2                                                                                      
			VCGE.U8  q0,q8,q9                                                                                      
			VBIT     q10,q0,q2                                                                                     
			VSHL.I8  q0,q2,#1                                                                                      
			VBIC     q7,q10,q0                                                                                     
			VBIT     q6,q1,q0         
			VST2.8	 {q6,q7},[r2]
			@VST2.8   {d12,d14},[r2]                                                                                
			@VST2.8   {d13,d15},[r0]                                                                                
			@LDR      r0,[r3,#0]                                                                                    
			ADD      r0,r2,#0x20                                                                                   
			STR      r0,[r3,#0]                                                                                    
			VPOP     {d8-d15}                                                                                      
			POP      {r4-r11}                                                                                      
			BX       lr                                                                                            

	.end
			