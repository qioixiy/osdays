TOOLPATH = ../../tools/z_tools/
INCPATH	 = ../../tools/z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe

CC1	 = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul

EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = $(TOOLPATH)copy
DEL      = $(TOOLPATH)del

OBJS	 = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj int.obj fifo.obj \
	mouse.obj keyboard.obj memory.obj sheet.obj window.obj timer.obj mtask.obj fat.obj console.obj


default:
	$(MAKE) img

ipl10.bin : ipl10.nas Makefile
	@$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin: asmhead.nas Makefile
	@$(NASK) asmhead.nas asmhead.bin asmhead.lst

%.gas: %.c *.h Makefile
	@$(CC1) -o $*.gas $*.c
%.nas: %.gas Makefile
	@$(GAS2NASK) $*.gas -o $*.nas 
%.obj: %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

naskfunc.obj: naskfunc.nas Makefile
	@$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

hankaku.bin: hankaku.txt Makefile
	@$(MAKEFONT) hankaku.txt hankaku.bin
hankaku.obj: hankaku.bin Makefile
	@$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim: $(OBJS) Makefile
	@$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map $(OBJS)

#3MB+64KB=3136KB
bootpack.hrb: bootpack.bim Makefile 
	@$(BIM2HRB) bootpack.bim bootpack.hrb 0

haribote.sys: asmhead.bin bootpack.hrb Makefile $(OBJS)
	@copy /B asmhead.bin+bootpack.hrb haribote.sys

#Appliction
hello.hrb: hello.nas Makefile
	$(NASK) hello.nas hello.hrb hello.lst
hello2.hrb: hello2.nas Makefile
	$(NASK) hello2.nas hello2.hrb hello2.lst

#C app start
a.bim: a.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:a.bim map:a.map a.obj a_nask.obj
a.hrb: a.bim Makefile
	$(BIM2HRB) a.bim a.hrb 0

hello3.bim: hello3.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:hello3.bim map:hello3.map hello3.obj a_nask.obj
hello3.hrb: hello3.bim Makefile
	$(BIM2HRB) hello3.bim hello3.hrb 0
#C app end

haribote.img : ipl10.bin haribote.sys hello.hrb hello2.hrb hello3.hrb a.hrb Makefile $(OBJS)
	$(EDIMG) imgin:$(TOOLPATH)/fdimg0at.tek \
	wbinimg src:ipl10.bin len:512 from:0 to:0 \
	copy from:haribote.sys to:@: \
	copy from:ipl10.nas to:@: \
	copy from:make.bat to:@: \
	copy from:hello.hrb to:@: \
	copy from:hello2.hrb to:@: \
	copy from:hello3.hrb to:@: \
	copy from:a.hrb to:@: \
	imgout:haribote.img

img:
	@$(MAKE) haribote.img

run:
	$(MAKE) img
	copy haribote.img ..\..\tools\z_tools\qemu\fdimage0.bin
	$(MAKE) -C $(TOOLPATH)\qemu

clean:
	-rm  *.gas *.obj *.hrb *.bim *.map *.bin *.lst *~ *# *.img *.sys
	-rm bootpack.nas

cls src_only:
	$(MAKE) clean
	-$(DEL) haribote.img
	-$(DEL) haribote.sys
