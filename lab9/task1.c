#include <stdio.h>

int main() {
    FILE *file = fopen("output.txt", "w");
    fprintf(file, "String from file");
    fclose(file);

    file = fopen("output.txt", "r");
    fseek(file, 0, SEEK_END);
    long pos = ftell(file);
    char ch;

    printf("Строка из файла (с конца): ");
    while (pos > 0) {
        pos--;
        fseek(file, pos, SEEK_SET);
        ch = fgetc(file);
        putchar(ch);
    }
    printf("\n");
    fclose(file);
    
    return 0;
}