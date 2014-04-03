#include "kernel.h"

struct hd_i_struct hd_info[] =
{
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0}
};
// 判断并循环等待驱动器就绪。
// 读硬盘控制器状态寄存器端口HD_STATUS(0x1f7)，并循环检测驱动器就绪比特位和控制器忙位。
static int controller_ready (void)
{
	int retries = 10000;

	while (--retries && (io_in8(0x1f7) & 0xc0) != 0x40);
		return (retries);		// 返回等待循环的次数。
}

void sendHardDiskCmd(unsigned int drive, unsigned int nsect, unsigned int sect, unsigned int head, unsigned int cyl, unsigned int cmd){
	register int port; //asm ("dx");	// port 变量对应寄存器dx。

	if (drive > 1 || head > 15)	// 如果驱动器号(0,1)>1 或磁头号>15，则程序不支持。
		panic_s ("Trying to write bad sector");
	if (!controller_ready ())	// 如果等待一段时间后仍未就绪则出错，死机。
		panic_s ("HD controller not ready");

//	do_hd = intr_addr;		// do_hd 函数指针将在硬盘中断程序中被调用。
	int	i;
	i = controller_ready();
	if(i == 0){
		panic_s("BUSY");
		return;
	}
	io_out8 (hd_info[drive].ctl, HD_CMD);	// 向控制寄存器(0x3f6)输出控制字节。
	port = HD_DATA;		// 置dx 为数据寄存器端口(0x1f0)。
	io_out8 (++port, hd_info[drive].wpcom >> 2);	// 参数：写预补偿柱面号(需除4)。
	io_out8 (++port, nsect);	// 参数：读/写扇区总数。
	io_out8 (++port, sect);	// 参数：起始扇区。
	io_out8 (++port, cyl);		// 参数：柱面号低8 位。
	io_out8 (++port, cyl >> 8);	// 参数：柱面号高8 位。
	io_out8 (++port, 0xA0 | (drive << 4) | head);	// 参数：驱动器号+磁头号。
	io_out8 (++port, cmd);		// 命令：硬盘控制命令。
}

void readHardDisk(unsigned char nsector, unsigned char sector, unsigned int cyl, unsigned char current){
	struct BOOTINFO	*binfo = (struct BOOTINFO	*) ADR_BOOTINFO;
	int	i;
	i = controller_ready();
	if(i == 0){
		putfonts8_asc(binfo->vram, binfo->scrnx, 400,	400, COL8_00FF00,	"Read Busy");
		putHex(binfo->vram, binfo->scrnx, 0,	0, COL8_00FF00,	(unsigned	char *)0x1f7);
		return;
	}
	io_out8(0x3f6, 0x00);
	io_out8(0x1f2, nsector);
	io_out8(0x1f3, sector);
	io_out8(0x1f4, cyl);
	io_out8(0x1f5, cyl>>8);
	io_out8(0x1f6, current | 0xa0);
	io_out8(0x1f7, 0x20);
}

void writeHardDisk(unsigned char nsector, unsigned char sector, unsigned int cyl, unsigned char current, short* buf){
	struct BOOTINFO	*binfo = (struct BOOTINFO	*) ADR_BOOTINFO;
	int	i;
	i = controller_ready();
	if(i == 0){
		putfonts8_asc(binfo->vram, binfo->scrnx, 400,	400, COL8_00FF00,	"Busy");
		return;
	}
	io_out8(0x1f2, nsector);
	io_out8(0x1f3, sector);
	io_out8(0x1f4, cyl);
	io_out8(0x1f5, cyl>>8);
	io_out8(0x1f6, current | 0xa0);
	io_out8(0x1f7, 0x30);
	i = controller_ready();
	if(i == 0){
		putfonts8_asc(binfo->vram, binfo->scrnx, 400,	400, COL8_00FF00,	"Busy in writing");
		return;
	}
	for(i	=	0; i < 256;	i++){
		io_out16(0x1f0, buf[i]);
	}
}

void inthandler2e(int	*esp){
	struct BOOTINFO	*binfo = (struct BOOTINFO	*) ADR_BOOTINFO;
	int	i;
	short	hd_data[256];
	for(i	=	0; i < 256;	i++){
		hd_data[i] = io_in16(0x1f0);
	}
	//*((unsigned char *)hd_data) = io_in8(0x1f7);
	putHexs(binfo->vram, binfo->scrnx, 0,	0, COL8_00FF00,	(unsigned	char *)hd_data,	512);
	io_out8(PIC1_OCW2, 0x66);
	io_out8(PIC0_OCW2, 0x62);
	return;
}
