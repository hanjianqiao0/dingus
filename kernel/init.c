#include "kernel.h"

struct MEMMAN *memman;
int init_all(void){
	init_mem();
	init_hw();
	init_task();
	return 0;
}

int init_hw(void){
	init_pic();
	init_palette();
	init_gdtidt();
	init_pit();
	return 0;
}

int init_mem(void){
	memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int memtotal;
	memtotal = memtest(0x00400000, 0xffffffff);
	memman_init(memman);
//	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	return 0;
}

int init_task(void){
	task_init(memman);
	return 0;
}
