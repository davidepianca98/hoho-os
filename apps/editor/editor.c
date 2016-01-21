#include <stdio.h>
#include <string.h>
#include <system_calls.h>

int main(int argc, char **argv) {
    system("clear");
    printf("argc: %d\n", argc);
    printf("argv: %s\n", argv[0]);
    return 0;
}

