all:
	$(CC) -Wall -g -gstabs -Wextra -fno-builtin -nodefaultlibs -nostartfiles -nostdlib -m32 -I ../../include/lib -c main.c
	$(LD) $(LDFLAGS) -o proc.exc main.o ../../lib/stdio.o ../../lib/string.o

