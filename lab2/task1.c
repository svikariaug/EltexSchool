#include <stdio.h>

int main() {
    int N;
    printf("Введите N: ");
    scanf("%d", &N);
    
    int num = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", num++);
        }
        printf("\n");
    }
    
    return 0;
}