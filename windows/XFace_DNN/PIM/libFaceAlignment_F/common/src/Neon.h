#if !defined(__NEON_H__)
#define __NEON_H__

#if defined(WIN32)

#define __transfersize(n) 
/* typedefs */
/* scalar types */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

/* D register types */
typedef struct uint8x8_t_t {
	unsigned char val[8];
} uint8x8_t;
typedef struct int8x8_t_t {
	char val[8];
} int8x8_t;

typedef struct int16x4_t_t {
	short val[4];
}int16x4_t;
typedef struct uint16x4_t_t {
	unsigned short val[4];
}uint16x4_t;

typedef struct int32x2_t_t {
	int val[2];
}int32x2_t;
typedef struct uint32x2_t_t {
	unsigned int val[2];
}uint32x2_t;


/* Q register types */
typedef struct uint8x16_t_t {
	uint8x8_t val[2];
} uint8x16_t;
typedef struct int8x16_t_t {
	int8x8_t val[2];
} int8x16_t;

typedef struct int16x8_t_t {
	int16x4_t val[2];
}int16x8_t;
typedef struct uint16x8_t_t {
	uint16x4_t val[2];
}uint16x8_t;

typedef struct int32x4_t_t {
	int32x2_t val[2];
} int32x4_t;
typedef struct uint32x4_t_t {
	uint32x2_t val[2];
} uint32x4_t;

typedef struct uint8x8x4_t_t{
	uint8x8_t val[4];
}uint8x8x4_t;

typedef struct uint8x16x4_t_t{
	uint8x16_t val[4];
}uint8x16x4_t;

/** Functions **/
static inline int32x2_t veorq_s32 (int32x2_t a, int32x2_t b) {
	int32x2_t ret; 
	ret.val[0]=a.val[0]^b.val[0];
	ret.val[1]=a.val[1]^b.val[1];
	return ret;
}
static inline int32x4_t veorq_s32 (int32x4_t a, int32x4_t b) {
	int32x4_t ret; 
	ret.val[0] = veorq_s32(a.val[0],b.val[0]);
	ret.val[1] = veorq_s32(a.val[1],b.val[1]);
	return ret;
}
static inline uint32x2_t veor_u32 (uint32x2_t a, uint32x2_t b) {
	uint32x2_t ret; 
	ret.val[0]=a.val[0]^b.val[0];
	ret.val[1]=a.val[1]^b.val[1];
	return ret;
}
static inline uint32x4_t veorq_u32 (uint32x4_t a, uint32x4_t b) {
	uint32x4_t ret; 
	ret.val[0] = veor_u32(a.val[0],b.val[0]);
	ret.val[1] = veor_u32(a.val[1],b.val[1]);
	return ret;
}

//Load Instructions
static inline uint8x8_t vld1_u8 (const uint8_t *ptr) {
	uint8x8_t ret;
	ret.val[0] = ptr[0];
	ret.val[1] = ptr[1];
	ret.val[2] = ptr[2];
	ret.val[3] = ptr[3];
	ret.val[4] = ptr[4];
	ret.val[5] = ptr[5];
	ret.val[6] = ptr[6];
	ret.val[7] = ptr[7];
	return ret;
}

static inline uint8x16_t vld1q_u8(const uint8_t * ptr) {
	uint8x16_t ret;
	ret.val[0] = vld1_u8(ptr); 
	ptr+=8;
	ret.val[1] = vld1_u8(ptr); 
	return ret;
}

static inline int32x2_t vld1_s32 (const int32_t *ptr ) {
	int32x2_t ret;
	ret.val[0] = *ptr++;
	ret.val[1] = *ptr++;
	return ret;
}

static inline int32x4_t vld1q_s32 (const int32_t *ptr) {
	int32x4_t ret;
	ret.val[0] = vld1_s32(ptr); ptr+=2;
	ret.val[1] = vld1_s32(ptr); 
	return ret;
}
static inline uint32x2_t vld1_u32 (const uint32_t *ptr ) {
	uint32x2_t ret;
	ret.val[0] = *ptr++;
	ret.val[1] = *ptr++;
	return ret;
}

static inline uint32x4_t vld1q_u32 (const uint32_t *ptr) {
	uint32x4_t ret;
	ret.val[0] = vld1_u32(ptr); ptr+=2;
	ret.val[1] = vld1_u32(ptr); 
	return ret;
}

//Store Instructions
static inline void vst1_u8(uint8_t * ptr, uint8x8_t val){
	int i;
	for(i = 0; i < 8; i++)
	{
		*ptr++ = val.val[i];
	}
}

static inline void vst4_u8(uint8_t * ptr, uint8x8x4_t val){
	int i,j;
	for ( i = 0; i < 8; i++)
	{
		for ( j = 0; j < 4; j++) // 4 for VLD4
		{
			*ptr++ = val.val[j].val[i];
		}
	}
}


static inline void vst4q_u8(uint8_t * ptr, uint8x16x4_t val){
	int i,j;
	for ( i = 0; i < 8; i++)
	{
		for ( j = 0; j < 4; j++) // 4 for VLD4
		{
			*ptr++ = val.val[j].val[0].val[i];
		}
	}
	for ( i = 0; i < 8; i++)
	{
		for ( j = 0; j < 4; j++) // 4 for VLD4
		{
			*ptr++ = val.val[j].val[1].val[i];
		}
	}
}

static inline uint16x8_t vmovl_u8 (uint8x8_t a) {
	uint16x8_t ret;
	ret.val[0].val[0] = a.val[0];
	ret.val[0].val[1] = a.val[1];
	ret.val[0].val[2] = a.val[2];
	ret.val[0].val[3] = a.val[3];
	ret.val[1].val[0] = a.val[4];
	ret.val[1].val[1] = a.val[5];
	ret.val[1].val[2] = a.val[6];
	ret.val[1].val[3] = a.val[7];
	return ret;
}

static inline int32x4_t vmovl_s16 (int16x4_t a) {
	int32x4_t ret;
	ret.val[0].val[0] = a.val[0];
	ret.val[0].val[1] = a.val[1];
	ret.val[1].val[0] = a.val[2];
	ret.val[1].val[1] = a.val[3];
	return ret;
}
static inline uint32x4_t vmovl_u16 (uint16x4_t a) {
	uint32x4_t ret;
	ret.val[0].val[0] = a.val[0];
	ret.val[0].val[1] = a.val[1];
	ret.val[1].val[0] = a.val[2];
	ret.val[1].val[1] = a.val[3];
	return ret;
}



//ReInterpret Instructions
static inline int16x4_t vreinterpret_s16_u16 (uint16x4_t a) {
	int16x4_t ret;
	ret.val[0] = a.val[0];
	ret.val[1] = a.val[1];
	ret.val[2] = a.val[2];
	ret.val[3] = a.val[3];
	return ret;
}

static inline uint8x8_t vreinterpret_u8_s32 (int32x2_t a) {
	uint8x8_t ret;
	ret.val[0] = a.val[0] & 0x0ff;
	ret.val[1] = (a.val[0]>>8) & 0x0ff;
	ret.val[2] = (a.val[0]>>16) & 0x0ff;
	ret.val[3] = (a.val[0]>>24) & 0x0ff;
	ret.val[4] = a.val[1] & 0x0ff;
	ret.val[5] = (a.val[1]>>8) & 0x0ff;
	ret.val[6] = (a.val[1]>>16) & 0x0ff;
	ret.val[7] = (a.val[1]>>24) & 0x0ff;
	return ret;
}

static inline uint8x8_t vreinterpret_u8_u32 (uint32x2_t a) {
	uint8x8_t ret;
	ret.val[0] = a.val[0] & 0x0ff;
	ret.val[1] = (a.val[0]>>8) & 0x0ff;
	ret.val[2] = (a.val[0]>>16) & 0x0ff;
	ret.val[3] = (a.val[0]>>24) & 0x0ff;
	ret.val[4] = a.val[1] & 0x0ff;
	ret.val[5] = (a.val[1]>>8) & 0x0ff;
	ret.val[6] = (a.val[1]>>16) & 0x0ff;
	ret.val[7] = (a.val[1]>>24) & 0x0ff;
	return ret;
}

static inline uint16x4_t vget_low_u16 (uint16x8_t a) {
	return a.val[0];
}

static inline uint16x4_t vget_high_u16 (uint16x8_t a) {
	return a.val[1];
}

static inline int32x2_t vmla_s32 (int32x2_t d, int32x2_t n, int32x2_t m) {
	d.val[0] = d.val[0] + (n.val[0] * m.val[0]);
	d.val[1] = d.val[1] + (n.val[1] * m.val[1]);
	return d;
}

static inline int32x4_t vmlaq_s32 (int32x4_t d, int32x4_t n, int32x4_t m) {
	d.val[0] = vmla_s32(d.val[0], n.val[0],m.val[0]);
	d.val[1] = vmla_s32(d.val[1], n.val[1],m.val[1]);
	return d;
}
static inline uint32x2_t vmla_u32 (uint32x2_t d, uint32x2_t n, uint32x2_t m) {
	d.val[0] = d.val[0] + (n.val[0] * m.val[0]);
	d.val[1] = d.val[1] + (n.val[1] * m.val[1]);
	return d;
}

static inline uint32x4_t vmlaq_u32 (uint32x4_t d, uint32x4_t n, uint32x4_t m) {
	d.val[0] = vmla_u32(d.val[0], n.val[0],m.val[0]);
	d.val[1] = vmla_u32(d.val[1], n.val[1],m.val[1]);
	return d;
}

static inline int32x2_t vadd_s32 (int32x2_t n, int32x2_t m) {
	int32x2_t ret;
	ret.val[0] = n.val[0] + m.val[0];
	ret.val[1] = n.val[1] + m.val[1];
	return ret;
}

static inline int32x4_t vaddq_s32 (int32x4_t n, int32x4_t m) {
	int32x4_t ret;
	ret.val[0] = vadd_s32(n.val[0],m.val[0]);
	ret.val[1] = vadd_s32(n.val[1],m.val[1]);
	return ret;
}
static inline uint32x2_t vadd_u32 (uint32x2_t n, uint32x2_t m) {
	uint32x2_t ret;
	ret.val[0] = n.val[0] + m.val[0];
	ret.val[1] = n.val[1] + m.val[1];
	return ret;
}

static inline uint32x4_t vaddq_u32 (uint32x4_t n, uint32x4_t m) {
	uint32x4_t ret;
	ret.val[0] = vadd_u32(n.val[0],m.val[0]);
	ret.val[1] = vadd_u32(n.val[1],m.val[1]);
	return ret;
}

static inline int32x2_t vpadd_s32 (int32x2_t n, int32x2_t m) {
	int32x2_t ret;
	ret.val[0] = n.val[0] + n.val[1];
	ret.val[1] = m.val[0] + m.val[1];
	return ret;
}
static inline uint32x2_t vpadd_u32 (uint32x2_t n, uint32x2_t m) {
	uint32x2_t ret;
	ret.val[0] = n.val[0] + n.val[1];
	ret.val[1] = m.val[0] + m.val[1];
	return ret;
}

static inline int32x2_t vsub_s32 (int32x2_t n, int32x2_t m) {
	int32x2_t ret;
	ret.val[0] = n.val[0] - m.val[0];
	ret.val[1] = n.val[1] - m.val[1];
	return ret;
}

static inline int32x4_t vsubq_s32 (int32x4_t n, int32x4_t m) {
	int32x4_t ret;
	ret.val[0] = vsub_s32(n.val[0],m.val[0]);
	ret.val[1] = vsub_s32(n.val[1],m.val[1]);
	return ret;
}

static inline int32x2_t vget_high_s32 (int32x4_t a) {
	return a.val[1];
}

static inline int32x2_t vget_low_s32 (int32x4_t a) {
	return a.val[0];
}

static inline uint32x2_t vget_high_u32 (uint32x4_t a) {
	return a.val[1];
}

static inline uint32x2_t vget_low_u32 (uint32x4_t a) {
	return a.val[0];
}


//Shift Instructions
static inline int32x2_t vshr_n_s32 (int32x2_t a, const int shift) {
	a.val[0] = a.val[0]>>shift;
	a.val[1] = a.val[1]>>shift;
	return a;
}
static inline uint32x2_t vshr_n_u32 (uint32x2_t a, const int shift) {
	a.val[0] = a.val[0]>>shift;
	a.val[1] = a.val[1]>>shift;
	return a;
}

static inline uint16x4_t vqshrun_n_s32(int32x4_t a, const int b){
	uint16x4_t c;
	c.val[0] = a.val[0].val[0] >> b;
	c.val[1] = a.val[0].val[1] >> b;
	c.val[2] = a.val[1].val[0] >> b;
	c.val[3] = a.val[1].val[1] >> b;	
	return c;
}

static inline uint8x8_t vshrn_n_u16(uint16x8_t a, const int b){
	uint8x8_t c;
	c.val[0] =	a.val[0].val[0] >> b;
	c.val[1] =	a.val[0].val[1] >> b;
	c.val[2] =	a.val[0].val[2] >> b;
	c.val[3] =	a.val[0].val[3] >> b;
	c.val[4] =	a.val[1].val[0] >> b;
	c.val[5] =	a.val[1].val[1] >> b;
	c.val[6] =	a.val[1].val[2] >> b;
	c.val[7] =	a.val[1].val[3] >> b;
	return c;
}


static inline void vst1_lane_s32 (int32_t *ptr, int32x2_t a, const int lane) {
	*ptr = a.val[lane];
}

static inline void vst1_lane_s8 (int8_t *ptr, int8x8_t a, const int lane) {
	*ptr = a.val[lane];
}

static inline void vst1_lane_u8 (uint8_t *ptr, uint8x8_t a, const int lane) {
	*ptr = a.val[lane];
}

//Combine Instructions
static inline uint16x8_t vcombine_u16(uint16x4_t low, uint16x4_t high){
	uint16x8_t res;
	res.val[0] = low;
	res.val[1] = high;
	return res;
}
static inline int32x4_t vcombine_s32(int32x2_t low, int32x2_t high){
	int32x4_t res;
	res.val[0] = low;
	res.val[1] = high;
	return res;
}

//Setting all lanes to same value
static inline int32x2_t vdup_n_s32(int32_t value){
	int32x2_t ret;
	ret.val[0] = value;
	ret.val[1] = value;
	return ret;
}
static inline int32x4_t vdupq_n_s32(int32_t value){
	int32x4_t ret;
	ret.val[0] = vdup_n_s32(value);
	ret.val[1] = vdup_n_s32(value);
	return ret;
}


//Initialize a vector from a literal bit pattern
static inline int32x2_t vcreate_s32(uint64_t a){
	int32x2_t ret;
	ret.val[0] = a & 0xFFFFFFFF;
	ret.val[1] = a >> 32;
	return ret;
}	

#else
#include <arm_neon.h>
#endif //#if defined(WIN32)
#endif //#if !defined(__NEON_H__)
