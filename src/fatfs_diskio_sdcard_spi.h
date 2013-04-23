#ifndef SDCARD_H
#define SDCARD_H
struct hwif {
	int initialized;
	int sectors;
	int erase_sectors;
	int capabilities;
};
typedef struct hwif hwif;

#define CAP_VER2_00  (1<<0)
#define CAP_SDHC     (1<<1)

int hwif_init(hwif* hw);
int sd_read(hwif* hw, u32 address, u8 *buf);
int sd_write(hwif* hw, u32 address,const u8 *buf);

#endif /* SDCARD_H */
