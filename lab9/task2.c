#include <ncurses.h>
#include <locale.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_ENTRIES 1000
#define MAX_PATH 1024

typedef struct {
    char name[MAX_PATH];
    int is_dir;
} FileEntry;

FileEntry entries_left[MAX_ENTRIES];
FileEntry entries_right[MAX_ENTRIES];
int count_left = 0, count_right = 0;
int selected_left = 0, selected_right = 0;
int active_panel = 0;
char path_left[MAX_PATH] = ".";
char path_right[MAX_PATH] = ".";

void load_directory(char *path, FileEntry *entries, int *count) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char fullpath[MAX_PATH + 10];

    *count = 0;
    dir = opendir(path);
    if (dir == NULL) return;

    strcpy(entries[*count].name, "..");
    entries[*count].is_dir = 1;
    (*count)++;

    while ((entry = readdir(dir)) != NULL && *count < MAX_ENTRIES) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        if (stat(fullpath, &st) == 0) {
            strncpy(entries[*count].name, entry->d_name, MAX_PATH - 1);
            entries[*count].name[MAX_PATH - 1] = '\0';
            entries[*count].is_dir = S_ISDIR(st.st_mode);
            (*count)++;
        }
    }
    closedir(dir);
}

void draw_panel(int x, int y, int width, int height, FileEntry *entries, int count, int selected, char *path, int is_active) {
    int i;

    if (is_active) attron(A_REVERSE);
    for (i = 0; i < width; i++) mvaddch(y, x + i, ' ');
    mvprintw(y, x + 2, " %s ", path);
    if (is_active) attroff(A_REVERSE);

    mvhline(y + 1, x, ACS_HLINE, width);
    mvaddch(y + 1, x, ACS_LTEE);
    mvaddch(y + 1, x + width - 1, ACS_RTEE);

    for (i = 0; i < height - 3 && i < count; i++) {
        if (i == selected) attron(A_REVERSE);
        if (entries[i].is_dir) attron(A_BOLD);
        
        mvprintw(y + 2 + i, x + 2, "%-*s", width - 4, entries[i].name);
        
        if (entries[i].is_dir) attroff(A_BOLD);
        if (i == selected) attroff(A_REVERSE);
    }

    mvhline(y + height - 1, x, ACS_HLINE, width);
    mvaddch(y + height - 1, x, ACS_LLCORNER);
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);
}

void change_directory(char *path, FileEntry *entries, int *count, int *selected) {
    char newpath[MAX_PATH * 2];
    
    if (*count > 0 && *selected >= 0 && *selected < *count) {
        if (strcmp(entries[*selected].name, "..") == 0) {
            char *last_slash = strrchr(path, '/');
            if (last_slash != NULL) {
                if (last_slash == path) {
                    strcpy(path, "/");
                } else {
                    *last_slash = '\0';
                }
            } else {
                if (strcmp(path, ".") == 0) {
                    char cwd[MAX_PATH];
                    if (getcwd(cwd, sizeof(cwd))) {
                        char *parent = strrchr(cwd, '/');
                        if (parent && parent != cwd) {
                            *parent = '\0';
                            strcpy(path, cwd);
                        } else if (parent == cwd) {
                            strcpy(path, "/");
                        } else {
                            strcpy(path, "..");
                        }
                    }
                } else {
                    char *slash = strrchr(path, '/');
                    if (slash) {
                        *slash = '\0';
                    } else {
                        strcpy(path, ".");
                    }
                }
            }
            load_directory(path, entries, count);
            *selected = 0;
            return;
        }
        
        if (entries[*selected].is_dir) {
            if (strcmp(path, ".") == 0) {
                snprintf(newpath, sizeof(newpath), "%s", entries[*selected].name);
            } else if (strcmp(path, "/") == 0) {
                snprintf(newpath, sizeof(newpath), "/%s", entries[*selected].name);
            } else {
                snprintf(newpath, sizeof(newpath), "%s/%s", path, entries[*selected].name);
            }
            newpath[sizeof(newpath) - 1] = '\0';
            strncpy(path, newpath, MAX_PATH - 1);
            path[MAX_PATH - 1] = '\0';
            load_directory(path, entries, count);
            *selected = 0;
        }
    }
}

int main() {
    int height, width, ch;

    setlocale(LC_ALL, "");
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    use_default_colors();

    getmaxyx(stdscr, height, width);
    int panel_width = width / 2 - 2;
    int panel_height = height - 4;

    load_directory(path_left, entries_left, &count_left);
    load_directory(path_right, entries_right, &count_right);

    while (1) {
        clear();
        getmaxyx(stdscr, height, width);
        panel_width = width / 2 - 2;
        panel_height = height - 4;

        draw_panel(2, 1, panel_width, panel_height, entries_left, count_left, selected_left, path_left, active_panel == 0);
        draw_panel(width / 2 + 2, 1, panel_width, panel_height, entries_right, count_right, selected_right, path_right, active_panel == 1);

        mvprintw(height - 2, 2, "Tab - switch panel | Enter - open folder / go up | Q - quit");
        refresh();

        ch = getch();
        switch (ch) {
            case KEY_UP:
                if (active_panel == 0 && selected_left > 0) selected_left--;
                else if (active_panel == 1 && selected_right > 0) selected_right--;
                break;
            case KEY_DOWN:
                if (active_panel == 0 && selected_left < count_left - 1) selected_left++;
                else if (active_panel == 1 && selected_right < count_right - 1) selected_right++;
                break;
            case '\n':
            case KEY_ENTER:
                if (active_panel == 0) {
                    change_directory(path_left, entries_left, &count_left, &selected_left);
                } else {
                    change_directory(path_right, entries_right, &count_right, &selected_right);
                }
                break;
            case '\t':
                active_panel = !active_panel;
                break;
            case 'q':
            case 'Q':
                endwin();
                return 0;
        }
    }
    endwin();
    return 0;
}