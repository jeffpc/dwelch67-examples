include ../config.make

COPS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding 

all : notmain.bin

clean :
	rm -f *.o
	rm -f *.bin
	rm -f *.elf
	rm -f *.hex
	rm -f *.list

vectors.o : vectors.s
	$(AS) -march=armv6k -mfpu=vfpv2 -mfloat-abi=hard vectors.s -o vectors.o

notmain.o : notmain.c 
	$(CC) $(COPS) -c notmain.c -o notmain.o

uart.o : uart.c 
	$(CC) $(COPS) -c uart.c -o uart.o

notmain.bin : mapfile vectors.o uart.o notmain.o 
	$(LD) -dy -b -znointerp -o notmain.elf -e _start -M mapfile vectors.o notmain.o uart.o
	#$(OBJDUMP) -D notmain.elf > notmain.list
	$(OBJCOPY) notmain.elf -O ihex notmain.hex 
	$(OBJCOPY) notmain.elf -O binary notmain.bin 
