#include "kernel.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct CLOCK_CTL *clock_ctl;

#define CLOCK_FLAGS_FREE		0
#define CLOCK_FLAGS_ALLOC		1
#define CLOCK_FLAGS_USING		2

void init_pit(void)
{
	int i;
	char cw = 0;
	struct CLOCK *c;
	clock_ctl = (struct CLOCK_CTL*)memman_alloc(memman, sizeof(struct CLOCK_CTL));
	io_out8(PIT_CTRL, 0x34);        /*IRQ0 CLOCK interrupt
										AL = 0x34：OUT(0x43, AL);
										AL = low ；OUT(0x40, AL);
										AL = high；OUT(0x40, AL);
										CLK = CRY_FREQ/AX
                                    */
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	clock_ctl->count = 0;
	for (i = 0; i < MAX_CLOCK; i++) {
		clock_ctl->clock_list[i].flag_usage = CLOCK_FLAGS_FREE;
	}
	c = clock_alloc();
	c->time_out = 0xffffffff;
	c->flag_usage = CLOCK_FLAGS_USING;
	c->next = 0;
	clock_ctl->c0 = c;
	clock_ctl->next = 0xffffffff; 
	cw = io_in8(PIC0_IMR);
	cw = cw & 0xfe;
	io_out8(PIC0_IMR, cw);
	return;
}

struct CLOCK *clock_alloc(void)
{
	int i;
	for (i = 0; i < MAX_CLOCK; i++) {
		if (clock_ctl->clock_list[i].flag_usage == CLOCK_FLAGS_FREE) {
			clock_ctl->clock_list[i].flag_usage = CLOCK_FLAGS_ALLOC;
			clock_ctl->clock_list[i].flags2 = 0;
			return &clock_ctl->clock_list[i];
		}
	}
	return 0;
}

void clock_free(struct CLOCK *clock)
{
	clock->flag_usage = CLOCK_FLAGS_ALLOC;
	return;
}

void clock_init(struct CLOCK *clock, struct FIFO32 *fifo, int data)
{
	clock->fifo = fifo;
	clock->data = data;
	return;
}

void clock_set_time(struct CLOCK *clock, unsigned int time_out)
{
	int e;
	struct CLOCK *c, *s;
	clock->time_out = time_out + clock_ctl->count;
	clock->flag_usage = CLOCK_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	c = clock_ctl->c0;
	if (clock->time_out <= c->time_out) {
		clock_ctl->c0 = clock;
		clock->next = c;
		clock_ctl->next = clock->time_out;
		io_store_eflags(e);
		return;
	}
	for (;;) {
		s = c;
		c = c->next;
		if (clock->time_out <= c->time_out) {
			s->next = clock;
			clock->next = c;
			io_store_eflags(e);
			return;
		}
	}
}

void inthandler20(int *esp)
{
	int flag_switch = 0;
	struct CLOCK *clock;
	io_cli();
	io_out8(PIC0_OCW2, 0x60);
	clock_ctl->count++;
	if (clock_ctl->next > clock_ctl->count) {
		return;
	}
	clock = clock_ctl->c0;
	
	for (;;) {
		if (clock->time_out > clock_ctl->count) {
			break;
		}
		if(clock != task_clock){
			fifo32_put(clock->fifo, clock->data);
		}else{
			flag_switch = 1;
		}
		clock->flag_usage = CLOCK_FLAGS_ALLOC;
		clock = clock->next;
	}
	clock_ctl->c0 = clock;
	clock_ctl->next = clock->time_out;
	io_sti();
	if(flag_switch == 1){
		task_switch();
	}
	return;
}
