#include <stdio.h>

int main() {
    int num;
    unsigned char new_byte;
    
    printf("Введите целое положительное число: ");
    scanf("%d", &num);
    
    printf("Введите новое значение для третьего байта (0-255): ");
    scanf("%hhu", &new_byte);
    
    printf("Исходное число: %d (0x%X)\n", num, num);
    
    unsigned char *ptr = (unsigned char*)&num;
    
    ptr[2] = new_byte;
    
    printf("Изменённое число: %d (0x%X)\n", num, num);
    
    return 0;
}