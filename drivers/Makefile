all:
	cd ata; make
	cd floppy; make
	cd keyboard; make
	cd video; make
	cd x86; make
	$(CC) $(CFLAGS) audio.c
	$(CC) $(CFLAGS) io.c

