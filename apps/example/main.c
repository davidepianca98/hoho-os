#include <stdio.h>

int main() {
    int n = 0;
    printf("Inserisci un numero: ");
    scanf("%d", &n);
    printf("Il numero e': %d\n", n);
    
    char c;
    printf("Inserisci un carattere: ");
    scanf("%c", &c);
    printf("Il carattere e': %c\n", c);
    
    char s[64];
    printf("Inserisci una stringa: ");
    scanf("%s", s);
    printf("La stringa e': %s\n", s);
    return 0;
}

