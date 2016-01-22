#include <stdio.h>
#include <string.h>
#include <system_calls.h>

int main(int argc, char **argv) {
    int i;
    //system("clear");
    printf("argc: %d\n", argc);
    for(i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    printf("addr: %x\n", argv);
    return 0;
}

