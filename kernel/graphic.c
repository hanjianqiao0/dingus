#include "kernel.h"

void init_palette(void){
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:black */
		0xff, 0x00, 0x00,	/*  1:red */
		0x00, 0xff, 0x00,	/*  2:green */
		0xff, 0xff, 0x00,	/*  3:yellow */
		0x00, 0x00, 0xff,	/*  4:blue */
		0xff, 0x00, 0xff,	/*  5:purple */
		0x00, 0xff, 0xff,	/*  6:iceblue */
		0xff, 0xff, 0xff,	/*  7:white */
		0xc6, 0xc6, 0xc6,	/*  8:silver */
		0x84, 0x00, 0x00,	/*  9:maroon */
		0x00, 0x84, 0x00,	/* 10:deepgreen */
		0x84, 0x84, 0x00,	/* 11:olivegreen */
		0x00, 0x00, 0x84,	/* 12:darkblue */
		0x84, 0x00, 0x84,	/* 13:darkpurple */
		0x00, 0x84, 0x84,	/* 14:chartreuse */
		0x84, 0x84, 0x84	/* 15:darksilver */
	};
	set_palette(0, 15, table_rgb);      /* 0 - 15 */
	return;
}

void set_palette(int start, int end, unsigned char *rgb){
	int i, eflags;
	eflags = io_load_eflags();
	io_cli();					/* During initalization, the interruption should not take place*/
	/*
	Access pallete：
	Set color:
	1. Cli
	2. Write the color number to 0x03c8, then write to 0x03c9 in RGB order.
	3. If you want to set the color next to the previous, then write to 0x03c9 in RGB order.
	   If you want to set another color, goto step 2.
	4. Sti
	
	Read color:
	Steps are similar, and you just need to read 0x03c9 insted of writing.
	*/
	io_out8(0x03c8, start);
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);
	return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1){
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char ascii[4096];

	for (; *s != 0x00; s++) {
		putfont8(vram, xsize, x, y, c, ascii + *s * 16);
		x += 8;
	}
	return;
}

void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}

void putHex(char *vram, int xsize, int x, int y, char c, unsigned char *s){
	extern char ascii[4096];
	if((*s)/16 < 10){
		putfont8(vram, xsize, x, y, c, ascii + '0' * 16 + (*s)/16 * 16);
	}else{
		putfont8(vram, xsize, x, y, c, ascii + ('A' - 10) * 16 + (*s)/16 * 16);
	}
	x += 8;
	if((*s)%16 < 10){
		putfont8(vram, xsize, x, y, c, ascii + '0' * 16 + (*s)%16 * 16);
	}else{
		putfont8(vram, xsize, x, y, c, ascii + ('A' - 10) * 16 + (*s++)%16 * 16);
	}
}

void putHexs(char *vram, int xsize, int x, int y, char c, unsigned char *s, int len){
	extern char ascii[4096];
	for (; len > 0x00; len--) {
		putHex(vram, xsize, x, y, c, s++);
		x += 16;
		putfont8(vram, xsize, x, y, c, ascii + ' ' * 16);
		x += 8;
		if(len % 27 == 0){
			y += 24;
			x = 0;
		}
	}
}
