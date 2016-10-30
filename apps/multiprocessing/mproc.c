#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid;
    pid = fork();
    if(pid == 0) {
        printf("Siamo nel figlio\n");
        exit(0);
    } else {
        printf("Siamo nel padre: %x\n", pid);
        while(1);
    }
    return 0;
}
/*
int main() {
    pid_t pid;
    int sema = 0;
    pid = fork();
    if(pid == 0) {
        char c = 'a';
        while(1) {
            if(!sema) {
                sema = 1;
                printf("%c", c++);
                if(c > 'z')
                    c = 'a';
                sema = 0;
            }
        }
    } else {
        if(pid < 0)
            return 1;
        char c = '0';
        while(1) {
            if(!sema) {
                sema = 1;
                printf("%c", c++);
                if(c > '9')
                    c = '0';
                sema = 0;
            }
        }
    }
    return 0;
}*/

