TOOLPATH = tools/
INCPATH  = include/

MAKE     = $(TOOLPATH)make.exe -r
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

default :
	$(MAKE) dingus.img

dingus.img : bootloader/ipl.bin kernel/dingus.kernel Makefile
	$(EDIMG)   imgin:$(INCPATH)fdimg0at.tek \
		wbinimg src:bootloader/ipl.bin len:512 from:0 to:0 \
		copy from:kernel/dingus.kernel to:@: \
		imgout:dingus.img

full :
	$(MAKE) -C bootloader
	$(MAKE) -C kernel
	$(MAKE) dingus.img

run :
	$(MAKE) dingus.img
	$(COPY) dingus.img tools\qemu\fdimage0.bin
	$(MAKE) -C $(TOOLPATH)qemu
