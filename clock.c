#include <dispatch/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

int i = 0;
int bpm = 120;
int signature = 4; // quarter notes per beat
dispatch_queue_t queue;
dispatch_source_t timer1;

unsigned long long int bpm_to_usec(int bpm) {
    return (NSEC_PER_SEC * 60) / bpm;
}

int notes[] = {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void draw_grid(int loc) {
    erase();
    printw("bpm: %i\n", bpm);
    printw("================\n");
    printw("%i%i\n", notes[0], notes[1]);
    printw("================\n");
    mvaddstr(2, loc, "*");
}

extern void init() {
	initscr();
    curs_set(0);
	raw();
	keypad(stdscr, TRUE);
	noecho();
}

void sigtrap(int sig)
{
    dispatch_source_cancel(timer1);
    printw("CTRL-C received, exiting program\n");
    refresh();
    exit(EXIT_SUCCESS);
}

void vector1(dispatch_source_t timer)
{
        // printw("a: %d\n", i);
        draw_grid(i%16);
        refresh();
        i++;
}

int clk_main() {
    init();
    signal(SIGINT, &sigtrap);   //catch the cntl-c
    queue = dispatch_queue_create("timerQueue", 0);

    // Create dispatch timer source
    timer1 = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
    
    // Set block for dispatch source when catched events
    dispatch_source_set_event_handler(timer1, ^{vector1(timer1);});

    // Set block for dispatch source when cancel source
    dispatch_source_set_cancel_handler(timer1, ^{
        dispatch_release(timer1);
        dispatch_release(queue);
        printw("end\n");
        refresh();
        exit(0);
    });
    dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, 0); // after 1 sec

    // Set timer
    dispatch_source_set_timer(timer1, start, bpm_to_usec(bpm)/signature, 0);
    dispatch_resume(timer1);

    int ch;
    while(ch != KEY_F(2)) {
        ch = getch();
        if(ch == 'j') {
            bpm--;
        }
        else if(ch == 'k') {
            bpm++;
        }
        refresh();
    }
    return 0;
}
