all:
	$(CC) $(CFLAGS) cpu.c
	$(CC) $(CFLAGS) dma.c
	$(CC) $(CFLAGS) pic.c
	$(CC) $(CFLAGS) pit.c
	$(AS) $(ASFLAGS) pit_asm.o pit.asm
	$(AS) $(ASFLAGS) v8086.o v8086.asm
