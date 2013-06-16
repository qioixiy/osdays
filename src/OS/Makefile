TOOLPATH = ../../tools/z_tools/
MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = $(TOOLPATH)copy
DEL      = $(TOOLPATH)del

default:
	$(MAKE) img
img: 
	$(MAKE) -r helloos.img
ipl.bin : ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst

helloos.img : ipl.bin Makefile
	$(EDIMG) imgin:$(TOOLPATH)/fdimg0at.tek \
	wbinimg src:ipl.bin len:512 from:0 to:0   imgout:helloos.img
asm:
	$(MAKE) -r ipl.bin

run:
	$(MAKE) img
	copy helloos.img ..\..\tools\z_tools\qemu\fdimage0.bin
	$(MAKE) -C $(TOOLPATH)\qemu

clean:
	-rm *.bin *.lst *.img

src_only:
	$(MAKE) clean
	-$(DEL) haribote.img
