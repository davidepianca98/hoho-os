all:
	$(CC) -Wall -g -gstabs -Wextra -fno-builtin -nodefaultlibs -nostartfiles -nostdlib -m32 -I ../../include/lib -c mproc.c
	$(LD) $(LDFLAGS) -o mproc mproc.o ../../lib/stdio.o ../../lib/string.o ../../lib/unistd.o ../../lib/system_calls.o ../../lib/stdlib.o

