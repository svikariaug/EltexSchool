#include <stdio.h>
#include "calc.h"

int main() {
    int choice, a, b, result;

    while (1) {
        printf("\nКАЛЬКУЛЯТОР\n");
        printf("1) Сложение\n");
        printf("2) Вычитание\n");
        printf("3) Умножение\n");
        printf("4) Деление\n");
        printf("5) Выход\n");
        printf("Выберите пункт: ");
        scanf("%d", &choice);

        if (choice == 5) {
            printf("Выход из программы.\n");
            break;
        }

        if (choice < 1 || choice > 5) {
            printf("Ошибка: неверный пункт меню.\n");
            continue;
        }

        printf("Введите два целых числа: ");
        scanf("%d %d", &a, &b);

        switch (choice) {
            case 1:
                result = add(a, b);
                printf("Результат: %d + %d = %d\n", a, b, result);
                break;
            case 2:
                result = sub(a, b);
                printf("Результат: %d - %d = %d\n", a, b, result);
                break;
            case 3:
                result = mul(a, b);
                printf("Результат: %d * %d = %d\n", a, b, result);
                break;
            case 4:
                if (b == 0) {
                    printf("Ошибка: деление на ноль!\n");
                } else {
                    result = div(a, b);
                    printf("Результат: %d / %d = %d\n", a, b, result);
                }
                break;
        }
    }

    return 0;
}