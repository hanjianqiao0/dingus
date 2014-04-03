#include "kernel.h"

struct hd_i_struct hd_info[] =
{
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0}
};
// �жϲ�ѭ���ȴ�������������
// ��Ӳ�̿�����״̬�Ĵ����˿�HD_STATUS(0x1f7)����ѭ�������������������λ�Ϳ�����æλ��
static int controller_ready (void)
{
	int retries = 10000;

	while (--retries && (io_in8(0x1f7) & 0xc0) != 0x40);
		return (retries);		// ���صȴ�ѭ���Ĵ�����
}

void sendHardDiskCmd(unsigned int drive, unsigned int nsect, unsigned int sect, unsigned int head, unsigned int cyl, unsigned int cmd){
	register int port; //asm ("dx");	// port ������Ӧ�Ĵ���dx��

	if (drive > 1 || head > 15)	// �����������(0,1)>1 ���ͷ��>15�������֧�֡�
		panic_s ("Trying to write bad sector");
	if (!controller_ready ())	// ����ȴ�һ��ʱ�����δ���������������
		panic_s ("HD controller not ready");

//	do_hd = intr_addr;		// do_hd ����ָ�뽫��Ӳ���жϳ����б����á�
	int	i;
	i = controller_ready();
	if(i == 0){
		panic_s("BUSY");
		return;
	}
	io_out8 (hd_info[drive].ctl, HD_CMD);	// ����ƼĴ���(0x3f6)��������ֽڡ�
	port = HD_DATA;		// ��dx Ϊ���ݼĴ����˿�(0x1f0)��
	io_out8 (++port, hd_info[drive].wpcom >> 2);	// ������дԤ���������(���4)��
	io_out8 (++port, nsect);	// ��������/д����������
	io_out8 (++port, sect);	// ��������ʼ������
	io_out8 (++port, cyl);		// ����������ŵ�8 λ��
	io_out8 (++port, cyl >> 8);	// ����������Ÿ�8 λ��
	io_out8 (++port, 0xA0 | (drive << 4) | head);	// ��������������+��ͷ�š�
	io_out8 (++port, cmd);		// ���Ӳ�̿������
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
