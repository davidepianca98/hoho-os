#include <stdio.h>
#include <string.h>
#include <system_calls.h>

int main(int argc, char **argv) {
    char s[64];
    printf("File to edit: ");
    scanf("%s", s);
    
    FILE *f = fopen(s, "r");
    //system("clear");
    
    return 0;
}

