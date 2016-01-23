#include <stdio.h>
#include <unistd.h>
/*
int main() {
    pid_t pid;
    pid = fork();
    if(pid == 0) {
        printf("Siamo nel figlio\n");
        exit(0);
    } else
        printf("BOH: %x\n", pid);
    return 0;
}*/

int main() {
    pid_t pid;
    pid = fork();
    if(pid == 0) {
        char c = 'a';
        while(1) {
            printf("%c", c++);
            if(c > 'z')
                c = 'a';
        }
    } else {
        char c = '0';
        while(1) {
            printf("%c", c++);
            if(c > '9')
                c = '0';
        }
    }
    return 0;
}

