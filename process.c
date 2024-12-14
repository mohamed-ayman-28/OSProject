#include "headers.h"
#include <signal.h>
#include <stdlib.h>

/* Modify this file as needed*/
int remainingtime;

void decrementRemainingTime();

int main(int agrc, char *argv[])
{
    signal(SIGUSR1, decrementRemainingTime);
    remainingtime = atoi(argv[1]);

    initClk();

    while (remainingtime > 0)
    {
        // stay here and only respond for signals
    }

    destroyClk(false);

    return 0;
}

void decrementRemainingTime(){
    int clk_time = getClk();
    remainingtime = remainingtime - 1;
}