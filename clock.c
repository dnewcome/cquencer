#include <dispatch/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <rtmidi_c.h>

RtMidiOutPtr out_ptr;
unsigned char msg[3] = {0x90, 0x3c, 0x40};
int i = 0;
int bpm = 60;
int key = '-';
int signature = 4; // quarter notes per beat
dispatch_queue_t queue;
dispatch_source_t timer1;

void send_midi_note() {
  int retval = rtmidi_out_send_message(out_ptr, msg , 3);
}

unsigned long long int bpm_to_usec(int bpm) {
    return (NSEC_PER_SEC * 60) / bpm;
}

int notes[] = {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void draw_grid(int loc) {
    send_midi_note();
    erase();
    printw("bpm: %i\n", bpm);
    printw("key: %c\n", key);
    printw("================\n");
    printw("%i%i\n", notes[0], notes[1]);
    printw("================\n");
    mvaddstr(2, loc, "*");
}

void init_midi() {
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
  rtmidi_open_port(out_ptr, 0, "Bus 1");
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
    // signal(SIGINT, &sigtrap);   //catch the cntl-c
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
    draw_grid(0);

    int ch;
    while(ch != KEY_F(2) && ch != 'q') {
        ch = getch();
        key = ch;
        if(ch == 'j') {
            bpm--;
        }
        else if(ch == 'k') {
            bpm++;
        }
        else if(ch == 'p') {
            dispatch_resume(timer1);
        }
        else if(ch == 's') {
            dispatch_suspend(timer1);
        }
        refresh();
    }
    endwin();
    return 0;
}
