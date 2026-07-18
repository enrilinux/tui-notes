#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define NOTES_DIR ".notes"
#define MAX_NOTES 100
#define MAX_TITLE_LEN 256
#define MAX_PATH_LEN 512
#define APP_NAME "tui-notes"

char *note_titles[MAX_NOTES];
int note_count = 0;
char notes_path[MAX_PATH_LEN];

void get_notes_dir(char *buf, size_t size) {
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(buf, size, "%s/%s", home, NOTES_DIR);
}

void get_note_path(char *buf, size_t size, const char *title) {
    snprintf(buf, size, "%s/%s.txt", notes_path, title);
}

void load_notes() {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char fullpath[MAX_PATH_LEN];

    for (int i = 0; i < note_count; i++) {
        free(note_titles[i]);
    }
    note_count = 0;

    dir = opendir(notes_path);
    if (!dir) {
        if (mkdir(notes_path, 0700) == 0) {
            dir = opendir(notes_path);
            if (!dir) return;
        } else {
            return;
        }
    }

    while ((entry = readdir(dir)) != NULL && note_count < MAX_NOTES) {
        if (entry->d_type == DT_REG) {
            snprintf(fullpath, sizeof(fullpath), "%s/%s", notes_path, entry->d_name);
            if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode)) {
                char *dot = strrchr(entry->d_name, '.');
                if (dot && strcmp(dot, ".txt") == 0) {
                    char *title = strdup(entry->d_name);
                    if (title) {
                        char *ext = strrchr(title, '.');
                        if (ext) *ext = '\0';
                        note_titles[note_count++] = title;
                    }
                }
            }
        }
    }
    closedir(dir);
}

void edit_note(const char *title) {
    char filepath[MAX_PATH_LEN];
    get_note_path(filepath, sizeof(filepath), title);

    const char *editor = getenv("EDITOR");
    if (!editor || strlen(editor) == 0) editor = "nano";

    char command[MAX_PATH_LEN + 20];
    snprintf(command, sizeof(command), "%s \"%s\"", editor, filepath);

    endwin();
    system(command);
    refresh();
}

void new_note() {
    char title[MAX_TITLE_LEN];
    char filepath[MAX_PATH_LEN];

    WINDOW *win = newwin(5, 60, (LINES - 5) / 2, (COLS - 60) / 2);
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "Enter note title:");
    mvwprintw(win, 2, 2, "> ");
    wmove(win, 2, 4);
    wrefresh(win);

    echo();
    curs_set(1);
    wgetnstr(win, title, MAX_TITLE_LEN - 1);
    curs_set(0);
    noecho();
    delwin(win);

    if (strlen(title) == 0) return;

    char san_title[MAX_TITLE_LEN];
    strcpy(san_title, title);
    for (char *p = san_title; *p; p++) {
        if (*p == ' ') *p = '_';
    }

    get_note_path(filepath, sizeof(filepath), san_title);

    if (access(filepath, F_OK) == 0) {
        WINDOW *warn = newwin(5, 50, (LINES - 5) / 2, (COLS - 50) / 2);
        box(warn, 0, 0);
        mvwprintw(warn, 1, 2, "A note with this title already exists.");
        mvwprintw(warn, 2, 2, "Press 's' to overwrite, any other to cancel.");
        wrefresh(warn);
        int ch = wgetch(warn);
        delwin(warn);
        if (ch != 's' && ch != 'S') return;
    }

    edit_note(san_title);
    load_notes();
}

void delete_note(const char *title) {
    char filepath[MAX_PATH_LEN];
    get_note_path(filepath, sizeof(filepath), title);

    WINDOW *conf = newwin(5, 50, (LINES - 5) / 2, (COLS - 50) / 2);
    box(conf, 0, 0);
    mvwprintw(conf, 1, 2, "Delete note '%s'?", title);
    mvwprintw(conf, 2, 2, "Press 's' to confirm, any other to cancel.");
    wrefresh(conf);
    int ch = wgetch(conf);
    delwin(conf);
    if (ch == 's' || ch == 'S') {
        if (remove(filepath) == 0) {
            load_notes();
        } else {
            WINDOW *err = newwin(3, 40, (LINES - 3) / 2, (COLS - 40) / 2);
            box(err, 0, 0);
            mvwprintw(err, 1, 2, "Error deleting note.");
            wrefresh(err);
            wgetch(err);
            delwin(err);
        }
    }
}

void draw_main(WINDOW *win, int selected) {
    werase(win);
    box(win, 0, 0);
    char header[MAX_PATH_LEN];
    snprintf(header, sizeof(header), " %s v1.0 ", APP_NAME);
    mvwprintw(win, 0, 2, "%s", header);
    
    char info[64];
    snprintf(info, sizeof(info), " %d notes ", note_count);
    int right_pos = getmaxx(win) - strlen(info) - 2;
    mvwprintw(win, 0, right_pos, "%s", info);

    int max_y = getmaxy(win) - 2;
    int start_y = 1;
    
    if (note_count == 0) {
        mvwprintw(win, start_y, 2, "(No notes. Press 'n' to create one.)");
    } else {
        for (int i = 0; i < note_count && i < max_y; i++) {
            if (i == selected) wattron(win, A_REVERSE);
            mvwprintw(win, start_y + i, 2, "[%2d] %s", i+1, note_titles[i]);
            if (i == selected) wattroff(win, A_REVERSE);
        }
    }

    mvwprintw(win, getmaxy(win) - 1, 2, 
              "[UP/DOWN] Navigate  [Enter/E] Edit  [N] New  [D] Delete  [Q] Quit");
    wrefresh(win);
}

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
    }

    get_notes_dir(notes_path, sizeof(notes_path));
    load_notes();

    int selected = 0;
    int ch;

    WINDOW *main_win = newwin(LINES - 2, COLS - 4, 1, 2);
    keypad(main_win, TRUE);

    draw_main(main_win, selected);

    while ((ch = wgetch(main_win)) != 'q' && ch != 'Q') {
        switch (ch) {
            case KEY_UP:
                if (selected > 0) selected--;
                break;
            case KEY_DOWN:
                if (selected < note_count - 1) selected++;
                break;
            case KEY_HOME:
                selected = 0;
                break;
            case KEY_END:
                if (note_count > 0) selected = note_count - 1;
                break;
            case '\n':
            case KEY_ENTER:
            case 'e':
            case 'E':
                if (note_count > 0 && selected < note_count) {
                    edit_note(note_titles[selected]);
                    load_notes();
                    if (selected >= note_count) selected = note_count - 1;
                }
                break;
            case 'n':
            case 'N':
                new_note();
                if (selected >= note_count && note_count > 0) {
                    selected = note_count - 1;
                }
                break;
            case 'd':
            case 'D':
                if (note_count > 0 && selected < note_count) {
                    delete_note(note_titles[selected]);
                    if (selected >= note_count) selected = note_count - 1;
                }
                break;
            case KEY_RESIZE:
                wresize(main_win, LINES - 2, COLS - 4);
                mvwin(main_win, 1, 2);
                break;
            default:
                break;
        }
        draw_main(main_win, selected);
    }

    delwin(main_win);
    endwin();

    for (int i = 0; i < note_count; i++) {
        free(note_titles[i]);
    }

    return 0;
}
