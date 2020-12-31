#ifndef _SCHED_H
#define _SCHED_H

// offset of cpu_context in task_struct
#define THREAD_CPU_CONTEXT			0 		

#ifndef __ASSEMBLER__

// used to offset start addr of allocated page
// to usable stack.
#define THREAD_SIZE				4096 

// Size of task array.
#define NR_TASKS				64 

// Unused for now.
// #define FIRST_TASK task[0]
// #define LAST_TASK task[NR_TASKS-1]

// Only defined process status.
#define TASK_RUNNING				0

// Linked externally, and defined first in sched.c
// The current task that is running.
extern struct task_struct *current;
// defined in sched.c
// Array of pointers to task structs.
extern struct task_struct *task[NR_TASKS];
// set to one initially in sched.c
// Current number of tasks running.
extern int nr_tasks;

// struct representing registers that have to be
// saved between context switching.
struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

struct task_struct {
	struct cpu_context cpu_context;
	long state;	
	long counter;
	long priority;
	long preempt_count;
};

//extern void sched_init(void);

// Voluntary method used to allow other processes
// to continue running.
extern void schedule(void);

// Used in timer interrupt handler to 
// decrement current running processes counter
// and to schedule a newer process.
extern void timer_tick(void);

// Disable a process from being preempted.
extern void preempt_disable(void);

// Enable a process to be preempted.
extern void preempt_enable(void);

// Preliminary step to cpu_switch_to.
// Swap current running process to next.
extern void switch_to(struct task_struct* next);

// Save caller register data into memory, and load next register data from
// memory.
extern void cpu_switch_to(struct task_struct* prev, struct task_struct* next);

// Used to initialize the init_task when kernel first starts.
#define INIT_TASK \
/*cpu_context*/	{ {0,0,0,0,0,0,0,0,0,0,0,0,0}, \
/* state etc */	0,0,1, 0 \
}

#endif
#endif
