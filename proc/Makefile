all:
	$(CC) $(CFLAGS) elf.c
	$(AS) $(ASFLAGS) end_process.o end_process.asm
	$(CC) $(CFLAGS) proc.c
	$(CC) $(CFLAGS) sched.c
	$(CC) $(CFLAGS) thread.c

