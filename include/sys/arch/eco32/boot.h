/**
 * $Id$
 */

#ifndef ECO32_BOOT_H_
#define ECO32_BOOT_H_

#include <esc/common.h>

#define MAX_PATH_LEN	20
#define MAX_PROG_COUNT	8

/* a program we should load */
typedef struct {
	char path[MAX_PATH_LEN];
	uint id;
	uint start;
	uint size;
} sLoadProg;

typedef struct {
	uint progCount;
	const sLoadProg *progs;
	uint memSize;
	uint diskSize;
} sBootInfo;

#define BL_HDD_ID	0
#define BL_FS_ID	1
#define BL_RTC_ID	2
#define BL_K_ID		3

void boot_init(const sBootInfo *binfo);
const sBootInfo *boot_getInfo(void);

#endif /* ECO32_BOOT_H_ */
