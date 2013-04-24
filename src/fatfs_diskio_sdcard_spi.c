/*
 * (c) Domen Puncer, Visionect, d.o.o.
 * BSD License
 *
 * v0.2 add support for SDHC
 */

#include "stm32f10x_lib.h"
#include "uartio.h"
#include "fatfs_diskio_sdcard_spi.h"

/*
 * Code is split into 3 parts:
 * - generic SPI code: adapt for your MCU
 * - sd card code, with crc7 and crc16 calculations
 *   there's lots of it, but it's simple
 * - fatfs interface. If you use anything else, look here for
 *   interface to SD card code
 */


enum sd_speed { SD_SPEED_INVALID, SD_SPEED_400KHZ, SD_SPEED_25MHZ };


/*** spi functions ***/

static void spi_set_speed(enum sd_speed speed);

/* SD card is connected to SPI2, PB12-15 */
#define SPI_SD SPI2
static void spi_init(void)
{
	GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &gpio);

   /* The MISO pin should be configured as input pin with pull up.
      Alternate Function is used in SPI slave mode */
   gpio.GPIO_Pin = GPIO_Pin_14;
   gpio.GPIO_Speed = GPIO_Speed_50MHz;
   gpio.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_Init(GPIOB, &gpio);

   /* Set up CS-pin and power enable*/
	gpio.GPIO_Pin = GPIO_Pin_12| GPIO_Pin_1;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &gpio);

   /* Enable SD-card power */
   GPIO_SetBits(GPIOB, GPIO_Pin_1);

	spi_set_speed(SD_SPEED_400KHZ);
}
#define spi_cs_low() do { GPIOB->BRR = GPIO_Pin_12; } while (0)
#define spi_cs_high() do { GPIOB->BSRR = GPIO_Pin_12; } while (0)

static void spi_set_speed(enum sd_speed speed)
{
	SPI_InitTypeDef spi;
	u16 prescaler = SPI_BaudRatePrescaler_128;

	if (speed == SD_SPEED_400KHZ)
		prescaler = SPI_BaudRatePrescaler_128;
	else if (speed == SD_SPEED_25MHZ)
		prescaler = SPI_BaudRatePrescaler_2;
	/* ^ with /2 APB1 this will be 15mhz/234k at 60mhz
	 * 18/281 at 72. which is ok, 100<x<400khz, and <25mhz */

	SPI_Cmd(SPI_SD, DISABLE);

	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_DataSize = SPI_DataSize_8b;
	spi.SPI_CPOL = SPI_CPOL_Low;
	spi.SPI_CPHA = SPI_CPHA_1Edge;

	spi.SPI_NSS = SPI_NSS_Soft;
	spi.SPI_BaudRatePrescaler = prescaler;
	spi.SPI_FirstBit = SPI_FirstBit_MSB;
	spi.SPI_CRCPolynomial = 7;
	SPI_Init(SPI_SD, &spi);

	SPI_Cmd(SPI_SD, ENABLE);
}

static u8 spi_txrx(u8 data)
{
	/* RXNE always happens after TXE, so if this function is used
	 * we don't need to check for TXE */
	SPI_SD->DR = (vu16) data;
	while ((SPI_SD->SR & SPI_FLAG_RXNE) == 0)
		;
	return (u8) SPI_SD->DR;
}



/* crc helpers */
static u8 crc7_one(u8 t, u8 data)
{
	int i;
	const u8 g = (u8) 0x89;

	t ^= data;
	for (i=0; i<8; i++) {
		if ((t & 0x80) != (u8)0)
			t ^= g;
		t <<= 1;
	}
	return t;
}

u8 crc7(const u8 *p, int len)
{
	int j;
	u8 crc = (u8) 0;
	for (j=0; j<len; j++)
		crc = crc7_one(crc, p[j]);

	return crc>>1;
}

/* http://www.eagleairaust.com.au/code/crc16.htm */
static u16 crc16_ccitt(u16 crc, u8 ser_data)
{
	crc  = (u16) ((u8)(crc >> 8) | (crc << 8));
	crc ^= ser_data;
	crc ^= (u16) ((u8)(crc & 0xff) >> 4);
	crc ^= (crc << 8) << 4;
	crc ^= ((crc & 0xff) << 4) << 1;

	return crc;
}
static u16 crc16(const u8 *p, int len)
{
	int i;
	u16 crc = 0;

	for (i=0; i<len; i++)
		crc = crc16_ccitt(crc, p[i]);

	return crc;
}


/*** sd functions - on top of spi code ***/

static void sd_cmd(u8 cmd, u32 arg)
{
	u8 crc = (u8) 0;
	(void) spi_txrx((u8)0x40 | cmd);
	crc = crc7_one(crc, (u8)(0x40 | cmd));
	(void) spi_txrx((u8)(arg >> 24));
	crc = crc7_one(crc, (u8)(arg >> 24));
	(void) spi_txrx((u8)(arg >> 16));
	crc = crc7_one(crc, (u8)(arg >> 16));
	(void) spi_txrx((u8)(arg >> 8));
	crc = crc7_one(crc, (u8)(arg >> 8));
	(void) spi_txrx((u8)arg);
	crc = crc7_one(crc, (u8)arg);
	/*spi_txrx(0x95);*/	/* crc7, for cmd0 */
	(void) spi_txrx(crc | 0x1);	/* crc7, for cmd0 */
}

static u8 sd_get_r1()
{
	int tries = 1000;
	u8 r;

	while (tries-- != 0) {
		r = spi_txrx((u8)0xff);
		if ((r & 0x80) == (u8)0)
			return r;
	}
	return (u8) 0xff;
}

static u16 sd_get_r2()
{
	int tries = 1000;
	u16 r = (u16) 0;

	while (tries-- != 0) {
		r = (u16) spi_txrx((u8)0xff);
		if ((r & 0x80) == (u16)0)
			break;
	}
	if (tries < 0)
		return 0xff;
	r = r<<8 | spi_txrx((u8)0xff);

	return r;
}

/*
 * r1, then 32-bit reply... same format as r3
 */
static u8 sd_get_r7(/*@out@*/u32 *r7)
   /*@modifies *r7@*/
{
	u32 r;
   u8 s;
	s = sd_get_r1();
	if (s != (u8) 0x01) {
      *r7 = 0xffffffff; /* Don't let r7 be undefined */
		return s;
   }

	r = (u32) spi_txrx((u8)0xff) << 24;
	r |= spi_txrx((u8)0xff) << 16;
	r |= spi_txrx((u8)0xff) << 8;
	r |= spi_txrx((u8)0xff);

	*r7 = r;
	return (u8) 0x01;
}
#define sd_get_r3 sd_get_r7

static const char *r1_strings[7] = {
	"in idle",
	"erase reset",
	"illegal command",
	"communication crc error",
	"erase sequence error",
	"address error",
	"parameter error"
};

static void print_r1(u8 r)
{
	unsigned int i;
	print("R1: ");printhex((unsigned int)r); print("\n");
	for (i=0; i<7; i++)
		if ((r & (1<<i)) != 0)
			print(r1_strings[i]);
}

static const char *r2_strings[15] = {
	"card is locked",
	"wp erase skip | lock/unlock cmd failed",
	"error",
	"CC error",
	"card ecc failed",
	"wp violation",
	"erase param",
	"out of range | csd overwrite",
	"in idle state",
	"erase reset",
	"illegal command",
	"com crc error",
	"erase sequence error",
	"address error",
	"parameter error",
};

static void print_r2(u16 r)
{
	unsigned int i;
	print("R2: ");printhex((unsigned int)r);print("\n");
	for (i=0; i<15; i++)
		if ((r & (1<<i)) != 0)
			print(r2_strings[i]);
}

/* Nec (=Ncr? which is limited to [0,8]) dummy bytes before lowering CS,
 * as described in sandisk doc, 5.4. */
static void sd_nec()
{
	int i;
	for (i=0; i<8; i++)
		(void)spi_txrx((u8)0xff);
}


static int sd_init(hwif *hw)
{
	unsigned int i;
	int r;
	u32 r7;
	u32 r3;
	int tries;
	u32 hcs = 0;

	hw->capabilities = 0;

	/* start with 100-400 kHz clock */
	spi_set_speed(SD_SPEED_400KHZ);

	print("cmd0 - reset.. ");
	spi_cs_high();
	/* 74+ clocks with CS high */
	for (i=0; i<10; i++)
		(void)spi_txrx((u8)0xff);

	/* reset */
	spi_cs_low();
	sd_cmd((u8)0, 0);
	r = (int) sd_get_r1();
	sd_nec();
	spi_cs_high();
	if (r == 0xff)
		goto err_spi;
	if (r != 0x01) {
		print("fail\n");
		print_r1((u8)r);
		goto err;
	}
	print("success\n");


	print("cmd8 - voltage.. ");
	/* ask about voltage supply */
	spi_cs_low();
	sd_cmd((u8)8, 0x1aa /* VHS = 1 */);
	r = (int) sd_get_r7(&r7);
	sd_nec();
	spi_cs_high();
	hw->capabilities |= CAP_VER2_00;
	if (r == 0xff)
		goto err_spi;
	if (r == 0x01)
		print("success, SD v2.x\n");
	else if ((r & 0x4) != 0) {
		hw->capabilities &= ~CAP_VER2_00;
		print("not implemented, SD v1.x\n");
	} else {
		print("fail\n");
		print_r1((u8)r);
		return -2;
	}


	print("cmd58 - ocr.. ");
	/* ask about voltage supply */
	spi_cs_low();
	sd_cmd((u8)58, 0);
	r = (int) sd_get_r3(&r3);
	sd_nec();
	spi_cs_high();
	if (r == 0xff)
		goto err_spi;
	if (r != 0x01 && ((r & 0x4) == 0)) { /* allow it to not be implemented - old cards */
		print("fail\n");
		print_r1((u8)r);
		return -2;
	}
	else {
		for (i=4; i<=23; i++)
			if ((r3 & 1<<i) != 0)
				break;
		print("Vdd voltage window: ");printhex((unsigned int)12+i);
		for (i=23; i>=4; i--)
			if ((r3 & 1<<i) != 0)
				break;
		/* CCS shouldn't be valid here yet */
      printhex((unsigned int)13+i);print("deciVolt, CCS:");
      printhex((unsigned int)(r3>>30)&1); print(" power up status: ");printhex((unsigned int)r3>>31);
		print("\nsuccess\n");
	}


	print("acmd41 - hcs.. ");
	tries = 1000;
	/* say we support SDHC */
	if ((hw->capabilities & CAP_VER2_00) != 0)
		hcs = (u32)(1<<30);

	/* needs to be polled until in_idle_state becomes 0 */
	do {
		/* send we don't support SDHC */
		spi_cs_low();
		/* next cmd is ACMD */
		sd_cmd((u8)55, 0);
		r = (int) sd_get_r1();
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		/* well... it's probably not idle here, but specs aren't clear */
		if ((r & 0xfe) != 0) {
			print("fail\n");
			print_r1((u8)r);
			goto err;
		}

		spi_cs_low();
		sd_cmd((u8)41, hcs);
		r = (int) sd_get_r1();
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		if ((r & 0xfe) != 0) {
			print("fail\n");
			print_r1((u8)r);
			goto err;
		}
	} while (r != 0 && tries-- != 0);
	if (tries == -1) {
		print("timeouted\n");
		goto err;
	}
	print("success\n");

	/* Seems after this card is initialized which means bit 0 of R1
	 * will be cleared. Not too sure. */


	if ((hw->capabilities & CAP_VER2_00) != 0) {
		print("cmd58 - ocr, 2nd time.. ");
		/* ask about voltage supply */
		spi_cs_low();
		sd_cmd((u8)58, 0);
		r = (int) sd_get_r3(&r3);
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		if ((r & 0xfe) != 0) {
			print("fail\n");
			print_r1((u8)r);
			return -2;
		}
		else {
#if 1
			for (i=4; i<=23; i++)
				if ((r3 & 1<<i) != 0)
					break;
			print("Vdd voltage window: ");printhex((unsigned int)12+i);
			for (i=23; i>=4; i--)
				if ((r3 & 1<<i) != 0)
					break;
			/* CCS shouldn't be valid here yet */
         printhex((unsigned int)13+i);print("deciVolt, CCS:");
         printhex((unsigned int)(r3>>30)&1); print(" power up status: ");printhex((unsigned int)r3>>31);
			/* XXX power up status should be 1 here, since we're finished initializing, but it's not. WHY?
			that means CCS is invalid, so we'll set CAP_SDHC later */
#endif
			if ((r3>>30 & 1) != 0) {
				hw->capabilities |= CAP_SDHC;
			}

			print("success\n");
		}
	}


	/* with SDHC block length is fixed to 1024 */
	if ((hw->capabilities & CAP_SDHC) == 0) {
		print("cmd16 - block length.. ");
		spi_cs_low();
		sd_cmd((u8)16, 512);
		r = (int) sd_get_r1();
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		if ((r & 0xfe) != 0) {
			print("fail\n");
			print_r1((u8)r);
			goto err;
		}
		print("success\n");
	}


	print("cmd59 - enable crc.. ");
	/* crc on */
	spi_cs_low();
	sd_cmd((u8)59, 0);
	r = (int) sd_get_r1();
	sd_nec();
	spi_cs_high();
	if (r == 0xff)
		goto err_spi;
	if ((r & 0xfe) != 0) {
		print("fail\n");
		print_r1((u8) r);
		goto err;
	}
	print("success\n");


	/* now we can up the clock to <= 25 MHz */
	spi_set_speed(SD_SPEED_25MHZ);

	return 0;

 err_spi:
	print("fail spi\n");
	return -1;
 err:
	return -2;
}

static int sd_read_status(/*@unused@*/ hwif *hw)
{
	u16 r2;

	spi_cs_low();
	sd_cmd((u8)13, 0);
	r2 = sd_get_r2();
	sd_nec();
	spi_cs_high();
	if ((r2 & 0x8000) != 0)
		return -1;
	if (r2 != 0)
		print_r2(r2);

	return 0;
}

/*@-compdef -mustdefine@*/
/* 0xfe marks data start, then len bytes of data and crc16 */
static int sd_get_data(/*@unused@*/hwif *hw, /*@out@*/u8 *buf, int len)
{
	int tries = 20000;
	u8 r;
	u16 _crc16;
	u16 calc_crc;
	int i;

	while (tries-- != 0) {
		r = spi_txrx((u8)0xff);
		if (r == (u8) 0xfe)
			break;
	}
	if (tries < 0) {
		return -1;
   }

	for (i=0; i<len; i++)
		buf[i] = spi_txrx((u8)0xff);

	_crc16 = (u16) spi_txrx((u8)0xff) << 8;
	_crc16 |= spi_txrx((u8)0xff);

	calc_crc = crc16(buf, len);
	if (_crc16 != calc_crc) {
      print(__func__);print(", crcs differ: ");printhex((unsigned int)_crc16);printhex((unsigned int)calc_crc);
      print("len: "); printhex((unsigned int)len);print("\n");
		return -1;
	}

	return 0;
}
/*@=compdef =mustdefine@*/

static int sd_put_data(/*@unused@*/hwif *hw, const u8 *buf, int len)
{
	u8 r = (u8) 0;
	int tries = 10;
#if 0
	u8 b[16];
	int bi = 0;
#endif
	u16 crc;

	(void) spi_txrx((u8)0xfe); /* data start */

	while (len-- != 0)
		(void)spi_txrx(*buf++);

	crc = crc16(buf, len);
	/* crc16 */
	(void)spi_txrx((u8)(crc>>8));
	(void)spi_txrx((u8)crc);

	/* normally just one dummy read in between... specs don't say how many */
	while (tries-- != 0) {
#if 0
		b[bi++] =
#endif
      r = spi_txrx((u8)0xff);
		if (r != (u8) 0xff)
			break;
	}
	if (tries < 0)
		return -1;

	/* poll busy, about 300 reads for 256 MB card */
	tries = 100000;
	while (tries-- != 0) {
		if (spi_txrx((u8)0xff) == (u8)0xff)
			break;
	}
	if (tries < 0)
		return -2;

	/* data accepted, WIN */
	if ((r & 0x1f) == (u8)0x05)
		return 0;

	return (int) r;
}

static int sd_read_csd(/*@unused@*/hwif *hw)
{
	u8 buf[16];
	int r;
	unsigned int capacity;

	spi_cs_low();
	sd_cmd((u8)9, 0);
	r = (int) sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		return -1;
	}
	if ((r & 0xfe) != 0) {
		spi_cs_high();
		print(__func__);
		print_r1((u8) r);
		return -2;
	}

	r = sd_get_data(hw, buf, 16);
	sd_nec();
	spi_cs_high();
	if (r == -1) {
		print("failed to get csd\n");
		return -3;
	}

	if ((buf[0] >> 6) + (u8)1 == (u8)1) {
	/* CSD v1 */
#if 0
	printf("CSD: CSD v%i taac:%02x, nsac:%i, tran:%02x, classes:%02x%x, read_bl_len:%i, "
		"read_bl_part:%i, write_blk_misalign:%i, read_blk_misalign:%i, dsr_imp:%i, "
		"c_size:%i, vdd_rmin:%i, vdd_rmax:%i, vdd_wmin:%i, vdd_wmax:%i, "
		"c_size_mult:%i, erase_blk_en:%i, erase_s_size:%i, "
		"wp_grp_size:%i, wp_grp_enable:%i, r2w_factor:%i, write_bl_len:%i, write_bl_part:%i, "
		"filef_gpr:%i, copy:%i, perm_wr_prot:%i, tmp_wr_prot:%i, filef:%i\n",
			(buf[0] >> 6) + 1,
			buf[1], buf[2], buf[3],
			buf[4], buf[5] >> 4, 1<<(buf[5] & 0xf), /* classes, read_bl_len */
			buf[6]>>7, (buf[6]>>6)&1, (buf[6]>>5)&1, (buf[6]>>4)&1,
			(buf[6]&0x3)<<10 | buf[7]<<2 | buf[8]>>6, /* c_size */
			(buf[8]&0x38)>>3, buf[8]&0x07, buf[9]>>5, (buf[9]>>2)&0x7,
			1<<(2+(((buf[9]&3) << 1) | buf[10]>>7)), /* c_size_mult */
			(buf[10]>>6)&1,
			((buf[10]&0x3f)<<1 | buf[11]>>7) + 1, /* erase sector size */
			(buf[11]&0x7f) + 1, /* write protect group size */
			buf[12]>>7, 1<<((buf[12]>>2)&7),
			1<<((buf[12]&3)<<2 | buf[13]>>6), /* write_bl_len */
			(buf[13]>>5)&1,
			buf[14]>>7, (buf[14]>>6)&1, (buf[14]>>5)&1, (buf[14]>>4)&1,
			(buf[14]>>2)&3 /* file format */);
#endif
	capacity = ((((unsigned int)buf[6]&0x3)<<10 | (unsigned int)buf[7]<<2 | (unsigned int)buf[8]>>6)+1U) << (2U+((((unsigned int)buf[9]&3) << 1) | (unsigned int)buf[10]>>7)) << (((unsigned int)buf[5] & 0xf) - 9U);
	/* ^ = (c_size+1) * 2**(c_size_mult+2) * 2**(read_bl_len-9) */

	} else {
	/* CSD v2 */
		/* this means the card is HC */
		hw->capabilities |= CAP_SDHC;
#if 0
	printf("CSD: CSD v%i taac:%02x, nsac:%i, tran:%02x, classes:%02x%x, read_bl_len:%i, "
		"read_bl_part:%i, write_blk_misalign:%i, read_blk_misalign:%i, dsr_imp:%i, "
		"c_size:%i, erase_blk_en:%i, erase_s_size:%i, "
		"wp_grp_size:%i, wp_grp_enable:%i, r2w_factor:%i, write_bl_len:%i, write_bl_part:%i, "
		"filef_gpr:%i, copy:%i, perm_wr_prot:%i, tmp_wr_prot:%i, filef:%i\n",
			(buf[0] >> 6) + 1,
			buf[1], buf[2], buf[3],
			buf[4], buf[5] >> 4, 1<<(buf[5] & 0xf), /* classes, read_bl_len */
			buf[6]>>7, (buf[6]>>6)&1, (buf[6]>>5)&1, (buf[6]>>4)&1,
			buf[7]<<16 | buf[8]<<8 | buf[9], /* c_size */
			(buf[10]>>6)&1,
			((buf[10]&0x3f)<<1 | buf[11]>>7) + 1, /* erase sector size */
			(buf[11]&0x7f) + 1, /* write protect group size */
			buf[12]>>7, 1<<((buf[12]>>2)&7),
			1<<((buf[12]&3)<<2 | buf[13]>>6), /* write_bl_len */
			(buf[13]>>5)&1,
			buf[14]>>7, (buf[14]>>6)&1, (buf[14]>>5)&1, (buf[14]>>4)&1,
			(buf[14]>>2)&3 /* file format */);
#endif
	capacity = (unsigned int)(buf[7]<<16 | buf[8]<<8 | buf[9]); /* in 512 kB */
	capacity *= 1024; /* in 512 B sectors */

	}

	print("capacity = ");printhex(capacity/2);print("kB\n");
	hw->sectors = (int) capacity;

	/* if erase_blk_en = 0, then only this many sectors can be erased at once
	 * this is NOT yet tested */
	hw->erase_sectors = 1;
	if (((buf[10]>>6)&1) == (u8)0)
		hw->erase_sectors = (int) ((buf[10]&0x3f)<<1 | buf[11]>>7) + 1;

	return 0;
}

static int sd_read_cid(hwif *hw)
{
	u8 buf[16];
	int r;

	spi_cs_low();
	sd_cmd((u8)10, 0);
	r = (int) sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		return -1;
	}
	if ((r & 0xfe) != 0) {
		spi_cs_high();
		print(__func__);
		print_r1((u8) r);
		return -2;
	}

	r = sd_get_data(hw, buf, 16);
	sd_nec();
	spi_cs_high();
	if (r == -1) {
		print("failed to get cid\n");
		return -3;
	}

#if 0
	printf("CID: mid:%x, oid:%c%c, pnm:%c%c%c%c%c, prv:%i.%i, psn:%02x%02x%02x%02x, mdt:%i/%i\n",
			buf[0], buf[1], buf[2],			/* mid, oid */
			buf[3], buf[4], buf[5], buf[6], buf[7],	/* pnm */
			buf[8] >> 4, buf[8] & 0xf,		/* prv */
			buf[9], buf[10], buf[11], buf[12],	/* psn */
			2000 + (buf[13]<<4 | buf[14]>>4), 1 + (buf[14] & 0xf));
#endif

	return 0;
}


static int sd_readsector(hwif *hw, u32 address, u8 *buf)
{
	int r;

	spi_cs_low();
	if ((hw->capabilities & CAP_SDHC) != 0)
		sd_cmd((u8)17, address); /* read single block */
	else
		sd_cmd((u8)17, address*512); /* read single block */

	r = (int) sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		r = -1;
		goto fail;
	}
	if ((r & 0xfe) != 0) {
		spi_cs_high();
		print(__func__);print("\n");
		print_r1((u8)r);
		r = -2;
		goto fail;
	}

	r = sd_get_data(hw, buf, 512);
	sd_nec();
	spi_cs_high();
	if (r == -1) {
		r = -3;
		goto fail;
	}

	return 0;
 fail:
	print("failed to read sector ");printhex((unsigned int)address);
   print("err: ");printhex((unsigned int)r);print("\n");
	return r;
}

static int sd_writesector(hwif *hw, u32 address, const u8 *buf)
{
	int r;

	spi_cs_low();
	if ((hw->capabilities & CAP_SDHC) != 0)
		sd_cmd((u8)24, address); /* write block */
	else
		sd_cmd((u8)24, address*512); /* write block */

	r = (int) sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		r = -1;
		goto fail;
	}
	if ((r & 0xfe) != 0) {
		spi_cs_high();
		print(__func__);print("\n");
		print_r1((u8)r);
		r = -2;
		goto fail;
	}

	(void)spi_txrx((u8)0xff); /* Nwr (>= 1) high bytes */
	r = sd_put_data(hw, buf, 512);
	sd_nec();
	spi_cs_high();
	if (r != 0) {
		print("sd_put_data returned: ");printhex((unsigned int)r);print("\n");
		r = -3;
		goto fail;
	}

	/* efsl code is weird shit, 0 is error in there?
	 * not that it's properly handled or anything,
	 * and the return type is char, fucking efsl */
	return 0;
 fail:
	print("failed to write sector ");printhex((unsigned int)address);
   print("err:");printhex((unsigned int)r);print("\n");
	return r;
}


/*** public API - on top of sd/spi code ***/

int hwif_init(hwif* hw)
{
	int tries = 10;

	if (hw->initialized != 0)
		return 0;

	spi_init();

	while (tries-- != 0) {
		if (sd_init(hw) == 0)
			break;
	}
	if (tries == -1)
		return -1;

	/* read status register */
	(void) sd_read_status(hw);

	(void) sd_read_cid(hw);
	if (sd_read_csd(hw) != 0)
		return -1;

	hw->initialized = 1;
	return 0;
}

int sd_read(hwif* hw, u32 address, u8 *buf)
{
	int r;
	int tries = 10;

	r = sd_readsector(hw, address, buf);

	while (r < 0 && (tries-- != 0)) {
		if (sd_init(hw) != 0)
			continue;

		/* read status register */
		(void) sd_read_status(hw);

		r = sd_readsector(hw, address, buf);
	}
	if (tries == -1)
		print(__func__);print(": couldn't read sector ");printhex((unsigned int)address);print("\n");

	return r;
}

int sd_write(hwif* hw, u32 address,const u8 *buf)
{
	int r;
	int tries = 10;

	r = sd_writesector(hw, address, buf);

	while (r < 0 && (tries-- != 0)) {
		if (sd_init(hw) != 0)
			continue;

		/* read status register */
		(void) sd_read_status(hw);

		r = sd_writesector(hw, address, buf);
	}
	if (tries == -1)
		print(__func__);print(": couldn't write sector ");printhex((unsigned int)address);print("\n");

	return r;
}
