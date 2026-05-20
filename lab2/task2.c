#include <stdio.h>

int main() {
    int N;
    printf("Введите N: ");
    scanf("%d", &N);
    
    int arr[N];
    printf("Введите %d элементов: ", N);
    for (int i = 0; i < N; i++) {
        scanf("%d", &arr[i]);
    }
    
    printf("В обратном порядке: ");
    for (int i = N - 1; i >= 0; i--) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    
    return 0;
}