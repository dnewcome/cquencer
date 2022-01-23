#include <ncurses.h>

int frame = 0;

extern void init() {
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
}

void draw_grid(int loc) {
    printw("=====================\n");
    printw("|    |    |    |    |\n");
    printw("=====================\n");
    mvaddstr(loc+1, 1, "*");
}

int cur_main() {
    int ch;
    init();

    // while(ch != KEY_F(2)) {
    //     ch = getch();
	// 	printw("%c", ch);
    //     refresh();
    // }

	endwin();
	return 0;
}
