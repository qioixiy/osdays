TOOLPATH = ../../tools/z_tools/
MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = $(TOOLPATH)copy
DEL      = $(TOOLPATH)del

default:
	$(MAKE) img

ipl.bin : ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst

haribote.sys: haribote.nas Makefile
	$(NASK) haribote.nas haribote.sys haribote.lst

haribote.img : ipl.bin haribote.sys Makefile
	$(EDIMG) imgin:$(TOOLPATH)/fdimg0at.tek \
	wbinimg src:ipl.bin len:512 from:0 to:0 \
	copy from:haribote.sys to:@: \
	imgout:haribote.img

img:
	$(MAKE) haribote.img

run:
	$(MAKE) img
	copy haribote.img ..\..\tools\z_tools\qemu\fdimage0.bin
	$(MAKE) -C $(TOOLPATH)\qemu

clean:
	-rm *.bin *.lst *~ *#

src_only:
	$(MAKE) clean
	-$(DEL) haribote.img
	-$(DEL) haribote.sys
