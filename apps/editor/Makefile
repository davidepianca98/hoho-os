all:
	$(CC) -Wall -g -gstabs -Wextra -fno-builtin -nodefaultlibs -nostartfiles -nostdlib -m32 -I ../../include/lib -c editor.c
	$(LD) $(LDFLAGS) -o editor editor.o ../../lib/stdio.o ../../lib/string.o ../../lib/system_calls.o ../../lib/stdlib.o

