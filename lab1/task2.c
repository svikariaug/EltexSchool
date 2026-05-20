#include <stdio.h>

void printBinarySigned(int n) {\
    for (int i = 31; i >= 0; i--) {
        printf("%d", (n >> i) & 1);
        if (i % 8 == 0 && i != 0) printf(" ");
    }
    printf("\n");
}

int main() {
    int num;
    
    printf("Введите целое отрицательное число: ");
    scanf("%d", &num);
    
    printf("Двоичное представление числа %d:\n", num);
    printBinarySigned(num);
    
    return 0;
}