#include <stdio.h>

int main() {
    unsigned int num;
    int count = 0;
    
    printf("Введите целое положительное число: ");
    scanf("%u", &num);
    
    for (int i = 0; i < 32; i++) {
        if ((num >> i) & 1) {
            count++;
        }
    }
    
    printf("Количество единиц в двоичном представлении: %d\n", count);
    
    return 0;
}