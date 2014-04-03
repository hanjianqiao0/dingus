#include "kernel.h"

struct MEMMAN *memman;		//Structure of memory manager
int init_all(void){		//Initialize all hardware and software
	init_mem();
	init_hw();
	init_task();
	return 0;
}

int init_hw(void){		//Initialize hardware
	init_pic();
	init_palette();
	init_gdtidt();
	init_pit();
	return 0;
}

int init_mem(void){		//Initialize memory
	memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int memtotal;
	memtotal = memtest(0x00400000, 0xffffffff);
	memman_init(memman);
//	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	return 0;
}

int init_task(void){		//Initialize task manager
	task_init(memman);
	return 0;
}
