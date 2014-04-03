#include "kernel.h"

void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  ); /* Disable all interrupt */
	io_out8(PIC1_IMR,  0xff  ); /* Disable all interrupt */

	io_out8(PIC0_ICW1, 0x11  ); /* Edge trigger mode */
	io_out8(PIC0_ICW2, 0x20  ); /* INT20-27 handle IRQ0-7 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1 connected by IRQ2 */
	io_out8(PIC0_ICW4, 0x01  ); /* no buffer */

	io_out8(PIC1_ICW1, 0x11  ); /* Edge trigger mode */
	io_out8(PIC1_ICW2, 0x28  ); /* INT28-2f handle IRQ8-15 */
	io_out8(PIC1_ICW3, 2     ); /* PIC1 connected by IRQ2 */
	io_out8(PIC1_ICW4, 0x01  ); /* no buffer */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 Disable all interrupt except PIC1 */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 Disable all interrupt */

	return;
}
