#include <stdio.h>

char* find_substring(const char *str, const char *sub) {
    if (*sub == '\0') return (char*)str;
    
    for (const char *s = str; *s != '\0'; s++) {
        const char *s_start = s;
        const char *sub_start = sub;

        while (*s_start != '\0' && *sub_start != '\0' && *s_start == *sub_start) {
            s_start++;
            sub_start++;
        }

        if (*sub_start == '\0') {
            return (char*)s;
        }
    }
    
    return NULL;
}

int main() {
    char str[500];
    char sub[100];
    
    printf("Введите строку: ");
    fgets(str, sizeof(str), stdin);
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
    
    printf("Введите подстроку для поиска: ");
    fgets(sub, sizeof(sub), stdin);
    for (int i = 0; sub[i] != '\0'; i++) {
        if (sub[i] == '\n') {
            sub[i] = '\0';
            break;
        }
    }
    
    char *result = find_substring(str, sub);
    
    if (result != NULL) {
        printf("Подстрока найдена! Указатель на начало: %p\n", result);
        printf("Начиная с: %s\n", result);
    } else {
        printf("Подстрока не найдена (NULL)\n");
    }
    
    return 0;
}