#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    if(argc < 2)
        return 1;
    
    printf("Welcome\n");
    FILE *f = fopen(argv[1], "r");
    //system("clear");
    fclose(f);
    
    return 0;
}
