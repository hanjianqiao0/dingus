OBJS_BOOTPACK = bootpack.obj ascii.obj fifo.obj graphic.obj dsctbl.obj \
				eint.obj keyboard.obj mouse.obj clock.obj init.obj \
				naskfunc.obj sys.obj memory.obj task.obj FAT32.obj \
				harddisk.obj

TOOLPATH = ../tools/
INCPATH  = ../include/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(INCPATH)dingus.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
GOLIB    = $(TOOLPATH)golib00.exe 
COPY     = copy
DEL      = del

default :
	$(MAKE) dingus.kernel

setup.bin : setup.nas Makefile
	$(NASK) setup.nas setup.bin setup.lst

ascii.bin : ascii.txt Makefile
	$(MAKEFONT) ascii.txt ascii.bin

ascii.obj : ascii.bin Makefile
	$(BIN2OBJ) ascii.bin ascii.obj _ascii

bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

dingus.kernel : setup.bin bootpack.hrb Makefile
	copy /B setup.bin+bootpack.hrb dingus.kernel

%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) *.hrb
	-$(DEL) *.bak

src_only :
	$(MAKE) clean
	-$(DEL) dingus.kernel
