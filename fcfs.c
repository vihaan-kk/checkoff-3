#include <stdio.h>

/* One process: a name and how long it needs the CPU (its burst). */
typedef struct {
    const char *name;
    int burst;      /* seconds of CPU time needed */
} Process;

int main(void) {
    /* All three arrive at time 0, in this order. */
    Process procs[] = {
        {"A (video export)", 10},
        {"B (text editor) ",  1},
        {"C (music player)",  1},
    };
    int n = sizeof(procs) / sizeof(procs[0]);

    int clock = 0;           /* current time on the CPU */
    int total_wait = 0;

    printf("FCFS schedule (each # = 1 second on the CPU)\n\n");

    /* FCFS: run each process in arrival order, start to finish. */
    for (int i = 0; i < n; i++) {
        int start  = clock;              /* waits until everyone before it is done */
        int finish = clock + procs[i].burst;

        printf("%s | ", procs[i].name);
        for (int t = 0; t < start; t++)  printf(".");   /* waiting */
        for (int t = start; t < finish; t++) printf("#"); /* running */
        printf("\n");

        printf("                 | waited %2d s, finished at %2d s\n\n",
               start, finish);

        total_wait += start;
        clock = finish;                  /* CPU is now free for the next one */
    }

    printf("Average wait: %.1f s\n", (double)total_wait / n);
    return 0;
}
