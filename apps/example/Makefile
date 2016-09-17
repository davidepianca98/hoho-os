all:
	$(CC) -Wall -g -gstabs -Wextra -fno-builtin -nodefaultlibs -nostartfiles -nostdlib -m32 -I ../../include/lib -c main.c
	$(LD) $(LDFLAGS) -o proc main.o ../../lib/stdio.o ../../lib/string.o ../../lib/system_calls.o
