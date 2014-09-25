
#define DCTSIZE 8
//#define MAX_IMAGE_WIDTH (1 << 11) // 2048
//#define MAX_IMAGE_WIDTH (1 << 9) // 512
#define MAX_IMAGE_WIDTH 800

#define PIX_BUF_SIZE_CBCR (MAX_IMAGE_WIDTH * 2)
#define COEF_BUF_SIZE_Y (MAX_IMAGE_WIDTH * DCTSIZE)
#define COEF_BUF_SIZE_CBCR (MAX_IMAGE_WIDTH * DCTSIZE / 2)
// DCTSIZE * 2 rows : Y
// DCTSIZE * 0.5 rows : Cr
// DCTSIZE * 0.5 rows : Cb

typedef long INT32;
typedef unsigned char UCHAR;
typedef UCHAR UINT8;
typedef unsigned int UINT;
typedef unsigned short USHORT;

#define MAXJSAMPLE 255
#define CENTERJSAMPLE 128


/////////////////////// color conversion : RGB -> YCbCr /////////////////////////

#define SCALEBITS	16	/* speediest right-shift on some machines */
#define CBCR_OFFSET	((INT32) CENTERJSAMPLE << SCALEBITS)
#define ONE_HALF	((INT32) 1 << (SCALEBITS-1))
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))

/* We allocate one big table and divide it up into eight parts, instead of
* doing eight alloc_small requests.  This lets us use a single table base
* address, which can be held in a register in the inner loops on many
* machines (more than can hold all eight addresses, anyway).
*/

#define R_Y_OFF		0			/* offset to R => Y section */
#define G_Y_OFF		(1*(MAXJSAMPLE+1))	/* offset to G => Y section */
#define B_Y_OFF		(2*(MAXJSAMPLE+1))	/* etc. */
#define R_CB_OFF	(3*(MAXJSAMPLE+1))
#define G_CB_OFF	(4*(MAXJSAMPLE+1))
#define B_CB_OFF	(5*(MAXJSAMPLE+1))
#define R_CR_OFF	B_CB_OFF		/* B=>Cb, R=>Cr are the same */
#define G_CR_OFF	(6*(MAXJSAMPLE+1))
#define B_CR_OFF	(7*(MAXJSAMPLE+1))
#define TABLE_SIZE	(8*(MAXJSAMPLE+1))
#define TABLE_SIZE_3	(3*(MAXJSAMPLE+1))
#define TABLE_SIZE_1	((MAXJSAMPLE+1))

#define RY		R_Y_OFF
#define GY		G_Y_OFF
#define BY		B_Y_OFF
#define RCB		R_CB_OFF
#define GCB		G_CB_OFF
#define BCB		B_CB_OFF
#define RCR		B_CB_OFF		
#define GCR		G_CR_OFF
#define BCR		B_CR_OFF

typedef struct {
  unsigned int ehufco[256];	/* code for each symbol */
  char ehufsi[256];		/* length of code for each symbol */
  /* If no code has been allocated for a symbol S, ehufsi[S] contains 0 */
} c_derived_tbl;

#define MAX_COEF_BITS 10

/////////////////////// forward DCT /////////////////////////


#define CONST_BITS  13
#define PASS1_BITS  2
#define FIX_0_298631336  ((INT32)  2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ((INT32)  3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ((INT32)  4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ((INT32)  6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ((INT32)  7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ((INT32)  9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  ((INT32)  12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  ((INT32)  15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  ((INT32)  16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  ((INT32)  16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  ((INT32)  20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  ((INT32)  25172)	/* FIX(3.072711026) */

#define MULTIPLY16C16(var,const)  ((var) * (const))
#define MULTIPLY(var,const)  MULTIPLY16C16(var,const)

#define ONE	((INT32) 1)
#define CONST_SCALE (ONE << CONST_BITS)

#define SHIFT_TEMPS
#define RIGHT_SHIFT(x,shft)	((x) >> (shft))

#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)

#define DIVIDE_BY(a,b)	if (a >= b) a /= b; else a = 0
typedef short JCOEF;

typedef struct {
	INT32 put_buffer;		/* current bit-accumulation buffer */
	int put_bits;			/* # of bits now in it */
} buf_state;//savable_state;

#if 1	//	2/12/07
typedef struct {
	//unsigned int buffer[64];		/* bit-accumulation buffer */
	unsigned char buffer[64];		/* bit-accumulation buffer */
	int nbytes;						/* # of bytes now in it */
} bit_buf;
#else
typedef struct {
	unsigned int buffer[64];		/* bit-accumulation buffer */
	int nbits;						/* # of bits now in it */
} bit_buf;
#endif

void emit_byte(int byte_data);
//void emit_all_bytes();
void write_JPEG_header();
void write_JPEG_trailer();

const unsigned int std_luminance_quant_tbl[DCTSIZE * DCTSIZE] = {
	16,  11,  10,  16,  24,  40,  51,  61,
		12,  12,  14,  19,  26,  58,  60,  55,
		14,  13,  16,  24,  40,  57,  69,  56,
		14,  17,  22,  29,  51,  87,  80,  62,
		18,  22,  37,  56,  68, 109, 103,  77,
		24,  35,  55,  64,  81, 104, 113,  92,
		49,  64,  78,  87, 103, 121, 120, 101,
		72,  92,  95,  98, 112, 100, 103,  99
};
const unsigned int std_chrominance_quant_tbl[DCTSIZE * DCTSIZE] = {
	17,  18,  24,  47,  99,  99,  99,  99,
		18,  21,  26,  66,  99,  99,  99,  99,
		24,  26,  56,  99,  99,  99,  99,  99,
		47,  66,  99,  99,  99,  99,  99,  99,
		99,  99,  99,  99,  99,  99,  99,  99,
		99,  99,  99,  99,  99,  99,  99,  99,
		99,  99,  99,  99,  99,  99,  99,  99,
		99,  99,  99,  99,  99,  99,  99,  99
};
	

const int jpeg_natural_order[DCTSIZE * DCTSIZE +16] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
 63, 63, 63, 63, 63, 63, 63, 63, /* extra entries for safety in decoder */
 63, 63, 63, 63, 63, 63, 63, 63
};

const UINT8 bits_dc_luminance[17] =
{ /* 0-base */ 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
const UINT8 val_dc_luminance[] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

const UINT8 bits_dc_chrominance[17] =
{ /* 0-base */ 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
const UINT8 val_dc_chrominance[] =
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

const UINT8 bits_ac_luminance[17] =
{ /* 0-base */ 0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
const UINT8 val_ac_luminance[] =
{ 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
0xf9, 0xfa };

const UINT8 bits_ac_chrominance[17] =
{ /* 0-base */ 0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
const UINT8 val_ac_chrominance[] =
{ 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
0xf9, 0xfa };

