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

#crack
crack1.bim: crack1.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:crack1.bim map:crack1.map crack1.obj a_nask.obj
crack1.hrb: crack1.bim Makefile
	$(BIM2HRB) crack1.bim crack1.hrb 0
crack2.hrb: crack2.nas Makefile
	$(NASK) crack2.nas crack2.hrb crack2.lst
crack3.hrb: crack3.nas Makefile
	$(NASK) crack3.nas crack3.hrb crack3.lst
crack4.hrb: crack4.nas Makefile
	$(NASK) crack4.nas crack4.hrb crack4.lst
crack5.hrb: crack5.nas Makefile
	$(NASK) crack5.nas crack5.hrb crack5.lst
crack6.hrb: crack6.nas Makefile
	$(NASK) crack6.nas crack6.hrb crack6.lst
#bug
#bug1.bim: bug1.obj a_nask.obj Makefile
#	$(OBJ2BIM) @$(RULEFILE) out:bug1.bim map:bug1.map bug1.obj a_nask.obj
#bug1.hrb: bug1.bim Makefile
#	$(BIM2HRB) bug1.bim bug1.hrb 0
%.hrb: %.obj a_nask.obj api.h Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$*.bim map:$*.map $*.obj a_nask.obj
	$(BIM2HRB) $*.bim $*.hrb 0

#hrb
hello5.hrb: hello5.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:hello5.bim stack:1k map:hello5.map hello5.obj
	$(BIM2HRB) hello5.bim hello5.hrb 0

#C app end

haribote.img: ipl10.bin haribote.sys hello.hrb hello2.hrb hello3.hrb hello4.hrb hello5.hrb a.hrb Makefile $(OBJS)\
	crack1.hrb crack2.hrb crack3.hrb crack4.hrb crack5.hrb crack6.hrb \
	bug1.hrb bug2.hrb bug3.hrb\
	winhelo.hrb winhelo2.hrb winhelo3.hrb\
	star1.hrb starts.hrb stars2.hrb\
	lines.hrb walk.hrb\
	noodle.hrb
	$(EDIMG) imgin:$(TOOLPATH)/fdimg0at.tek \
	wbinimg src:ipl10.bin len:512 from:0 to:0 \
	copy from:haribote.sys to:@: \
	copy from:ipl10.nas to:@: \
	copy from:make.bat to:@: \
	copy from:hello.hrb to:@: \
	copy from:hello2.hrb to:@: \
	copy from:hello3.hrb to:@: \
	copy from:hello4.hrb to:@: \
	copy from:hello5.hrb to:@: \
	copy from:a.hrb to:@: \
	copy from:crack1.hrb to:@: \
	copy from:crack2.hrb to:@: \
	copy from:crack3.hrb to:@: \
	copy from:crack4.hrb to:@: \
	copy from:crack5.hrb to:@: \
	copy from:crack6.hrb to:@: \
	copy from:bug1.hrb to:@: \
	copy from:bug2.hrb to:@: \
	copy from:bug3.hrb to:@: \
	copy from:winhelo.hrb to:@: \
	copy from:winhelo2.hrb to:@: \
	copy from:winhelo3.hrb to:@: \
	copy from:star1.hrb to:@: \
	copy from:starts.hrb to:@: \
	copy from:stars2.hrb to:@: \
	copy from:lines.hrb to:@: \
	copy from:walk.hrb to:@: \
	copy from:noodle.hrb to:@: \
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
