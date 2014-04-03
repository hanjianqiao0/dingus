/* setup.nas */
#define ADR_BOOTINFO	0x00000ff0
#define ADR_DISKIMG		0x00100000
struct BOOTINFO { /* 0x0ff0-0x0fff */
	char cyls;
	char leds;
	char vmode;
	char reserve;
	short scrnx, scrny;
	char *vram;
};

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler2c(void);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void far_jmp(int eip, int cs);

/* fifo.c */
struct FIFO32 {
	int *buf;
	int p, q, size, free, flags;
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

/* graphic.c */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen8(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize);
void putHex(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void putHexs(char *vram, int xsize, int x, int y, char c, unsigned char *s, int len);
#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

/* dsctbl.c */
struct SEGMENT_DESCRIPTOR {
    /*
    ar low£º
		0x00    Free
		0x92    System£¬RW, not executable
		0x9a    System£¬R, executable
		0xf2    Application£¬RW, not executable
		0xfa    Application, R, executable
    */
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_LDT			0x0082
#define AR_TSS32		0x0089
#define AR_INTGATE32	0x008e

/* eint.c */
void init_pic(void);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

/* keyboard.c */
#define PORT_KEYDAT		0x0060
#define PORT_KEYCMD		0x0064
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo, int data0);

/* mouse.c */
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void inthandler2c(int *esp);
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

/* clock.c */
#define MAX_CLOCK		500
struct CLOCK{
	struct CLOCK *next;
	unsigned int time_out;
	char flag_usage, flags2;
	struct FIFO32 *fifo;
	int data;
};
struct CLOCK_CTL {
	unsigned int count, next;
	struct CLOCK *c0;
	struct CLOCK clock_list[MAX_CLOCK];
};
extern struct CLOCK_CTL *clock_ctl;
void init_pit(void);
struct CLOCK *clock_alloc(void);
void clock_init(struct CLOCK *clock, struct FIFO32 *fifo, int data);
void clock_set_time(struct CLOCK *clock, unsigned int time_out);
void inthandler20(int *esp);

/* init.c */
int init_all(void);
int init_hw(void);
int init_mem(void);
int init_task(void);

/* sys.c */
void panic_s(char *s);
void panic_i(int i);
void panic_ui(unsigned int ui);

/* memory.c */
#define MEMMAN_FREES		4090
#define MEMMAN_ADDR			0x003c0000
extern struct MEMMAN *memman;
struct FREE_UNIT{
	unsigned int addr, size;
};
struct MEMMAN{
	unsigned int frees, maxfrees, lostsize, losts;
	struct FREE_UNIT free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total_free(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/* task.c */
#define MAX_TASKS		1000
#define TASK_GDT0		3
#define MAX_TASKS_LV	100
#define MAX_TASK_LEVELS	10
struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};
struct TASK {
	int sel, flags;
	int priority;
	struct FIFO32 fifo;
	struct TSS32 tss;
	struct SEGMENT_DESCRIPTOR ldt[2];
	int ds_base, cons_stack;
};
struct TASK_CTL {
	int now_task, running;
	struct TASK *tasks[MAX_TASKS];
	struct TASK tasks0[MAX_TASKS];
};
extern struct TASK_CTL *task_ctl;
extern struct CLOCK *task_clock;
struct TASK *task_now(void);
void task_remove(struct TASK *task);
void task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task, int priority);
void task_switch(void);
void task_sleep(struct TASK *task);
struct TASK *first_task(void);

/* harddisk.c */
/* Hd controller regs. Ref: IBM AT Bios-listing */
#define HD_DATA 0x1f0		/* _CTL when writing */
#define HD_ERROR 0x1f1		/* see err-bits */
#define HD_NSECTOR 0x1f2	/* nr of sectors to read/write */
#define HD_SECTOR 0x1f3		/* starting sector */
#define HD_LCYL 0x1f4		/* starting cylinder */
#define HD_HCYL 0x1f5		/* high byte of starting cyl */
#define HD_CURRENT 0x1f6	/* 101dhhhh , d=drive, hhhh=head */
#define HD_STATUS 0x1f7		/* see status-bits */
#define HD_PRECOMP HD_ERROR	/* same io address, read=error, write=precomp */
#define HD_COMMAND HD_STATUS	/* same io address, read=status, write=cmd */

#define HD_CMD 0x3f6		// Hard disk control port

struct hd_i_struct
{
	int head, sect, cyl, wpcom, lzone, ctl;
};
void inthandler2e(int *esp);
void sendHardDiskCmd(unsigned int drive, unsigned int nsect, unsigned int sect, unsigned int head, unsigned int cyl, unsigned int cmd);
void readHardDisk(unsigned char nsector, unsigned char sector, unsigned int cyl, unsigned char current);
void writeHardDisk(unsigned char nsector, unsigned char sector, unsigned int cyl, unsigned char current, short* buf);

/* file.c */
struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};
void file_readfat(int *fat, unsigned char *img);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);
