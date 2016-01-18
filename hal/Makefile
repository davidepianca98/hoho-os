all:
	$(CC) $(CFLAGS) device.c
	$(CC) $(CFLAGS) exception.c
	$(AS) $(ASFLAGS) exception_asm.o exception.asm
	$(CC) $(CFLAGS) gdt.c
	$(AS) $(ASFLAGS) gdt_asm.o gdt.asm
	$(CC) $(CFLAGS) hal.c
	$(CC) $(CFLAGS) idt.c
	$(AS) $(ASFLAGS) idt_asm.o idt.asm
	$(CC) $(CFLAGS) panic.c
	$(CC) $(CFLAGS) syscall.c
	$(CC) $(CFLAGS) usermode.c

