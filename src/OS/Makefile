img: 
	../../tools/z_tools/make.exe -r helloos.img
ipl.bin : ipl.nas Makefile
	../../tools/z_tools/nask.exe ipl.nas ipl.bin ipl.lst

helloos.img : ipl.bin Makefile
	../../tools/z_tools/edimg.exe imgin:../../tools/z_tools/fdimg0at.tek \
	wbinimg src:ipl.bin len:512 from:0 to:0   imgout:helloos.img
asm:
	../../tools/z_tools/make.exe -r ipl.bin

run:
	../../tools/z_tools/make.exe img
	copy helloos.img ..\..\tools\z_tools\qemu\fdimage0.bin
	..\..\tools\z_tools\make.exe -C ..\..\tools\z_tools\qemu
