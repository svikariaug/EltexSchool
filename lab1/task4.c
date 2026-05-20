#include <stdio.h>

unsigned int replaceThirdByte(unsigned int num, unsigned char newByte) {
    num = num & ~(0xFF << 16);
    
    num = num | (newByte << 16);
    
    return num;
}

void printBinaryWithByteMarkers(unsigned int n) {
    printf("Двоичное представление (байты разделены пробелами, *третий байт*):\n");
    
    for (int i = 31; i >= 0; i--) {
        if (i == 23) printf("*");
        if (i == 15) printf(" ");
        
        printf("%d", (n >> i) & 1);
        
        if (i % 8 == 0 && i != 0) {
            if (i == 16) printf("*");
            printf(" ");
        }
    }
    printf("\n");
}

int main() {
    unsigned int num;
    unsigned char newByte;
    
    printf("Введите целое положительное число: ");
    scanf("%u", &num);
    
    printf("Введите новое значение для третьего байта (0-255): ");
    scanf("%hhu", &newByte);
    
    printf("\nИсходное число: %u\n", num);
    printf("Исходное число (HEX): 0x%08X\n", num);
    printBinaryWithByteMarkers(num);
    
    unsigned int result = replaceThirdByte(num, newByte);
    
    printf("\nРезультат: %u\n", result);
    printf("Результат (HEX): 0x%08X\n", result);
    printBinaryWithByteMarkers(result);
    
    return 0;
}