#include <stdio.h>

void printBinary(unsigned int n) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (n >> i) & 1);
        if (i % 8 == 0 && i != 0) printf(" ");
    }
    printf("\n");
}

int main() {
    unsigned int num;
    
    printf("Введите целое положительное число: ");
    scanf("%u", &num);
    
    printf("Двоичное представление числа %u:\n", num);
    printBinary(num);
    
    return 0;
}