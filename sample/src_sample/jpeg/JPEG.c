#include "stdio.h"
//#include "string.h"
#include "JPEG.h"
#include "008_2.h"
#include "008_2jpg.h"

//#include "../TCT.h"

#define THREAD(m) 
#define TCT_SIM_TMR_SET(m)

#define macro5  //   getint
//#define macro6  //   write_JPEG_header
//#define macro7  //   ReadOneLine
//#define macro8  //   downsampleCbCr
//#define macro9  //   BLK8x8
//#define macro10 //     CopyBlock
//#define macro11 //     DCTcore
//#define macro12 //     Quant
//#define macro13 //     UpdateDC
//#define macro14 //     EncHuff
//#define macro15 //     WriteBits
//#define macro16 //   flush_bits
//#define macro17 // write_JPEG_trailer

int pixBytes = 0, imageSizeX = 0, imageSizeY = 0;
int JPEGout[1000000] = {};
int JPEGflag = 0;
//FILE *fpjpegout;

UCHAR pixCb[PIX_BUF_SIZE_CBCR]; //PIX_BUF_SIZE_CBCR
UCHAR pixCr[PIX_BUF_SIZE_CBCR]; ///-> (MAX_IMAGE_WIDTH * 2)
UCHAR Y0[COEF_BUF_SIZE_Y];      //COEF_BUF_SIZE_Y
UCHAR Y1[COEF_BUF_SIZE_Y];      //-> (MAX_IMAGE_WIDTH * DCTSIZE)
UCHAR Cb[COEF_BUF_SIZE_CBCR];   //COEF_BUF_SIZE_CBCR
UCHAR Cr[COEF_BUF_SIZE_CBCR];   //->(MAX_IMAGE_WIDTH * DCTSIZE / 2)

//INT32 rgb_ycc_tab[TABLE_SIZE];
#if 1
INT32 r2y[TABLE_SIZE_1];        //TABLE_SIZE_1
INT32 r2b[TABLE_SIZE_1];       
INT32 r2r[TABLE_SIZE_1];
INT32 g2y[TABLE_SIZE_1];
INT32 g2b[TABLE_SIZE_1];
INT32 g2r[TABLE_SIZE_1];
INT32 b2y[TABLE_SIZE_1];
INT32 b2b[TABLE_SIZE_1];
INT32 b2r[TABLE_SIZE_1];
#else
INT32 r2y[TABLE_SIZE];
#endif

USHORT quantTableY[DCTSIZE * DCTSIZE], quantTableCbCr[DCTSIZE * DCTSIZE];

typedef enum {                  /* JPEG marker codes */
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,

  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,

  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,

  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,

  M_DHT   = 0xc4,

  M_DAC   = 0xcc,

  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,

  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,

  M_APP0  = 0xe0,
  M_APP1  = 0xe1,
  M_APP2  = 0xe2,
  M_APP3  = 0xe3,
  M_APP4  = 0xe4,
  M_APP5  = 0xe5,
  M_APP6  = 0xe6,
  M_APP7  = 0xe7,
  M_APP8  = 0xe8,
  M_APP9  = 0xe9,
  M_APP10 = 0xea,
  M_APP11 = 0xeb,
  M_APP12 = 0xec,
  M_APP13 = 0xed,
  M_APP14 = 0xee,
  M_APP15 = 0xef,

  M_JPG0  = 0xf0,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,

  M_TEM   = 0x01,

  M_ERROR = 0x100
} JPEG_MARKER;



int qualityScaling (int quality)
/* Convert a user-specified quality rating to a percentage scaling factor
* for an underlying quantization table, using our recommended scaling curve.
* The input 'quality' factor should be 0 (terrible) to 100 (very good).
*/
{
  //printf("set quality to %d\n", quality);
  /* Safety limit on quality factor.  Convert 0 to 1 to avoid zero divide. */
  if (quality <= 0) quality = 1;
  if (quality > 100) quality = 100;
  
  /* The basic table is used as-is (scaling 100) for a quality of 50.
   * Qualities 50..100 are converted to scaling percentage 200 - 2*Q;
   * note that at Q=100 the scaling is 0, which will cause jpeg_add_quant_table
   * to make all the table entries 1 (hence, minimum quantization loss).
   * Qualities 1..50 are converted to scaling percentage 5000/Q.
   */
  if (quality < 50)
    quality = 5000 / quality;
  else
    quality = 200 - quality*2;
  
  //printf("set quality to %d\n", quality);
  return quality;
}

void SetQuantTable (int tableIndex, const unsigned int *basic_table,
		    int scale_factor, int force_baseline)
/* Define a quantization table equal to the basic_table times
 * a scale factor (given as a percentage).
 * If force_baseline is TRUE, the computed quantization table entries
 * are limited to 1..255 for JPEG baseline compatibility.
 */
{
  int i;
  long temp;
  
  USHORT * qt = (tableIndex == 0) ? quantTableY : quantTableCbCr;

  for (i = 0; i < DCTSIZE * DCTSIZE; i++) {
    temp = ((long) basic_table[i] * scale_factor + 50L) / 100L;
    //printf("q[%d] = %d : %d : ", i, basic_table[i], temp);  //
    /* limit the values to the valid range */
    if (temp <= 0L) temp = 1L;
    if (temp > 32767L) temp = 32767L; /* max quantizer needed for 12 bits */
    if (force_baseline && temp > 255L)
      temp = 255L;		/* limit to baseline range if requested */
    qt[i] = (USHORT) temp;
    //printf("q[%d] = %d\n", i, qt[i]);  //
  }
}

c_derived_tbl dc_huff_derived_tbl[2], ac_huff_derived_tbl[2];

void jpeg_make_c_derived_tbl (const UINT8 *hbits, const UINT8 *hval, c_derived_tbl * dtbl)
{
	int p, i, l, lastp, si;
	char huffsize[257];
	unsigned int huffcode[257];
	unsigned int code;
	
	/* Note that huffsize[] and huffcode[] are filled in code-length order,
	* paralleling the order of the symbols themselves in htbl->huffval[].
	*/
			
	/* Figure C.1: make table of Huffman code length for each symbol */

	p = 0;
	for (l = 1; l <= 16; l++) {
		i = (int) hbits[l];
		//printf("i = %d, l = %d, p = %d\n", i, l, p);  //
		while (i--){
			huffsize[p++] = (char) l;
			//printf("*");  //
			//printf("i = %d, l = %d, p = %d\n", i, l, p);  //
		}
	}
	huffsize[p] = 0;
	lastp = p;

	//printf("lastp = %d, p = %d\n", lastp, p);  //
	
	/* Figure C.2: generate the codes themselves */
	/* We also validate that the counts represent a legal Huffman code tree. */
	
	code = 0;
	si = huffsize[0];
	p = 0;
	while (huffsize[p]) {
		while (((int) huffsize[p]) == si) {
			huffcode[p++] = code;
			code++;
		}
		/* code is now 1 more than the last code used for codelength si; but
		* it must still fit in si bits, since no code is allowed to be all ones.
		*/
		code <<= 1;
		si++;
	}

	//printf("p = %d. lastp = %d\n", p, lastp);  //

	/* Figure C.3: generate encoding tables */
	/* These are code and size indexed by symbol value */
	
	/* Set all codeless symbols to have code length 0;
	* this lets us detect duplicate VAL entries here, and later
	* allows emit_bits to detect any attempt to emit such symbols.
	*/
	//MEMZERO(dtbl->ehufsi, SIZEOF(dtbl->ehufsi));
	for(i = 0; i < 256; i ++){
		dtbl->ehufsi[i] = 0;
		//printf("(%d)", i);  //
	}
	
	/* This is also a convenient place to check for out-of-range
	* and duplicated VAL entries.  We allow 0..255 for AC symbols
	* but only 0..15 for DC.  (We could constrain them further
	* based on data depth and mode, but this seems enough.)
	*/
	
	for (p = 0; p < lastp; p++) {
		i = hval[p];
		dtbl->ehufco[i] = huffcode[p];
		dtbl->ehufsi[i] = huffsize[p];
		//printf("i = %3d, huffcode[%3d] = %04x, huffsize[%3d] = %d\n", i, p, huffcode[p], p, huffsize[p]);  //
	}
}

/* The legal range of a DCT coefficient is
 *  -1024 .. +1023  for 8-bit data;
 * -16384 .. +16383 for 12-bit data.
 * Hence the magnitude should always fit in 10 or 14 bits respectively.
 */



int imageQuality = 75;

void initializeWorkspace()
{

  int i;
  /////////// initialize buffers /////////////
  int scaleFactor = qualityScaling(imageQuality);


  SetQuantTable(0, std_luminance_quant_tbl,scaleFactor, 0);
  SetQuantTable(1, std_chrominance_quant_tbl,scaleFactor, 0);

  /////////// generate rgb_ycc_tab[] //////////////
  //int ii = 0;
  for (i = 0; i <= MAXJSAMPLE; i++) {
#if 1
    int temp;
    //printf("i = %d\n", i);  //
    r2y[i] = FIX(0.29900) * i;
    g2y[i] = FIX(0.58700) * i;
    b2y[i] = FIX(0.11400) * i     + ONE_HALF;
    r2b[i] = (-FIX(0.16874)) * i;
    g2b[i] = (-FIX(0.33126)) * i;
    /* We use a rounding fudge-factor of 0.5-epsilon for Cb and Cr.
     * This ensures that the maximum output will round to MAXJSAMPLE
     * not MAXJSAMPLE+1, and thus that we don't have to range-limit.
     */
    temp = FIX(0.50000) * i    + CBCR_OFFSET + ONE_HALF-1;
    b2b[i] = temp;//FIX(0.50000) * i    + CBCR_OFFSET + ONE_HALF-1;
    /*  B=>Cb and R=>Cr tables are the same
	rgb_ycc_tab[i+R_CR_OFF] = FIX(0.50000) * i    + CBCR_OFFSET + ONE_HALF-1;
    */
    r2r[i] = temp;
    g2r[i] = (-FIX(0.41869)) * i;
    b2r[i] = (-FIX(0.08131)) * i;
#else
    r2y[i+R_Y_OFF] = FIX(0.29900) * i;
    r2y[i+G_Y_OFF] = FIX(0.58700) * i;
    r2y[i+B_Y_OFF] = FIX(0.11400) * i     + ONE_HALF;
    r2y[i+R_CB_OFF] = (-FIX(0.16874)) * i;
    r2y[i+G_CB_OFF] = (-FIX(0.33126)) * i;
    /* We use a rounding fudge-factor of 0.5-epsilon for Cb and Cr.
     * This ensures that the maximum output will round to MAXJSAMPLE
     * not MAXJSAMPLE+1, and thus that we don't have to range-limit.
     */
    r2y[i+B_CB_OFF] = FIX(0.50000) * i    + CBCR_OFFSET + ONE_HALF-1;
    /*  B=>Cb and R=>Cr tables are the same
	rgb_ycc_tab[i+R_CR_OFF] = FIX(0.50000) * i    + CBCR_OFFSET + ONE_HALF-1;
    */
    r2y[i+G_CR_OFF] = (-FIX(0.41869)) * i;
    r2y[i+B_CR_OFF] = (-FIX(0.08131)) * i;
#endif
  }
  /////////// generate quantTables //////////////
  
  /////////// generate huffTables //////////////
  jpeg_make_c_derived_tbl (bits_dc_luminance, val_dc_luminance, &dc_huff_derived_tbl[0]);
  jpeg_make_c_derived_tbl (bits_ac_luminance, val_ac_luminance, &ac_huff_derived_tbl[0]);
  jpeg_make_c_derived_tbl (bits_dc_chrominance, val_dc_chrominance, &dc_huff_derived_tbl[1]);
  jpeg_make_c_derived_tbl (bits_ac_chrominance, val_ac_chrominance, &ac_huff_derived_tbl[1]);

}

unsigned int getint(int info[4])
{
  int c, c1, c2, c3;
  
  c = info[0];  c1 = info[1];  c2 = info[2];  c3 = info[3];
  return ((UCHAR) c) +
    (((UCHAR) c1) << 8) + 
    (((UCHAR) c2) << 16) +
    (((UCHAR) c3) << 24);
}

void downsampleCbCr(int i)//UCHAR * pCb, UCHAR * pCr)
{
	int j;
	UCHAR * ppCb = pixCb;
	UCHAR * ppCr = pixCr;
	static UCHAR * pCb = Cb, * pCr = Cr;
	UCHAR * pCb0 = pCb, * pCr0 = pCr;
	int colPadding = (imageSizeX & 0xf) ? 16 - (imageSizeX & 0xf) : 0;
	int jj = imageSizeX + colPadding;
	int bias = 1;			/* bias = 1,2,1,2,... for successive samples */

	for(j = 0; j < jj; j += 2, ppCb += 2, ppCr += 2){
		*(pCb0 ++) = (UCHAR) (((UINT) *(ppCb) + (UINT) *(ppCb + 1) +
			(UINT) *(ppCb + MAX_IMAGE_WIDTH) + (UINT) *(ppCb + (MAX_IMAGE_WIDTH + 1)) + bias) >> 2);
		*(pCr0 ++) = (UCHAR) (((UINT) *(ppCr) + (UINT) *(ppCr + 1) +
			(UINT) *(ppCr + MAX_IMAGE_WIDTH) + (UINT) *(ppCr + (MAX_IMAGE_WIDTH + 1)) + bias) >> 2);
		bias ^= 3;		/* 1=>2, 2=>1 */
	}
	if((i & (DCTSIZE * 2 - 1)) == 0){
		pCb = Cb;
		pCr = Cr;
	}
	else{
		pCb += MAX_IMAGE_WIDTH / 2;
		pCr += MAX_IMAGE_WIDTH / 2;
	}
}

void DCTcore(int * coefIn, int * coefOut)
{

  SHIFT_TEMPS
    
    /* Pass 1: process rows. */
    /* Note results are scaled up by sqrt(8) compared to a true DCT; */
    /* furthermore, we scale the results by 2**PASS1_BITS. */
    
    //THREAD(DCT)
    {
      int tempCoef[DCTSIZE * DCTSIZE];
      int * dataptr, * dataptr2, * dataptr3;
      {//THREAD(Dr){
	//dataptr = coefIn;
	//dataptr2 = tempCoef;
	INT32 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	INT32 tmp10, tmp11, tmp12, tmp13;
	INT32 z1, z2, z3, z4, z5;
	int ctr;
	dataptr = coefIn;
	dataptr2 = tempCoef;
	for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
	  tmp0 = dataptr[0] + dataptr[7];
	  tmp7 = dataptr[0] - dataptr[7];
	  tmp1 = dataptr[1] + dataptr[6];
	  tmp6 = dataptr[1] - dataptr[6];
	  tmp2 = dataptr[2] + dataptr[5];
	  tmp5 = dataptr[2] - dataptr[5];
	  tmp3 = dataptr[3] + dataptr[4];
	  tmp4 = dataptr[3] - dataptr[4];
	  
	  /* Even part per LL&M figure 1 --- note that published figure is faulty;
	   * rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
	   */
	  
	  tmp10 = tmp0 + tmp3;
	  tmp13 = tmp0 - tmp3;
	  tmp11 = tmp1 + tmp2;
	  tmp12 = tmp1 - tmp2;
	  
	  dataptr2[0] = (int) ((tmp10 + tmp11) << PASS1_BITS);
	  dataptr2[4] = (int) ((tmp10 - tmp11) << PASS1_BITS);
	  
	  z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
	  dataptr2[2] = (int) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),
				      CONST_BITS-PASS1_BITS);
	  dataptr2[6] = (int) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065),
				      CONST_BITS-PASS1_BITS);
	  
	  /* Odd part per figure 8 --- note paper omits factor of sqrt(2).
	   * cK represents cos(K*pi/16).
	   * i0..i3 in the paper are tmp4..tmp7 here.
	   */
	  
	  z1 = tmp4 + tmp7;
	  z2 = tmp5 + tmp6;
	  z3 = tmp4 + tmp6;
	  z4 = tmp5 + tmp7;
	  z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */
	  
	  tmp4 = MULTIPLY(tmp4, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
	  tmp5 = MULTIPLY(tmp5, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
	  tmp6 = MULTIPLY(tmp6, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
	  tmp7 = MULTIPLY(tmp7, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
	  z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
	  z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
	  z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
	  z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */
	  
	  z3 += z5;
	  z4 += z5;
	  
	  dataptr2[7] = (int) DESCALE(tmp4 + z1 + z3, CONST_BITS-PASS1_BITS);
	  dataptr2[5] = (int) DESCALE(tmp5 + z2 + z4, CONST_BITS-PASS1_BITS);
	  dataptr2[3] = (int) DESCALE(tmp6 + z2 + z3, CONST_BITS-PASS1_BITS);
	  dataptr2[1] = (int) DESCALE(tmp7 + z1 + z4, CONST_BITS-PASS1_BITS);
	  
	  dataptr += DCTSIZE;		/* advance pointer to next row */
	  dataptr2 += DCTSIZE;		/* advance pointer to next row */
	}
      }
      /* Pass 2: process columns.
       * We remove the PASS1_BITS scaling, but leave the results scaled up
       * by an overall factor of 8.
       */
      
      {//THREAD(Dc){
	//dataptr3 = coefOut;
	//dataptr2 = tempCoef;
	INT32 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	INT32 tmp10, tmp11, tmp12, tmp13;
	INT32 z1, z2, z3, z4, z5;
	int ctr;
	dataptr3 = coefOut;
	dataptr2 = tempCoef;
	for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
	  tmp0 = dataptr2[DCTSIZE*0] + dataptr2[DCTSIZE*7];
	  tmp7 = dataptr2[DCTSIZE*0] - dataptr2[DCTSIZE*7];
	  tmp1 = dataptr2[DCTSIZE*1] + dataptr2[DCTSIZE*6];
	  tmp6 = dataptr2[DCTSIZE*1] - dataptr2[DCTSIZE*6];
	  tmp2 = dataptr2[DCTSIZE*2] + dataptr2[DCTSIZE*5];
	  tmp5 = dataptr2[DCTSIZE*2] - dataptr2[DCTSIZE*5];
	  tmp3 = dataptr2[DCTSIZE*3] + dataptr2[DCTSIZE*4];
	  tmp4 = dataptr2[DCTSIZE*3] - dataptr2[DCTSIZE*4];
	  
	  /* Even part per LL&M figure 1 --- note that published figure is faulty;
	   * rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
	   */
	  
	  tmp10 = tmp0 + tmp3;
	  tmp13 = tmp0 - tmp3;
	  tmp11 = tmp1 + tmp2;
	  tmp12 = tmp1 - tmp2;
	  
	  dataptr3[DCTSIZE*0] = (int) DESCALE(tmp10 + tmp11, PASS1_BITS);
	  dataptr3[DCTSIZE*4] = (int) DESCALE(tmp10 - tmp11, PASS1_BITS);
	  
	  z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
	  dataptr3[DCTSIZE*2] = (int) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),
					      CONST_BITS+PASS1_BITS);
	  dataptr3[DCTSIZE*6] = (int) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065),
					      CONST_BITS+PASS1_BITS);
	  
	  /* Odd part per figure 8 --- note paper omits factor of sqrt(2).
	   * cK represents cos(K*pi/16).
	   * i0..i3 in the paper are tmp4..tmp7 here.
	   */
	  
	  z1 = tmp4 + tmp7;
	  z2 = tmp5 + tmp6;
	  z3 = tmp4 + tmp6;
	  z4 = tmp5 + tmp7;
	  z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */
	  
	  tmp4 = MULTIPLY(tmp4, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
	  tmp5 = MULTIPLY(tmp5, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
	  tmp6 = MULTIPLY(tmp6, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
	  tmp7 = MULTIPLY(tmp7, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
	  z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
	  z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
	  z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
	  z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */
	  
	  z3 += z5;
	  z4 += z5;
	  
	  dataptr3[DCTSIZE*7] = (int) DESCALE(tmp4 + z1 + z3,
					      CONST_BITS+PASS1_BITS);
	  dataptr3[DCTSIZE*5] = (int) DESCALE(tmp5 + z2 + z4,
					      CONST_BITS+PASS1_BITS);
	  dataptr3[DCTSIZE*3] = (int) DESCALE(tmp6 + z2 + z3,
					      CONST_BITS+PASS1_BITS);
	  dataptr3[DCTSIZE*1] = (int) DESCALE(tmp7 + z1 + z4,
					      CONST_BITS+PASS1_BITS);
	  
	  dataptr3++;			/* advance pointer to next column */
	  dataptr2++;			/* advance pointer to next column */
	}
      }
    }
}


void Quant(int * coef, JCOEF * qcoef, int qtableIndex)
{
  int i;
  int qval, temp;
  USHORT * divisors = (qtableIndex == 0) ? quantTableY : quantTableCbCr;

  //printf("divisors[5] = [%d]", divisors[5]);  //
  for (i = 0; i < DCTSIZE * DCTSIZE; i++) {
#if 1	// 9/21/06
    qval = divisors[i] << 2;
    temp = coef[i];
    if(temp < 0)
      qcoef[i] = (JCOEF)(-((-temp + qval) / (qval << 1)));
    else
      qcoef[i] = (JCOEF)((temp + qval) / (qval << 1));
#else
    qval = divisors[i] << 3;
    temp = coef[i];
    /* Divide the coefficient value by qval, ensuring proper rounding.
     * Since C does not specify the direction of rounding for negative
     * quotients, we have to force the dividend positive for portability.
     *
     * In most files, at least half of the output values will be zero
     * (at default quantization settings, more like three-quarters...)
     * so we should ensure that this case is fast.  On many machines,
     * a comparison is enough cheaper than a divide to make a special test
     * a win.  Since both inputs will be nonnegative, we need only test
     * for a < b to discover whether a/b is 0.
     * If your machine's division is fast enough, define FAST_DIVIDE.
     */
    if (temp < 0) {
      temp = -temp;
      temp += qval>>1;	/* for rounding */
      DIVIDE_BY(temp, qval);
      temp = -temp;
    } else {
      temp += qval>>1;	/* for rounding */
      DIVIDE_BY(temp, qval);
    }
    qcoef[i] = (JCOEF) temp;
#endif
  }
}
//#endif

void emit_bits (buf_state * state, unsigned int code, int size)
{
  /* This routine is heavily used, so it's worth coding tightly. */
  register INT32 put_buffer = (INT32) code;
  register int put_bits = state->put_bits;

  put_buffer &= (((INT32) 1)<<size) - 1; /* mask off any extra bits in code */
  
  put_bits += size;		/* new number of bits in buffer */
  
  put_buffer <<= 24 - put_bits; /* align incoming bits */
  
  put_buffer |= state->put_buffer; /* and merge with old buffer contents */
  
  
  while (put_bits >= 8) {
    int c = (int) ((put_buffer >> 16) & 0xFF);
    
    emit_byte(c);
    if (c == 0xFF) {		/* need to stuff a zero byte? */
      emit_byte(0);
    }
    put_buffer <<= 8;
    put_bits -= 8;
  }
  
  state->put_buffer = put_buffer; /* update state variables */
  state->put_bits = put_bits;
  
}

void flush_bits (buf_state * state)
{
	emit_bits(state, 0x7F, 7); /* fill any partial byte with ones */
	state->put_buffer = 0;	/* and reset bit-buffer to empty */
	state->put_bits = 0;
}

#define USE_LALITHA

bit_buf * ebb_buf;
int ebb_buf_nbytes;
int ebb_state_put_buffer, ebb_state_put_bits;
//int ebb_count, ebb_run_count;
void emit_bits_to_buffer (unsigned int code, int size)
{

  /* This routine is heavily used, so it's worth coding tightly. */
  {//THREAD(emit){
    register INT32 put_buffer = (INT32) code;
    register int put_bits = ebb_state_put_bits;
    
    put_bits += size;		/* new number of bits in buffer */
    
    put_buffer <<= 24 - put_bits; /* align incoming bits */
    put_buffer |= ebb_state_put_buffer; /* and merge with old buffer contents */
    
    while (put_bits >= 8) {
      
      ebb_buf->buffer[ebb_buf_nbytes] = (put_buffer >> 16);
      ebb_buf_nbytes ++;
      
      put_buffer <<= 8;
      put_bits -= 8;
    }
    
    ebb_state_put_buffer = put_buffer; /* update state variables */
    ebb_state_put_bits = put_bits;
    
  }
  
}
void EncHuff (buf_state * state, bit_buf * buf, JCOEF * block, int htbl_index)
{
  
  register int temp, temp2;
  register int nbits = 0;
  register int k, r, i;
  
  //ebb_buf_nbytes = 0;
  
  //ebb_buf = buf;
  //ebb_state_put_bits = 0;
  //ebb_state_put_buffer = 0;
  //
  /* Encode the DC coefficient difference per section F.1.2.1 */
  
  c_derived_tbl * dctbl = &dc_huff_derived_tbl[htbl_index];
  c_derived_tbl * actbl = &ac_huff_derived_tbl[htbl_index];
  
  ebb_buf_nbytes = 0;
  
  ebb_buf = buf;
  ebb_state_put_bits = 0;
  ebb_state_put_buffer = 0;
  
  
  temp = temp2 = block[0];// - last_dc_val;
  
  
  if (temp < 0) {
    temp = -temp;		/* temp is abs value of input */
    /* For a negative input, want temp2 = bitwise complement of abs(input) */
    /* This code assumes we are on a two's complement machine */
    temp2--;
  }
  temp = temp; // this helps to avoid mem-store in below loop
  
  /* Find the number of bits needed for the magnitude of the coefficient */
  nbits = 0;
  while (temp) {
    nbits++;
    temp >>= 1;
  }
  
  nbits = nbits; // this helps to avoid mem-store in above loop
  
  /* Emit the Huffman-coded symbol for the number of bits */
  emit_bits_to_buffer(dctbl->ehufco[nbits], dctbl->ehufsi[nbits]);
  
  /* Emit that number of bits of the value, if positive, */
  /* or the complement of its magnitude, if negative. */
  if (nbits){			/* emit_bits rejects calls with size 0 */
    emit_bits_to_buffer(temp2 & ((1 << nbits) - 1), nbits);
  }
  /* Encode the AC coefficients per section F.1.2.2 */
  
  r = 0;			/* r = run length of zeros */
  
  for (k = 1; k < DCTSIZE * DCTSIZE; k++) {
    if ((temp = block[jpeg_natural_order[k]]) == 0) {
      r++;
    } else {
      /* if run length > 15, must emit special run-length-16 codes (0xF0) */
      while (r > 15) {
				emit_bits_to_buffer(actbl->ehufco[0xF0], actbl->ehufsi[0xF0]);
				r -= 16;
      }
      
      temp2 = temp;
      if (temp < 0) {
	temp = -temp;		/* temp is abs value of input */
				/* This code assumes we are on a two's complement machine */
	temp2--;
      }
      temp = temp; // this helps to avoid mem-store in below loop
      
      /* Find the number of bits needed for the magnitude of the coefficient */
      nbits = 1;		/* there must be at least one 1 bit */
      while (temp >>= 1)
	nbits++;
      nbits = nbits; // this helps to avoid mem-store in above loop
      
      /* Emit Huffman symbol for run length / number of bits */
      i = (r << 4) + nbits;
      emit_bits_to_buffer(actbl->ehufco[i], actbl->ehufsi[i]);
			
      /* Emit that number of bits of the value, if positive, */
      /* or the complement of its magnitude, if negative. */
      emit_bits_to_buffer(temp2 & ((1 << nbits) - 1), nbits);
      
      r = 0;
    }
  }
  
  /* If the last coef(s) were zero, emit an end-of-block code */
  if (r > 0){
    emit_bits_to_buffer(actbl->ehufco[0], actbl->ehufsi[0]);
  }
  buf->nbytes = ebb_buf_nbytes;
  state->put_bits = ebb_state_put_bits;
  state->put_buffer = ebb_state_put_buffer;
  
}

void WriteBits(buf_state * gstate, buf_state * lstate, bit_buf * buf)
{
  int cnt;
  int bytes = buf->nbytes;

  for(cnt=0; cnt < bytes; cnt ++){
    emit_bits(gstate, buf->buffer[cnt], 8);
  }
  emit_bits(gstate, lstate->put_buffer >> (24 - lstate->put_bits), lstate->put_bits);
}

void UpdateDC(JCOEF * lastDC, JCOEF * qcoef)
{

  JCOEF tempLastDC = *lastDC;

  *lastDC = qcoef[0];
  qcoef[0] -= tempLastDC;
}

void CopyBlock(UCHAR * pcomp, int * coef, int compID, int isDummy)
{

  int i, j;
  int * pcoef = coef;
  UINT rowCount = (compID == 0) ? MAX_IMAGE_WIDTH : MAX_IMAGE_WIDTH / 2;
  //UINT rowCount = (compID == 0) ? MAX_IMAGE_WIDTH + compID : MAX_IMAGE_WIDTH / 2;

  if(isDummy == 0){
    for(i = 0; i < DCTSIZE; i ++){
      UCHAR * pp = pcomp;
      pcomp += rowCount;
      for(j = 0; j < DCTSIZE; j ++)
			*(pcoef ++) = *(pp ++) - CENTERJSAMPLE;
    }
  }
  else{
    for(j = 0; j < DCTSIZE * DCTSIZE; j ++)
      *(pcoef ++) = 0;
  }
}

int emit_byte_count = 0;
int emit_byte_check = 0;

void BLK8x8(UCHAR * pcomp, int compID, JCOEF * DC, buf_state * gstate, int isDummy)
{
  
  int i, j;
  int coef[DCTSIZE * DCTSIZE];
  int coef2[DCTSIZE * DCTSIZE];
  JCOEF qcoef[DCTSIZE * DCTSIZE];
  
#ifdef macro10
  CopyBlock(pcomp, coef, compID, isDummy);
#endif
  
#ifdef macro11
  DCTcore(coef, coef2);
#endif
  
  THREAD(Q){
    
#ifdef macro12
    Quant(coef2, qcoef, compID);
#endif
    
    if(!isDummy){
      
#ifdef macro13
      UpdateDC(DC, qcoef);
#endif
    }    

  }
  THREAD(E){
    buf_state lstate;
    bit_buf buf;
    
#ifdef macro14
    EncHuff(&lstate, &buf, qcoef, compID);
#endif
    
    THREAD(W){
      
#ifdef macro15
      WriteBits(gstate, &lstate,&buf);
#endif
      
    }
  }
}

void uchararraycopy(UCHAR a[], UCHAR b[],int i)
{
  int j; 
  for(j = 0;j < i;j++){
    a[j] = b[j];
  }
}

#define INLINE_DCT_ROW

void ReadOneLine(int i)
{

  static UCHAR * pY = Y0;
  static UCHAR * ppCb = pixCb;
  static UCHAR * ppCr =  pixCr;
  //static UCHAR * bottom_pY = pY, * bottom_ppCb = ppCb, * bottom_ppCr = ppCr;
  static UCHAR * bottom_pY = Y0, * bottom_ppCb = pixCb, * bottom_ppCr = pixCr;
  int colPadding = (imageSizeX & 0xf) ? 16 - (imageSizeX & 0xf) : 0;
  int imageSizeXPadding = imageSizeX + colPadding;
  int j, jj;
  UCHAR YBuf[MAX_IMAGE_WIDTH], CbBuf[MAX_IMAGE_WIDTH], CrBuf[MAX_IMAGE_WIDTH];
  UCHAR rgbBuf[MAX_IMAGE_WIDTH * 3];
  UCHAR yy, bb, rr;
  UCHAR r, g, b;
  THREAD(cY){
    if(i < imageSizeY){
      uchararraycopy(rgbBuf,IMGfile[i],2400);
    }
    if(i < imageSizeY){
      UCHAR * pp = rgbBuf;
      INT32 * r = r2y, * g = g2y, * b = b2y;
      int sx = imageSizeX;
      UCHAR * YB = YBuf;
      int _1 = 1;
      int _2 = 2;

      for(j = 0; j < sx; j ++){
	*YB = ((r[pp[_2]] + g[pp[_1]] + b[pp[0]]) >> SCALEBITS);
	YB ++;
	pp += 3;
      }
    }
  }

  THREAD(cCb){
    if(i < imageSizeY){
      UCHAR * pp = rgbBuf;
      INT32 * r = r2b, * g = g2b, * b = b2b;
      int sx = imageSizeX;
      UCHAR * BB = CbBuf;
      int _1 = 1;
      int _2 = 2;

      for(j = 0; j < sx; j ++){
	*BB = ((r[pp[_2]] + g[pp[_1]] + b[pp[0]]) >> SCALEBITS);
	BB ++;
	pp += 3;
      }
    }
  }
  THREAD(cCr){
    if(i < imageSizeY){
      UCHAR * pp = rgbBuf;
      INT32 * r = r2r, * g = g2r, * b = b2r;
      int sx = imageSizeX;
      UCHAR * RB = CrBuf;
      int _1 = 1;
      int _2 = 2;
      for(j = 0; j < sx; j ++){
	*RB = ((r[pp[_2]] + g[pp[_1]] + b[pp[0]]) >> SCALEBITS);
	RB ++;
	pp += 3;
      }
    }
  }
  THREAD(BUF){
    if(i < imageSizeY){
      int sx = imageSizeX;
      for(j = 0; j < sx; j ++){
	pY[j] = YBuf[j];
	ppCb[j] = CbBuf[j];
	ppCr[j] = CrBuf[j];
      }
      // copy right-pix to padding area
      for(j = imageSizeX; j < imageSizeXPadding; j ++){
	pY[j] = pY[imageSizeX - 1];
	ppCb[j] = ppCb[imageSizeX - 1];
	ppCr[j] = ppCr[imageSizeX - 1];
      }
      if(i == imageSizeY - 1){
	bottom_pY = pY;
	bottom_ppCb = ppCb;
	bottom_ppCr = ppCr;
      }
    }
    else{
      for(j = 0; j < imageSizeXPadding; j ++){
	pY[j] = bottom_pY[j];
      }
      if(i == imageSizeY){
	// since pixCb[] and pixCr[] are only 2 rows deep, only need to copy once
	for(j = 0; j < imageSizeXPadding; j ++){
	  ppCb[j] = bottom_ppCb[j];
	  ppCr[j] = bottom_ppCr[j];
	}
      }
    }
  }
  i &= (DCTSIZE * 2 - 1);
  if(i == DCTSIZE - 1)
    pY = Y1;
  else if(i == DCTSIZE * 2 - 1)
    pY = Y0;
  else
    pY += MAX_IMAGE_WIDTH;
  if((i & 1)){
    ppCb = pixCb;
    ppCr = pixCr;
  }
  else{
    ppCb += MAX_IMAGE_WIDTH;
    ppCr += MAX_IMAGE_WIDTH;
  }
}

//#include "TCTSIM.h"

void JPEGtop()
{
  int i, j;
  UINT checksum = 0, checksum2 = 0;
  
  int colPadding;// = (imageSizeX & 0xf) ? 16 - (imageSizeX & 0xf) : 0;
  int imageSizeXPadding;// = imageSizeX + colPadding;
  int rowPadding;// = (imageSizeY & 0xf) ? 16 - (imageSizeY & 0xf) : 0;
  int imageSizeYPadding;// = imageSizeY + rowPadding;
  
  int count = 0;
  JCOEF DCy = 0, DCcb = 0, DCcr = 0;
  buf_state state;

  pixBytes = getint(info[0]);
  imageSizeX = getint(info[1]);
  imageSizeY = getint(info[2]);

  //printf("image size = %d x %d, pixBytes = %d\n", imageSizeX, imageSizeY, pixBytes);
  
  if(pixBytes != 3){
    //printf("Sorry. Only RGB format is currently implemented\n");
    return;
  }

  write_JPEG_header();
  //printf("\nOK\n");

  //int i, j;
  
  if(imageSizeX > MAX_IMAGE_WIDTH){
    //printf("Not enough memory (image width = : should be smaller than)\n",
    //	   imageSizeX, MAX_IMAGE_WIDTH);
    return;
  }
  //UINT checksum = 0, checksum2 = 0;
  //////////////////////////////////////////////////
  //
  // 1. RGB is converted to YCbCr pixel-by-pixel
  //   - Y is directly stored at coefY[MAX_IMAGE_WIDTH * 16]
  //   - Cb is temporarily stored at pixCb[MAX_IMAGE_WIDTH * 2]
  //   - Cr is temporarily stored at pixCr[MAX_IMAGE_WIDTH * 2]
  // 2. Cb/Cr are downsampled by 2x2
  //   - Cb is downsampled and stored at coefCb[MAX_IMAGE_WIDTH / 2 * 8];
  //   - Cr is downsampled and stored at coefCr[MAX_IMAGE_WIDTH / 2 * 8];
  //
  // pad image so that image size becomes multiple of 16
  colPadding = (imageSizeX & 0xf) ? 16 - (imageSizeX & 0xf) : 0;
  imageSizeXPadding = imageSizeX + colPadding;
  rowPadding = (imageSizeY & 0xf) ? 16 - (imageSizeY & 0xf) : 0;
  imageSizeYPadding = imageSizeY + rowPadding;
  //int count = 0;
  //UCHAR * bottom_pY, * bottom_ppCb, * bottom_ppCr;
  //JCOEF DCy = 0, DCcb = 0, DCcr = 0;
  //savable_state state;
  //buf_state state;
  state.put_bits = 0;
  state.put_buffer = 0;
  
  //ebb_count = 0;
  //ebb_run_count = 0;
  
  //printf("out_checksum = 0x%x (%d bytes), lastDC = (%d, %d, %d), count = %d\n",
  //	 emit_byte_check, emit_byte_count, DCy, DCcb, DCcr, count);

  //printf("colPadding = %d,imageSizeXPadding = %d\n",colPadding,imageSizeXPadding);//
  //printf("rowPadding = %d,imageSizeYPadding = %d\n",rowPadding,imageSizeYPadding);//
  
#define ENABLE_TMR 1

  for(i = 0; i < imageSizeYPadding;){// i ++){
    int ii;
#if ENABLE_TMR
    TCT_SIM_TMR_SET(2);
#endif
    for(ii = 0; ii < DCTSIZE; ii ++){
      ReadOneLine(i); i ++;
      ReadOneLine(i); i ++;

      THREAD(Dsamp){
	//	printf("Dsamp.");
	//if((i & 0x6) == 0)
#if ENABLE_TMR
	if((i & 0x2) == 0)
	  TCT_SIM_TMR_SET(3);
#endif
	
	downsampleCbCr(i);
	
      }
    }
    THREAD(BLKcore){
      int nR = (i - DCTSIZE >= imageSizeY);
      count = 0;
      //printf("i = %d : ",i);
      for(j = 0; j < imageSizeX; j += DCTSIZE * 2){
	int nC = (j + DCTSIZE >= imageSizeX);

	THREAD(Y0){
	  //if((j & 0x70) == 0)
#if ENABLE_TMR
	  if((j & 0x30) == 0)
	    TCT_SIM_TMR_SET(1);
#endif
	  count ++;
	  BLK8x8(&Y0[j], 0, &DCy, &state, 0);
	  BLK8x8(&Y0[j + DCTSIZE], 0, &DCy, &state, nC);
	}
	THREAD(Y1){
	  count ++;
	  BLK8x8(&Y1[j], 0, &DCy, &state, nR);
	  BLK8x8(&Y1[j + DCTSIZE], 0, &DCy, &state, nC + nR);
	}
	THREAD(C){
	count ++;
	BLK8x8(&Cb[j >> 1], 1, &DCcb, &state, 0);
	BLK8x8(&Cr[j >> 1], 1, &DCcr, &state, 0);
	}
      }
      //printf("out_checksum = 0x%x (%d bytes), lastDC = (%d, %d, %d), count = %d\n",//, ebb_count = %d, run_count = %d\n",
      //     emit_byte_check, emit_byte_count, DCy, DCcb, DCcr, count);//, ebb_count, ebb_run_count);
    }
  }
  ////fclose(fp);

  flush_bits(&state);
}

const char imageDir[] = "../../images";

//FILE * fpJPG = 0;
void emit_byte(int byte_data)
{
    emit_byte_count ++;
    emit_byte_check += byte_data;

  JPEGout[JPEGflag] = byte_data;

  /*
  fprintf(fpjpegout,"%02x",JPEGout[JPEGflag]);
  if(JPEGflag % 16 == 15){
    fprintf(fpjpegout,"\n");
  }
  */

  JPEGflag++;
}

//void check()
//{
//	char fname[1000];
//	sprintf(fname, "%s/mytest.jpg", imageDir);
//	FILE * fp0 = fopen(fname, "rb");
//	sprintf(fname, "%s/testimg.jpg", imageDir);
//	FILE * fp1 = fopen(fname, "rb");
//
//	int i;
//	i = 0;
//	int ii = 0;
//	int c0, c1;
//	while(1){
//		c0 = getc(fp0);
//		c1 = getc(fp1);
//		if(c0 == EOF || c1 == EOF)
//			break;
//		if(c0 != c1)
//			ii = i;
//		i ++;
//	}
//	if(ii == 0 && c0 == EOF && c1 == EOF)
//		printf("Identical!!\n");
//	else
//		printf("Not identical\n");
//	fclose(fp0);
//	fclose(fp1);
//}


extern int emit_byte_count;
extern int emit_byte_check;

void emit_2bytes (int value)
/* Emit a 2-byte integer; these are always MSB first in JPEG files */
{
  emit_byte((value >> 8) & 0xFF);
  emit_byte(value & 0xFF);
}

void emit_marker (JPEG_MARKER mark)
/* Emit a marker code */
{
  emit_byte(0xFF);
  emit_byte((int) mark);
}

void emit_jfif_app0 ()
/* Emit a JFIF-compliant APP0 marker */
{
  /*
   * Length of APP0 block  (2 bytes)
   * Block ID                      (4 bytes - ASCII "JFIF")
   * Zero byte                     (1 byte to terminate the ID string)
   * Version Major, Minor  (2 bytes - major first)
   * Units                 (1 byte - 0x00 = none, 0x01 = inch, 0x02 = cm)
   * Xdpu                  (2 bytes - dots per unit horizontal)
   * Ydpu                  (2 bytes - dots per unit vertical)
   * Thumbnail X size              (1 byte)
   * Thumbnail Y size              (1 byte)
   */

  emit_marker(M_APP0);

  emit_2bytes(2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1); /* length */

  emit_byte(0x4A);        /* Identifier: ASCII "JFIF" */
  emit_byte(0x46);
  emit_byte(0x49);
  emit_byte(0x46);
  emit_byte(0);
  emit_byte(1);//cinfo->JFIF_major_version); /* Version fields */
  emit_byte(1);//cinfo->JFIF_minor_version);
  emit_byte(0);//cinfo->density_unit); /* Pixel size information */
  emit_2bytes(1);//cinfo, (int) cinfo->X_density);
  emit_2bytes(1);//cinfo, (int) cinfo->Y_density);
  emit_byte(0);           /* No thumbnail image */
  emit_byte(0);
}


void write_file_header ()
{
  emit_marker(M_SOI);     /* first the SOI */

  emit_jfif_app0();
}


extern USHORT quantTableY[DCTSIZE * DCTSIZE], quantTableCbCr[DCTSIZE * DCTSIZE];
extern const int jpeg_natural_order[DCTSIZE * DCTSIZE +16];


int emit_dqt (int index)
/* Emit a DQT marker */
/* Returns the precision used (0 = 8bits, 1 = 16bits) for baseline checking */
{
  //JQUANT_TBL * qtbl = cinfo->quant_tbl_ptrs[index];
  USHORT * qtbl = (index == 0) ? quantTableY : quantTableCbCr;

  int prec;
  int i;

  prec = 0;
  for (i = 0; i < DCTSIZE * DCTSIZE; i++) {
    if (qtbl[i] > 255)
      prec = 1;
  }

  emit_marker(M_DQT);

  emit_2bytes(prec ? DCTSIZE * DCTSIZE * 2 + 1 + 2 : DCTSIZE * DCTSIZE + 1 + 2);

  //printf("out_checksum = 0x%x (%d bytes)\n", emit_byte_check, emit_byte_count);
  emit_byte(index + (prec<<4));

  for (i = 0; i < DCTSIZE * DCTSIZE; i++) {
    /* The table entries must be emitted in zigzag order. */
    unsigned int qval = qtbl[jpeg_natural_order[i]];
    //printf("qval[%d] = %d\n", jpeg_natural_order[i], qval);
    if (prec)
      emit_byte((int) (qval >> 8));
    emit_byte((int) (qval & 0xFF));
  }

  //printf("out_checksum = 0x%x (%d bytes)\n", emit_byte_check, emit_byte_count);
  return prec;
}


//extern int imageSizeX, imageSizeY;


void emit_sof (JPEG_MARKER code)
/* Emit a SOF marker */
{
  emit_marker(code);
  
  //emit_2bytes(cinfo, 3 * cinfo->num_components + 2 + 5 + 1); /* length */
  emit_2bytes(3 * 3 + 2 + 5 + 1); /* length */
  
  
  /* Make sure image isn't bigger than SOF field can handle */
  
  //emit_byte(cinfo, cinfo->data_precision);
  emit_byte(8);
  //emit_2bytes(cinfo, (int) cinfo->image_height);
  //emit_2bytes(cinfo, (int) cinfo->image_width);
  //  printf("imageSizeY = %d, imageSizeX = %d\n", imageSizeY, imageSizeX);
  emit_2bytes((int) imageSizeY);
  emit_2bytes((int) imageSizeX);
  
  //emit_byte(cinfo, cinfo->num_components);
  emit_byte(3);
  
  emit_byte(1); //compptr->component_id);
  emit_byte((2 << 4) + 2); //(compptr->h_samp_factor << 4) + compptr->v_samp_factor);
  emit_byte(0); //compptr->quant_tbl_no);
  
  emit_byte(2); //compptr->component_id);
  emit_byte((1 << 4) + 1); //(compptr->h_samp_factor << 4) + compptr->v_samp_factor);
  emit_byte(1); //compptr->quant_tbl_no);
  
  emit_byte(3); //compptr->component_id);
  emit_byte((1 << 4) + 1); //(compptr->h_samp_factor << 4) + compptr->v_samp_factor);
  emit_byte(1); //compptr->quant_tbl_no);
#if 0
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    emit_byte(cinfo, compptr->component_id);
    emit_byte(cinfo, (compptr->h_samp_factor << 4) + compptr->v_samp_factor);
    emit_byte(cinfo, compptr->quant_tbl_no);
  }
#endif
}


void write_frame_header ()
{
  int prec;

  /* Emit DQT for each quantization table.
   * Note that emit_dqt() suppresses any duplicate tables.
   */
  prec = 0;
  prec += emit_dqt(0);
  prec += emit_dqt(1);
#if 0
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    prec += emit_dqt(cinfo, compptr->quant_tbl_no);
  }
#endif
  /* now prec is nonzero iff there are any 16-bit quant tables. */

  /* Check for a non-baseline specification.
   * Note we assume that Huffman table numbers won't be changed later.
   */
  /* Emit the proper SOF marker */
  //printf("emit_sof(M_SOF0)\n");
  emit_sof(M_SOF0);       /* SOF code for baseline implementation */
}
//void jpeg_make_c_derived_tbl (const UINT8 *hbits, const UINT8 *hval, c_derived_tbl * dtbl)


void emit_dht (const UINT8 *hbits, const UINT8 *hval, int index)
/* Emit a DHT marker */
{
  int length, i;


  emit_marker(M_DHT);

  length = 0;
  for (i = 1; i <= 16; i++)
    length += hbits[i];

  emit_2bytes(length + 2 + 1 + 16);
  emit_byte(index);

  for (i = 1; i <= 16; i++)
    emit_byte(hbits[i]);

  for (i = 0; i < length; i++)
    emit_byte(hval[i]);

}


void emit_sos ()
/* Emit a SOS marker */
{

  emit_marker(M_SOS);

  //emit_2bytes(cinfo, 2 * cinfo->comps_in_scan + 2 + 1 + 3); /* length */
  emit_2bytes(2 * 3 + 2 + 1 + 3); /* length */

  //emit_byte(cinfo, cinfo->comps_in_scan);
  emit_byte(3);

  emit_byte(1);
  emit_byte((0 << 4) + 0);
  emit_byte(2);
  emit_byte((1 << 4) + 1);
  emit_byte(3);
  emit_byte((1 << 4) + 1);

  emit_byte(0);
  emit_byte(63);
  emit_byte((0 << 4) + 0);

#if 0
  for (i = 0; i < cinfo->comps_in_scan; i++) {
    compptr = cinfo->cur_comp_info[i];
    emit_byte(cinfo, compptr->component_id);
    td = compptr->dc_tbl_no;
    ta = compptr->ac_tbl_no;
    if (cinfo->progressive_mode) {
      /* Progressive mode: only DC or only AC tables are used in one scan;
       * furthermore, Huffman coding of DC refinement uses no table at all.
       * We emit 0 for unused field(s); this is recommended by the P&M text
       * but does not seem to be specified in the standard.
       */
      if (cinfo->Ss == 0) {
	ta = 0;                 /* DC scan */
	if (cinfo->Ah != 0 && !cinfo->arith_code)
	  td = 0;         /* no DC table either */
      } else {
	td = 0;                 /* AC scan */
      }
    }
    emit_byte(cinfo, (td << 4) + ta);
  }
  emit_byte(cinfo, cinfo->Ss);
  emit_byte(cinfo, cinfo->Se);
  emit_byte(cinfo, (cinfo->Ah << 4) + cinfo->Al);
#endif

}


void write_scan_header ()
{

  emit_dht(bits_dc_luminance, val_dc_luminance, 0);
  emit_dht(bits_ac_luminance, val_ac_luminance, 0x10);
  emit_dht(bits_dc_chrominance, val_dc_chrominance, 1);
  emit_dht(bits_ac_chrominance, val_ac_chrominance, 0x11);

  /* Emit DRI if required --- note that DRI value could change for each scan.
   * We avoid wasting space with unnecessary DRIs, however.
   */
  emit_sos();
}

void write_JPEG_header()
{
  write_file_header();
  write_frame_header();
  write_scan_header();
}

void write_JPEG_trailer()
{
  emit_marker(M_EOI);
}


int main()
{
  
  int tt;
  int i,flag=0;

  {//THREAD(initWS){
    initializeWorkspace();
  }

  JPEGtop();

  write_JPEG_trailer();

  /*
  for(i = 0;i < JPEGflag;i++){
    if(JPEGout[i] != JPGfile[i] ){
      //printf("%d word is mismatched!\n");
      flag = 1;
    }
  }
  */

/*
  if(flag == 0){
    printf("SUCCESS.\n");
  }
*/
  
  
  return 0;
}
