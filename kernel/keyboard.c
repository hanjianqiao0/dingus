#include "kernel.h"
#define PORT_KEYSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

struct FIFO32 *fifo_keyboard;
int keyboard_data0;

void inthandler21(int *esp)
{
	int data;
	io_out8(PIC0_OCW2, 0x61);	/* Notify that interrupt received, IRQ number +0x60 */
	data = io_in8(PORT_KEYDAT);
	fifo32_put(fifo_keyboard, data + keyboard_data0);
	return;
}

void wait_KBC_sendready(void)
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(struct FIFO32 *fifo, int data0)
{
	char cw = 0;
	fifo_keyboard = fifo;
	keyboard_data0 = data0;
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	cw = io_in8(PIC0_IMR);
	cw = cw & 0xfd;
	io_out8(PIC0_IMR, cw);
	return;
}
