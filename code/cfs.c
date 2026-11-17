#include <stdio.h>

/*
 * Simplified Linux CFS (Completely Fair Scheduler) demo.
 *
 * Core rule: the process with the SMALLEST virtual runtime runs next.
 *
 * Simplifications vs. real Linux:
 *   - Real CFS keeps processes in a red-black tree (leftmost = smallest
 *     vruntime). Here we just scan the array to find the smallest.
 *   - Fixed 1-second time slice (real CFS adjusts it based on load).
 *   - No sleeping/waking; every process is always ready until it finishes.
 */

#define SLICE 1                /* seconds per turn */

typedef struct {
    const char *name;
    int burst;          /* total CPU seconds needed */
    int nice;           /* -20 (highest priority) to 19 (lowest). 0 = normal. */
    int remaining;      /* CPU seconds still needed */
    double vruntime;    /* CPU time used so far, adjusted for nice */
    int first_run;      /* time it first got the CPU (-1 = not yet) */
    int finish;         /* time it finished */
    char timeline[64];  /* '#' = running, '.' = waiting, per second */
} Process;

/* How fast vruntime grows for a given nice value.
 * Each nice step changes the rate by 1.25x:
 *   nice  0 -> 1.00 (normal)
 *   nice  5 -> ~3.05 (vruntime grows faster, so it runs less often)
 *   nice -5 -> ~0.33 (vruntime grows slower, so it runs more often)
 * Real Linux gets nearly the same numbers from a lookup table. */
double nice_rate(int nice) {
    double rate = 1.0;
    for (int i = 0; i < nice; i++)  rate *= 1.25;   /* lower priority  */
    for (int i = 0; i > nice; i--)  rate /= 1.25;   /* higher priority */
    return rate;
}

/* Return the index of the unfinished process with the smallest vruntime.
 * Ties go to the earlier process in the array. Real CFS: leftmost tree node. */
int pick_next(Process p[], int n) {
    int best = -1;
    for (int i = 0; i < n; i++) {
        if (p[i].remaining == 0) continue;               /* finished */
        if (best == -1 || p[i].vruntime < p[best].vruntime) best = i;
    }
    return best;
}

int main(void) {
    /* Same example as FCFS. All arrive at time 0, all nice 0.
     * To see priority in action, add a 4th long-running process with a
     * lower priority, e.g. {"D (backup job)  ", 10, 5, 0, 0, -1, 0, ""}
     * (nice 5), and add a "D" column to the header printf. */
    Process p[] = {
        /* name               burst nice */
        {"A (video export)", 10,    0, 0, 0, -1, 0, ""},
        {"B (text editor) ",  1,    0, 0, 0, -1, 0, ""},
        {"C (music player)",  1,    0, 0, 0, -1, 0, ""},
    };
    int n = sizeof(p) / sizeof(p[0]);
    for (int i = 0; i < n; i++) p[i].remaining = p[i].burst;

    int clock = 0;

    printf("Time | vruntime:   A      B      C   | Runs\n");
    printf("-----+--------------------------------+-----\n");

    int cur;
    while ((cur = pick_next(p, n)) != -1) {
        /* Show everyone's vruntime at the moment the scheduler decides. */
        printf("%4d |          ", clock);
        for (int i = 0; i < n; i++) {
            if (p[i].remaining == 0) printf("  done ");
            else                     printf("%6.2f ", p[i].vruntime);
        }
        printf("  | %c\n", p[cur].name[0]);

        if (p[cur].first_run == -1) p[cur].first_run = clock;

        /* Record this second on every unfinished process's timeline. */
        for (int i = 0; i < n; i++)
            if (p[i].remaining > 0)
                p[i].timeline[clock] = (i == cur) ? '#' : '.';

        /* Run for one slice. A nice 0 process gains exactly SLICE of vruntime;
         * a higher-priority (lower nice) one gains less, a lower-priority one more. */
        p[cur].remaining -= SLICE;
        p[cur].vruntime  += SLICE * nice_rate(p[cur].nice);
        clock += SLICE;

        if (p[cur].remaining == 0) p[cur].finish = clock;
    }

    printf("\nCFS schedule (# = running, . = waiting)\n\n");
    double total_wait = 0;
    for (int i = 0; i < n; i++) {
        int wait = p[i].finish - p[i].burst;   /* everyone arrived at 0 */
        total_wait += wait;
        printf("%s | %s\n", p[i].name, p[i].timeline);
        printf("                 | first ran at %2d s, waited %2d s, finished at %2d s\n\n",
               p[i].first_run, wait, p[i].finish);
    }
    printf("Average wait: %.1f s\n", total_wait / n);
    return 0;
}
