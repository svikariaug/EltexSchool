#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct abonent {
    char name[10];
    char second_name[10];
    char tel[10];
};

struct Node {
    struct abonent data;
    struct Node *prev;
    struct Node *next;
};

struct Node *head = NULL;
struct Node *tail = NULL;
int abonent_count = 0;

// 1) Добавить абонента
void add_abonent() {
    struct Node *new_node = (struct Node*)malloc(sizeof(struct Node));
    if (new_node == NULL) {
        printf("Ошибка: не удалось выделить память!\n");
        return;
    }

    printf("Введите имя: ");
    scanf("%9s", new_node->data.name);

    printf("Введите фамилию: ");
    scanf("%9s", new_node->data.second_name);

    printf("Введите телефон: ");
    scanf("%9s", new_node->data.tel);

    new_node->prev = NULL;
    new_node->next = NULL;

    // Добавление в конец списка
    if (head == NULL) {
        head = new_node;
        tail = new_node;
    } else {
        tail->next = new_node;
        new_node->prev = tail;
        tail = new_node;
    }

    abonent_count++;
    printf("Абонент добавлен. Всего абонентов: %d\n", abonent_count);
}

// 2) Удалить абонента (по номеру в списке)
void delete_abonent() {
    if (head == NULL) {
        printf("Справочник пуст, нечего удалять.\n");
        return;
    }

    printf("=== Список абонентов ===\n");
    struct Node *current = head;
    int index = 0;
    while (current != NULL) {
        printf("%d: %s %s, тел: %s\n", index,
               current->data.name,
               current->data.second_name,
               current->data.tel);
        current = current->next;
        index++;
    }

    int del_index;
    printf("Введите номер абонента для удаления (0..%d): ", abonent_count - 1);
    scanf("%d", &del_index);

    if (del_index < 0 || del_index >= abonent_count) {
        printf("Неверный номер!\n");
        return;
    }

    // Поиск узла по индексу
    current = head;
    for (int i = 0; i < del_index; i++) {
        current = current->next;
    }

    // Удаление узла
    if (current->prev != NULL) {
        current->prev->next = current->next;
    } else {
        head = current->next;
    }

    if (current->next != NULL) {
        current->next->prev = current->prev;
    } else {
        tail = current->prev;
    }

    free(current);
    abonent_count--;
    printf("Абонент удалён. Осталось абонентов: %d\n", abonent_count);
}

// 3) Поиск абонентов по имени
void search_by_name() {
    char search_name[10];
    printf("Введите имя для поиска: ");
    scanf("%9s", search_name);

    struct Node *current = head;
    int found = 0;
    int index = 0;

    while (current != NULL) {
        if (strcmp(current->data.name, search_name) == 0) {
            printf("%d: %s %s, тел: %s\n", index,
                   current->data.name,
                   current->data.second_name,
                   current->data.tel);
            found = 1;
        }
        current = current->next;
        index++;
    }

    if (!found) {
        printf("Абонентов с именем '%s' не найдено.\n", search_name);
    }
}

// 4) Вывод всех записей
void print_all() {
    if (head == NULL) {
        printf("Справочник пуст.\n");
        return;
    }

    printf("=== Список всех абонентов ===\n");
    struct Node *current = head;
    int index = 0;
    while (current != NULL) {
        printf("%3d: %10s %10s %10s\n", index,
               current->data.name,
               current->data.second_name,
               current->data.tel);
        current = current->next;
        index++;
    }
    printf("Всего абонентов: %d\n", abonent_count);
}

// 5) Освобождение всей памяти при завершении
void free_all() {
    struct Node *current = head;
    while (current != NULL) {
        struct Node *temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL;
    tail = NULL;
    abonent_count = 0;
    printf("Вся память освобождена.\n");
}

int main() {
    int choice;

    do {
        printf("\n=== Абонентский справочник (двусвязный список) ===\n");
        printf("1) Добавить абонента\n");
        printf("2) Удалить абонента\n");
        printf("3) Поиск абонентов по имени\n");
        printf("4) Вывод всех записей\n");
        printf("5) Выход\n");
        printf("Ваш выбор: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                add_abonent();
                break;
            case 2:
                delete_abonent();
                break;
            case 3:
                search_by_name();
                break;
            case 4:
                print_all();
                break;
            case 5:
                free_all();
                printf("Выход из программы.\n");
                break;
            default:
                printf("Неверный пункт меню. Повторите ввод.\n");
        }
    } while (choice != 5);

    return 0;
}