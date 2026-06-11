#include <stdio.h>
#include <string.h>

#define MAX_ABONENTS 100

struct abonent {
    char name[10];
    char second_name[10];
    char tel[10];
};

struct abonent phonebook[MAX_ABONENTS];
int abonent_count = 0;

void init_phonebook() {
    for (int i = 0; i < MAX_ABONENTS; i++) {
        memset(&phonebook[i], 0, sizeof(struct abonent));
    }
}

int is_empty(struct abonent *entry) {
    return (entry->name[0] == 0 &&
            entry->second_name[0] == 0 &&
            entry->tel[0] == 0);
}

// 1) Добавить абонента
void add_abonent() {
    if (abonent_count >= MAX_ABONENTS) {
        printf("Ошибка: справочник переполнен (максимум %d абонентов)!\n", MAX_ABONENTS);
        return;
    }

    for (int i = 0; i < MAX_ABONENTS; i++) {
        if (is_empty(&phonebook[i])) {
            printf("Введите имя: ");
            scanf("%9s", phonebook[i].name);

            printf("Введите фамилию: ");
            scanf("%9s", phonebook[i].second_name);

            printf("Введите телефон: ");
            scanf("%9s", phonebook[i].tel);

            abonent_count++;
            printf("Абонент добавлен (запись %d).\n", i);
            return;
        }
    }

    printf("Не найдено свободного места!\n");
}

// 2) Удалить абонента
void delete_abonent() {
    if (abonent_count == 0) {
        printf("Справочник пуст, нечего удалять.\n");
        return;
    }

    int index;
    printf("Введите номер записи для удаления (0..%d): ", MAX_ABONENTS - 1);
    scanf("%d", &index);

    if (index < 0 || index >= MAX_ABONENTS) {
        printf("Неверный индекс!\n");
        return;
    }

    if (is_empty(&phonebook[index])) {
        printf("Запись %d уже пуста.\n", index);
        return;
    }

    memset(&phonebook[index], 0, sizeof(struct abonent));
    abonent_count--;
    printf("Запись %d удалена (обнулена).\n", index);
}

// 3) Поиск абонентов по имени
void search_by_name() {
    char search_name[10];
    printf("Введите имя для поиска: ");
    scanf("%9s", search_name);

    int found = 0;
    for (int i = 0; i < MAX_ABONENTS; i++) {
        if (!is_empty(&phonebook[i]) &&
            strcmp(phonebook[i].name, search_name) == 0) {
            printf("Запись %d: %s %s, тел: %s\n",
                   i,
                   phonebook[i].name,
                   phonebook[i].second_name,
                   phonebook[i].tel);
            found = 1;
        }
    }

    if (!found) {
        printf("Абонентов с именем '%s' не найдено.\n", search_name);
    }
}

// 4) Вывод всех записей
void print_all() {
    if (abonent_count == 0) {
        printf("Справочник пуст.\n");
        return;
    }

    printf("=== Список всех абонентов ===\n");
    for (int i = 0; i < MAX_ABONENTS; i++) {
        if (!is_empty(&phonebook[i])) {
            printf("%3d: %10s %10s %10s\n",
                   i,
                   phonebook[i].name,
                   phonebook[i].second_name,
                   phonebook[i].tel);
        }
    }
    printf("Всего абонентов: %d\n", abonent_count);
}

int main() {
    int choice;

    init_phonebook();

    do {
        printf("\n=== Абонентский справочник ===\n");
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
                printf("Выход из программы.\n");
                break;
            default:
                printf("Неверный пункт меню. Повторите ввод.\n");
        }
    } while (choice != 5);

    return 0;
}