	@===========================================================================
    @ARM
    @REQUIRE8
    @PRESERVE8
    
    .global MVFD_IRMB_lcode_Gen_detect_arm
    @AREA FACE_DETECTION, CODE, READONLY, ALIGN=4
 
 @void MVFD_IRMB_lcode_Gen_detect_arm(int *fpos, unsigned char *p, int valid_w, unsigned short **wp)

    .section	.text
    .align	4
 

MVFD_IRMB_lcode_Gen_detect_arm:
			PUSH     {r4-r11,lr}                                                                                                                
			SUB      lr,r2,#0x20                                                                                                                
			CMP      lr,#0                                                                                                                      
			VPUSH    {d8-d15}                                                                                                                   
			LDR      r6,[r0,#0xc]                                                                                                               
			LDR      r12,[r0,#0]                                                                                                                
			LDR      r4,[r0,#4]                                                                                                                 
			ADD      r9,r6,r1                                                                                                                   
			LDR      r5,[r0,#8]                                                                                                                 
			ADD      r12,r12,r1                                                                                                                 
			LDR      r7,[r0,#0x10]                                                                                                              
			ADD      r4,r4,r1                                                                                                                   
			LDR      r6,[r0,#0x14]                                                                                                              
			ADD      r5,r5,r1                                                                                                                   
			LDR      r8,[r3,#0]                                                                                                                 
			ADD      r10,r7,r1                                                                                                                  
			ADD      r11,r6,r1                                                                                                                  
			@MOV      r7,#0                                                                                                                      
			BLE      MVFD_IRMB_lcode_Gen_detect_arm_skip@0x0000fc80 ; MVFD_IRMB_lcode_Gen(int*, unsigned char*, int, unsigned short**) + 0x15c         
			@MOV      r7,lr  
MVFD_IRMB_lcode_Gen_detect_arm_loopback:
			VLD2.8	 {q9,q10},[r4]!
			VLD2.8 	 {q0,q1},[r12]!
			VLD2.8 	 {q3,q4},[r11]!
			VLD2.8   {q10,q11},[r10]!				
			VLD2.8   {q13,q14},[r9]!
			VLD2.8 	 {q6,q7},[r5]!

			SUBS	 lr,lr,#0x20
			@ADD      r7,r7,#0x20       
			@CMP      r7,lr 

			VCGE.U8  q11,q0,q9 		@CMP i0 i1
			VCGE.U8  q12,q9,q3  		@CMP i1 i5
			VCGE.U8  q15,q0,q6      	@CMP i0 i2
			VCGE.U8  q5,q6,q13   		@CMP i2 i3
			VMOV.I8  q4,#0x2		@dup Mask1
			VCGE.U8  q1,q0,q13  		@CMP i0 i3
			VCGE.U8  q14,q6,q10 		@CMP i2 i4
			VBIT     q11,q15,q4 		@Bitsel 10
			VBIT     q12,q5,q4 		@Bitsel 11
			VMOV.I8  q2,#0x4   		@dup Mask2
			VSHL.I8  q4,q4,#2		@Sift Mask1
			VCGE.U8  q15,q0,q10 		@CMP i0 i4
			VCGE.U8  q5,q6,q3 		@CMP i2 i5
			VBIT 	 q11,q1,q2		@Bitsel 8
			VBIT	 q12,q14,q2		@Bitsel 9
			VCGE.U8  q7,q0,q3		@CMP i0 i5
			VCGE.U8  q8,q13,q10 		@CMP i3 i4
			VBIT	 q11,q15,q4		@Bitsel 10
			VBIT	 q12,q5,q4		@Bitsel 11
			VSHL.I8  q2,q2,#2		@Shift Mask2
			VCGE.U8  q15,q9,q6 		@CMP i1 i2
			VCGE.U8  q5,q13,q3  		@CMP i3 i5
			VBIT	 q11,q7,q2		@Bitsel 14
			VBIT	 q12,q8,q2		@Bitsel 15
			VSHL.I8  q4,q4,#2		@Sift Mask1
			VCGE.U8  q1,q9,q13		@CMP i1, i3
			VCGE.U8  q14,q10,q3		@CMP i4, i5
			VBIT	 q11,q15,q4		@Bitsel 10
			VBIT	 q12,q5,q4		@Bitsel 11
			VSHL.I8  q2,q2,#2		@Shift Mask2
			VCGE.U8  q7,q9,q10 		@CMP i1, i4
			VBIT	 q12,q14,q2		@Bitsel 9
			VBIT	 q11,q1,q2		@Bitsel 8
			VSHL.I8  q4,q4,#2		@Sift Mask1
			VBIC	 q12,q12,q4		@Bit Clear
			VBIT	 q11,q7,q4		@Bitsel 14
			VST2.8   {q11,q12},[r8]!
			BPL      MVFD_IRMB_lcode_Gen_detect_arm_loopback@0x0000fb70 ; MVFD_IRMB_lcode_Gen(int*, unsigned char*, int, unsigned short**) + 0x4c      

			@RSB		 r7,r7,#0
			@ADD 	 r7,lr,r7
MVFD_IRMB_lcode_Gen_detect_arm_skip:
			ADD      r1,r1,r2                                                                                                                   
			LDR      r12,[r0,#0x10]                                                                                                             
			SUB      r9,r1,#0x20                                                                                                                
			LDR      r1,[r0,#0]                                                                                                                 
			ADD      r12,r12,r9                                                                                                                 
			ADD      r6,r1,r9                                                                                                                   
			LDR      r1,[r0,#4]                                                                                                                 
			@SUB      r7,r2,r7,ASR #17                                                                                                           
			ADD      r5,r1,r9                                                                                                                   
			LDR      r1,[r0,#8]                                                                                                                 
			@ADD      r7,r8,r7,LSL #1                                                                                                            
			ADD      r4,r1,r9                                                                                                                   
			LDR      r1,[r0,#0xc]                                                                                                               
			LDR      r0,[r0,#0x14]                                                                                                              
			ADD      r1,r1,r9                                                                                                                   
			ADD      r0,r0,r9                                                                                                                   

			VLD2.8	 {q9,q10},[r5]
			VLD2.8 	 {q0,q1},[r6]
			VLD2.8 	 {q3,q4},[r0]
			VLD2.8   {q10,q11},[r12]
			VLD2.8   {q13,q14},[r1]
			VLD2.8 	 {q6,q7},[r4]

			LDR		r8,[sp,#0x64] 
			
			VCGE.U8  q11,q0,q9 		@CMP i0 i1
			VCGE.U8  q12,q9,q3  		@CMP i1 i5
			VCGE.U8  q15,q0,q6      	@CMP i0 i2
			VCGE.U8  q5,q6,q13   		@CMP i2 i3
			VMOV.I8  q4,#0x2		@dup Mask1
			VCGE.U8  q1,q0,q13  		@CMP i0 i3
			VCGE.U8  q14,q6,q10 		@CMP i2 i4
			VBIT     q11,q15,q4 		@Bitsel 10
			VBIT     q12,q5,q4 		@Bitsel 11
			VMOV.I8  q2,#0x4   		@dup Mask2
			VSHL.I8  q4,q4,#2		@Sift Mask1
			VCGE.U8  q15,q0,q10 		@CMP i0 i4
			VCGE.U8  q5,q6,q3 		@CMP i2 i5
			VBIT 	 q11,q1,q2		@Bitsel 8
			VBIT	 q12,q14,q2		@Bitsel 9
			VCGE.U8  q7,q0,q3		@CMP i0 i5
			VCGE.U8  q8,q13,q10 		@CMP i3 i4
			VBIT	 q11,q15,q4		@Bitsel 10
			VBIT	 q12,q5,q4		@Bitsel 11
			VSHL.I8  q2,q2,#2		@Shift Mask2
			VCGE.U8  q15,q9,q6 		@CMP i1 i2
			VCGE.U8  q5,q13,q3  		@CMP i3 i5
			VBIT	 q11,q7,q2		@Bitsel 14
			VBIT	 q12,q8,q2		@Bitsel 15
			VSHL.I8  q4,q4,#2		@Sift Mask1
			VCGE.U8  q1,q9,q13		@CMP i1, i3
			VCGE.U8  q14,q10,q3		@CMP i4, i5
			VBIT	 q11,q15,q4		@Bitsel 10
			VBIT	 q12,q5,q4		@Bitsel 11
			VSHL.I8  q2,q2,#2		@Shift Mask2
			VCGE.U8  q7,q9,q10 		@CMP i1, i4
			VBIT	 q12,q14,q2		@Bitsel 9
			VBIT	 q11,q1,q2		@Bitsel 8
			VSHL.I8  q4,q4,#2		@Sift Mask1
			VBIC	 q12,q12,q4		@Bit Clear
			VBIT	 q11,q7,q4		@Bitsel 14

			LDR      r4,[r3,#0]                                                                                                                 
			ADD      r4,r4,r2,LSL #1                                                                                                            
			STR      r4,[r3,#0]  
			
			VST2.8   {q11,q12},[r8]
			@VST2.8   {d20,d22},[r7]                                                                                                             
			@VST2.8   {d21,d23},[r0]                                                                                                             
                                                                                                               
			VPOP     {d8-d15}                                                                                                                   
			POP      {r4-r11,pc}                                                                                                                

	.end
			
