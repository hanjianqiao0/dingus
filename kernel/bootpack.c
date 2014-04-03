#include <stdio.h>
#include"kernel.h"

void mouse(void);
void keyboard(void);
void sysclock_task(void);

void HariMain(){
	int *buf_clock;
	char *s;
	struct FIFO32 fifo_clock;
	struct CLOCK *clock;
	struct TASK *task_keyboard, *task_mouse, *task_rtc;
	int count = 0;

	init_all();
	
	buf_clock = (int *) memman_alloc(memman, 40);
	s = (char *) memman_alloc(memman, 40);
	
	fifo32_init(&fifo_clock, 10, buf_clock);
	clock = clock_alloc();
	clock_init(clock, &fifo_clock, 1);
	clock_set_time(clock, 100);
	

	io_sti();

	task_run(first_task(), 2);

	task_keyboard = task_alloc();
	task_keyboard->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	task_keyboard->tss.eip = (int) &keyboard;
	task_keyboard->tss.es = 1 * 8;
	task_keyboard->tss.cs = 2 * 8;
	task_keyboard->tss.ss = 1 * 8;
	task_keyboard->tss.ds = 1 * 8;
	task_keyboard->tss.fs = 1 * 8;
	task_keyboard->tss.gs = 1 * 8;
	task_run(task_keyboard, 2);

	task_mouse = task_alloc();
	task_mouse->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	task_mouse->tss.eip = (int) &mouse;
	task_mouse->tss.es = 1 * 8;
	task_mouse->tss.cs = 2 * 8;
	task_mouse->tss.ss = 1 * 8;
	task_mouse->tss.ds = 1 * 8;
	task_mouse->tss.fs = 1 * 8;
	task_mouse->tss.gs = 1 * 8;
	task_run(task_mouse, 2);
	
	task_rtc = task_alloc();
	task_rtc->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	task_rtc->tss.eip = (int) &sysclock_task;
	task_rtc->tss.es = 1 * 8;
	task_rtc->tss.cs = 2 * 8;
	task_rtc->tss.ss = 1 * 8;
	task_rtc->tss.ds = 1 * 8;
	task_rtc->tss.fs = 1 * 8;
	task_rtc->tss.gs = 1 * 8;
	task_run(task_rtc, 2);

	for(;;){
		if(fifo32_status(&fifo_clock) != 0){
			panic_i(count += fifo32_get(&fifo_clock));
			clock_set_time(clock, 10);
		}
	}
}

void keyboard(void){
	struct BOOTINFO *binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	int *buf_keyboard;
	char *s;
	struct FIFO32 fifo_keyboard;
	buf_keyboard = (int *) memman_alloc(memman, 40);
	s = (char *) memman_alloc(memman, 40);
	fifo32_init(&fifo_keyboard, 10, buf_keyboard);
	init_keyboard(&fifo_keyboard, 0);
	for(;;){
		if(fifo32_status(&fifo_keyboard) != 0){
			sprintf(s, "%X", fifo32_get(&fifo_keyboard));
			boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 10, 10, 26, 26);
			putfonts8_asc(binfo->vram, binfo->scrnx, 10,  10, COL8_848400, s);
		}
	}
}
void mouse(void){
	struct BOOTINFO *binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	int *buf_mouse;
	char *s;
	struct FIFO32 fifo_mouse;
	struct MOUSE_DEC mdec;
	buf_mouse = (int *) memman_alloc(memman, 40);
	s = (char *) memman_alloc(memman, 40);
	fifo32_init(&fifo_mouse, 10, buf_mouse);
	enable_mouse(&fifo_mouse, 0, &mdec);
	for(;;){
		if(fifo32_status(&fifo_mouse) != 0){
			if(mouse_decode(&mdec, fifo32_get(&fifo_mouse)) == 1){
				sprintf(s, "X:%d Y:%d BTN:%d Phase:%d", mdec.x, mdec.y, mdec.btn, mdec.phase);
				boxfill8(binfo->vram, binfo->scrnx, COL8_FF0000, 16, 64, 220, 80);
				putfonts8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_C6C6C6, s);
			}
		}
	}
}

void sysclock_task(void){
	int i, j;
	unsigned char t[7];
	char err, cnt;
	unsigned char *s;
	static unsigned char adr[7] = { 0x00, 0x02, 0x04, 0x07, 0x08, 0x09, 0x32 };
	static unsigned char max[7] = { 0x60, 0x59, 0x23, 0x31, 0x12, 0x99, 0x99 };
	struct CLOCK *clock_rtc;
	int *buf_rtc;
	struct FIFO32 fifo_rtc;
	struct BOOTINFO *binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	buf_rtc = (int *) memman_alloc(memman, 40);
	s = (unsigned char *) memman_alloc(memman, 6);
	fifo32_init(&fifo_rtc, 10, buf_rtc);
	clock_rtc = clock_alloc();
	clock_init(clock_rtc, &fifo_rtc, 1);
	clock_set_time(clock_rtc, 100);

	for (;;) {
		if (fifo32_status(&fifo_rtc) != 0){
			i = fifo32_get(&fifo_rtc);
			if (i == 1) {
				for (cnt = 0; cnt < 3; cnt++) {
					err = 0;
					for (j = 0; j < 7; j++) {
						io_out8(0x70, adr[j]);
						t[j] = io_in8(0x71);
					}
					for (j = 0; j < 7; j++) {
						io_out8(0x70, adr[j]);
						if (t[j] != io_in8(0x71) || (t[j] & 0x0f) > 9 || t[j] > max[j]) {
							err = 1;
						}
					}
					if (err == 0) {
						break;
					}
				}
				sprintf(s, "%02X:%02X\0", t[2], t[1]);
				boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 100, 100, 164, 116);
				putfonts8_asc(binfo->vram, binfo->scrnx, 100,  100, COL8_848400, s);
				clock_set_time(clock_rtc, 100);
			}
		}
	}
}
