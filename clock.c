#include <dispatch/dispatch.h>
#include <stdio.h>
#include <stdlib.h>

int i = 0;
dispatch_queue_t queue;
dispatch_source_t timer1;


void sigtrap(int sig)
{
    dispatch_source_cancel(timer1);
    printf("CTRL-C received, exiting program\n");
    exit(EXIT_SUCCESS);
}

void vector1(dispatch_source_t timer)
{
        printf("a: %d\n", i);
        i++;
}

int clk_main() {

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
        printf("end\n");
        exit(0);
    });
    dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC); // after 1 sec

    // Set timer
    dispatch_source_set_timer(timer1, start, NSEC_PER_SEC / 5, 0);  // 0.2 sec
    dispatch_resume(timer1);
    while(1)
    {
        ;;
    }
    return 0;
}
