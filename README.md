# Operating System Scheduling

### Vihaan Kerekatte | COMP 211 Checkoff 3

**Presentation Slides:** 

**https://docs.google.com/presentation/d/e/2PACX-1vSdi9w_AM43qYjjLOG0oF4RUi4mPo5DgTrJ7nbPtmpTh3gACOov8QtemK9xwi3aZvWirwdXpTB0Fa5L/pub?start=false&loop=false&delayms=3000**

## Topic

A typical laptop CPU has a fixed amount of cores which each have thread that can run individual processes. Each process is a program that a computer needs to run (Chrome, VSCode, Safari, Terminal, etc.). The problem is with many different processes, how can the OS choose which process should be run and when. Think about sharing a pizza between friends, how do you determine splitting it up. Just as there are many methods to split the pizza up, there are many algorithms the OS could use to schedule processes. A good scheduler has to balance three goals that often pull against each other:

- **Fairness:** every process gets a share of the CPU
- **Responsiveness:** things like typing and clicking feel instant
- **Efficiency:** the CPU never sits idle when there's work waiting

## What I Explored

1. **First-Come, First-Served (FCFS).** I started with a very simple scheduler algorithm which runs processes in the order they arrive, each one until it finishes. It's easy to understand, but it has major weaknesses.

2. **Linux's Completely Fair Scheduler (CFS).** This was my main research focus and is a more effective algorithm than FCFS. CFS gives every process a counter called **vruntime** (virtual runtime) that tracks how much CPU time it has used. Instead of running processes as they arrive, we run the process with the lowest vruntime, meaning the one that has been shortchanged the most.

3. **Niceness and priority.** Every Linux process has a nice value from −20 (highest priority) to +19 (lowest), with 0 as the default. Each nice value maps to a weight, and vruntime grows by `time × (1024 ÷ weight)`. High-priority processes have a bigger weight, so their vruntime grows slower and they get picked more often.

4. **The connection to AI data centers.** The same problem shows up at a much bigger scale. Thousands of users send prompts at once, a router decides which GPU gets each prompt, and each GPU juggles many prompts at once, a few tokens at a time.

I also tested worked out models and examples and had my AI model create some simple C programs that helped me visualize each algorithm. There is one program for each algorithm stored in this repo and can be ran after compiling.

## Running the Code
 
I wrote two small C programs that schedule the same three processes so the two algorithms can be compared side by side:
 
| Process | CPU time needed |
|---|---|
| A (video export) | 10 s |
| B (text editor) | 1 s |
| C (music player) | 1 s |
 
All three are ready at time 0, and A is first in line.
 
**Compile and run** (in the course's Linux environment):
 
```bash
gcc -o fcfs fcfs.c
gcc -o cfs cfs.c
./fcfs
./cfs
```
 
### `fcfs`: First-Come, First-Served
 
Prints a timeline where each `#` is one second on the CPU and each `.` is one second spent waiting.
 
```
A (video export) | ##########
                 | waited  0 s, finished at 10 s
 
B (text editor)  | ..........#
                 | waited 10 s, finished at 11 s
 
C (music player) | ...........#
                 | waited 11 s, finished at 12 s
 
Average wait: 7.0 s
```
 
A runs all 10 seconds before anyone else gets a turn, so the text editor sits there for 10 seconds even though it only needs 1. That's the convoy effect.
 
### `cfs`: Completely Fair Scheduler
 
First prints each process's vruntime at every second and which process the scheduler picks (always the lowest vruntime), then prints the same kind of timeline.
 
```
Time | vruntime:   A      B      C   | Runs
-----+--------------------------------+-----
   0 |            0.00   0.00   0.00   | A
   1 |            1.00   0.00   0.00   | B
   2 |            1.00   done   0.00   | C
   3 |            1.00   done   done   | A
 ...
  11 |            9.00   done   done   | A
 
A (video export) | #..#########
B (text editor)  | .#
C (music player) | ..#
 
Average wait: 1.7 s
```
 
After A runs for one second, its vruntime goes up to 1, so B and C (still at 0) get to go next. They finish quickly, and then A runs alone until it's done.
 
### Comparing the two
 
| | FCFS | CFS |
|---|---|---|
| Text editor (B) waits | 10 s | 1 s |
| Music player (C) waits | 11 s | 2 s |
| Video export (A) finishes | 10 s | 12 s |
| **Average wait** | **7.0 s** | **1.7 s** |
 
Same processes, same total work (12 seconds), but CFS cuts the average wait by about 4×. The cost is that the long job, A, finishes 2 seconds later. That's the trade-off in one table.

## Most Interesting Discoveries

- **Interactive apps feel fast because they sleep.** Apps that wait on you, like a text editor, spend most of their time asleep, so their vruntime stays low. When you press a key, they wake up with the lowest counter and run almost immediately.

- **CFS itself has already been replaced.** Since Linux 6.6 (2023), the default scheduler is **EEVDF**. It keeps the same fairness idea and adds deadlines so urgent tasks can run sooner. It is interesting how this seemingly old OS scheduling architecture is still being updated to this day.

- **AI data centers face the same problem.** One long request can hold up everyone else, which is the convoy effect again. Systems like vLLM deal with it by rescheduling after every token, which is a lot like preemption on a CPU.
 
- **There's no perfect scheduler algorithm.** Every algorithm makes a trade off. In my programs, CFS cut the average wait from 7 seconds to under 2, but the long video export finished later. Making things better for one process almost always costs another one something.

- **The simplest approach can be the most unfair.** FCFS feels fair because nobody cuts in line, but one long job can make everyone behind it wait. Whether it performs well or badly comes down to the luck of the arrival order.

## Takeaway

From your laptop to a huge data center, something always has to pick who goes next, and every choice has tradeoffs.
