#include <stdio.h>
#include "kernel.h"

struct BOOTINFO *binfo = (struct BOOTINFO*) ADR_BOOTINFO;
void panic_s(char *s){
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 8, binfo->scrny - 24, binfo->scrnx - 8, binfo->scrny - 8);
	putfonts8_asc(binfo->vram, binfo->scrnx, 16, binfo->scrny - 24, COL8_848400, s);
	return;
}

void panic_i(int i){
	char s[40];
	sprintf(s, "%d", i);
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 8, binfo->scrny - 24, binfo->scrnx - 8, binfo->scrny - 8);
	putfonts8_asc(binfo->vram, binfo->scrnx, 16, binfo->scrny - 24, COL8_848400, s);
	return;
}

void panic_ui(unsigned int ui){
	char s[40];
	sprintf(s, "%d", ui);
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 8, binfo->scrny - 24, binfo->scrnx - 8, binfo->scrny - 8);
	putfonts8_asc(binfo->vram, binfo->scrnx, 16, binfo->scrny - 24, COL8_848400, s);
	return;
}
