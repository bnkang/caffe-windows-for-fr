	@===========================================================================
    @ARM
    @REQUIRE8
    @PRESERVE8
    
    .global MVFD_IRMB_longtoshort_arm_new
    @AREA FACE_DETECTION, CODE, READONLY, ALIGN=4
	.text
        
@void MVFD_IRMB_longtoshort_arm_new(const int *irmb2_table, unsigned short *p, int xcnt, int remain );
	.align 4
MVFD_IRMB_longtoshort_arm_new:
			@r0	irmb2_table
			@r1	p
			@r2	xcnt
			@r3 remain
			PUSH	{r4-r9,r14}

			MOV 	r7, r1
			ADD     r7,r7,#2
			MOV 	r8, r1
			MOV 	r9, r7

			CMP r2, #0
			BEQ MVFD_IRMB_longtoshort_arm_new_loopback2

MVFD_IRMB_longtoshort_arm_new_loopback:
			LDRH	r4, [r1],#4
			LDRH	r5, [r7],#4
			LDRH	r6, [r1],#4
			LDRH	r14, [r7],#4

			LDR		r4, [r0, r4, LSL #2]
			LDR		r5, [r0, r5, LSL #2]
			LDR		r6, [r0, r6, LSL #2]
			LDR		r14, [r0, r14, LSL #2]

			STRH    r4,[r8],#4
			STRH    r5,[r9],#4
			STRH    r6,[r8],#4
			STRH    r14,[r9],#4
			SUBS	r2,r2,#1
			BNE MVFD_IRMB_longtoshort_arm_new_loopback

			CMP r3, #0
			BEQ MVFD_IRMB_longtoshort_arm_new_end
				
MVFD_IRMB_longtoshort_arm_new_loopback2:
			LDRH	r4, [r1]
			SUBS	r3,r3,#1
			LDR	r4, [r0, r4, LSL #2]
			STRH    r4, [r1],#2
			BNE MVFD_IRMB_longtoshort_arm_new_loopback2
MVFD_IRMB_longtoshort_arm_new_end:
			POP 	{r4-r9,r15}
			
	        .end
			