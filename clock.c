#include <dispatch/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <rtmidi/rtmidi_c.h>
#include "cseq.h"
#include "array.c"


RtMidiOutPtr out_ptr;
unsigned char msg[3] = {0x90, 0x24, 0x7f};
unsigned char msg_off[3] = {0x80, 0x24, 0x00};

int i = 0;
int tick = 0;
int ticks_per_quarter = 24;
int bpm = 120;
int key = ' ';
int cursor_x = 0;
int cursor_y = 3;
int signature = 4; // quarter notes per beat
int timer_reset = 0;
int running = 0;
Array events_array;
dispatch_queue_t queue;
dispatch_source_t timer1;

void send_midi_note(int ch) {
  //int retval = rtmidi_out_send_message(out_ptr, msg , 3);
  msg[0] = 0x90 + ch;
  int retval = rtmidi_out_send_message(out_ptr, msg , 3);
}

void send_midi_note_off(int ch) {
  msg_off[0] = 0x80 + ch;
  int retval = rtmidi_out_send_message(out_ptr, msg_off , 3);
}

unsigned long long int bpm_to_usec(int bpm) {
    return (NSEC_PER_SEC * 60) / bpm;
}

void insert_event(Array* a, int start, int end, int chan) {
    struct event* ev = malloc(sizeof(struct event));
    ev->start = start;
    ev->end = end;
    ev->chan = chan;
    insertArray(a, ev);
}

void init_array() {
    Array a = events_array;
    struct event* ev = malloc(sizeof(struct event));
    struct event* ev2 = malloc(sizeof(struct event));
    ev->start = 1;
    ev->end = 2;
    ev->chan = 0;
    ev2->start = 2;
    ev2->end = 3;
    ev2->chan = 0;
    initArray(&a, 5);  // initially 5 elements
    insertArray(&a, ev);
    insertArray(&a, ev2);
    freeArray(&a);
}

struct event events[] = {
    {.start = 1, .end = 3, .chan = 0},
    {.start = 5, .end = 7, .chan = 0},
    {.start = 13, .end = 14, .chan = 1}
};
int events_len = 3;

int notes[][16] = {
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 1,0,0,0, 1,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0 },
    { 0,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,0,0 }
};

void clear_track(int n) {
    for(int ij = 0; ij < 16; ij++) {
        notes[n][ij] = 0;
    }
}

int tracks[] = {1,2,3,4};
int tracks_len = 4;

void draw_grid_sparse(int loc) {
    int offset = 3;
    for(int t = 0; t < events_len; t++) {
        for(int j = 0; j < events[t].end - events[t].start; j++) {
            mvaddch(offset+events[t].chan, events[t].start + j, 'x');
        }
    }
}

void draw_grid(int loc) {
	/*
    for(int t = 0; t < tracks_len; t++) {
        if(notes[t][loc] != 0) {
            send_midi_note(t);
            send_midi_note_off(t);
        }
    }
    */
    erase();
    printw("bpm: %i step: %i tick: %i\n", bpm, i%16, tick);
    printw("key: %c %i\n", key, key);
    printw("================\n");
    for(int k = 0; k < tracks_len; k++) {
        for(int j = 0; j < 16; j++) {
            printw("%i", notes[k][j]);
        }
        printw("\n");
    }
    printw("================\n");
    attron(A_REVERSE);
    mvaddch(2, loc, '=');
    attroff(A_REVERSE);
    move(cursor_y, cursor_x);
    if(timer_reset != 0) {
        dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, 0);
        dispatch_source_set_timer(timer1, start, bpm_to_usec(bpm)/signature/ticks_per_quarter, 0);
        timer_reset = 0;
    }
}

void list_midi_ports() {
  char* buf = (char*)malloc(128*sizeof(char));

  // enumerate the available apis.. could be core, alsa, jack, etc
  int num_apis = rtmidi_get_compiled_api(NULL, 0);
  printf("we have %i apis\n", num_apis);
  enum RtMidiApi* apis = malloc(num_apis * sizeof(enum RtMidiApi));
  rtmidi_get_compiled_api(apis, num_apis);
  // name of first api is `core' on osx
  for(int api_num = 0; api_num < num_apis; api_num++) {
      printf("api %i name is %s\n", api_num, rtmidi_api_name(apis[api_num]));
  }

  out_ptr = rtmidi_out_create(apis[0], "test");
  unsigned int num_ports = rtmidi_get_port_count(out_ptr);
  for(int port_num = 0; port_num < num_ports; port_num++) {
      rtmidi_get_port_name(out_ptr, port_num, buf, &((int){128}));
      printf("port %i name is %s\n", port_num, buf);
  }
}

void init_midi() {
  list_midi_ports();
  char* buf =  (char*)malloc(128*sizeof(char));
  // enumerate the available apis.. could be core, alsa, jack, etc
  int num_apis = rtmidi_get_compiled_api(NULL, 0);
  printf("we have %i apis\n", num_apis);
  enum RtMidiApi* apis = malloc(num_apis * sizeof(enum RtMidiApi));
  rtmidi_get_compiled_api(apis, num_apis);
  // name of first api is `core' on osx
  printf("api name is %s\n", rtmidi_api_name(apis[0]));

  // open a port 
  out_ptr = rtmidi_out_create(apis[0], "test");
  unsigned int num_ports = rtmidi_get_port_count(out_ptr);
  printf("we have %i ports\n", num_ports);
  rtmidi_get_port_name(out_ptr, 0, buf, &((int){128}));
  printf("port name for %i is %s \n", 0, buf);
  rtmidi_open_port(out_ptr, 4, "Bus 1");
}


extern void init() {
    initscr();
    // curs_set(0);
    move(cursor_y, cursor_x);
    raw();
    keypad(stdscr, TRUE);
    noecho();
    refresh();
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
    for(int t = 0; t < tracks_len; t++) {
	if(notes[t][tick/ticks_per_quarter%16] != 0) {
	    send_midi_note(t);
	    send_midi_note_off(t);
	}
    }
        // draw_grid_sparse(i%16);
        refresh();
        tick++;
	if(tick % ticks_per_quarter == 0) {
            draw_grid(i%16);
            i++;
	}
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
    dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, 0);

    // Set timer
    dispatch_source_set_timer(timer1, start, bpm_to_usec(bpm)/signature/ticks_per_quarter, 0);
    draw_grid(0);
    // draw_grid_sparse(0);

    int ch = 0;
    int ch2 = 0;
    while(ch != KEY_F(2) && ch != 'q') {
        ch2 = ch;
        ch = getch();
        key = ch;
        if(ch == '-') {
            bpm--;
            timer_reset = 1;
            draw_grid(i%16);
        }
	// left arrow, nudge tempo
	else if(ch == 260) {
            draw_grid(i%16);
        }
        else if(ch == '+') {
            bpm++;
            timer_reset = 1;
            draw_grid(i%16);
        }
	// right arrow, nudge tempo
	else if(ch == 261) {
            draw_grid(i%16);
        }
        else if(ch == 'g' && ch2 == 'g') {
	    i = 0;
            // timer_reset = 1;
            draw_grid(i%16);
        }
        else if(ch == 'd' && ch2 == 'd') {
	    clear_track(cursor_y-3);
            // timer_reset = 1;
            draw_grid(i%16);
        }
        else if(ch == ' ') {
            if(running) {
                dispatch_suspend(timer1);
                running = 0;
            }
            else {
                dispatch_resume(timer1);
                running = 1;
            }
        }
        else if(ch == 'h') {
            cursor_x = cursor_x == 0 ? 0 : cursor_x - 1;
            move(cursor_y, cursor_x);
        }
        else if(ch == 'l') {
            cursor_x = cursor_x == 15 ? 15 : cursor_x + 1;
            move(cursor_y, cursor_x);
        }
        else if(ch == 'x') {
            notes[cursor_y-3][cursor_x] = !notes[cursor_y-3][cursor_x];
            draw_grid(i%16);
            // draw_grid_sparse(i%16);
        }
        else if(ch == 'k') {
            cursor_y = cursor_y == 3 ? 3 : cursor_y - 1;
            move(cursor_y, cursor_x);
        }
        else if(ch == 'j') {
            cursor_y = cursor_y == tracks_len+2 ? tracks_len+2 : cursor_y + 1;
            move(cursor_y, cursor_x);
        }
	else {
            draw_grid(i%16);
	}
        refresh();
    }
    endwin();
    return 0;
}
