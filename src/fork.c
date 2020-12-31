#include "mm.h"
#include "sched.h"
#include "entry.h"
// #include "printf.h"

int copy_process(unsigned long fn, unsigned long arg)
{
	// We can't allow for reschedule at this point.
	preempt_disable();
	struct task_struct *p;

	// Get some space in memory to store task_struct data
	p = (struct task_struct *) get_free_page();
	if (!p)
		return 1;
	p->priority = current->priority;
	// printf("Priority of caller: %d\r\n", current->priority);
	p->state = TASK_RUNNING;
	p->counter = p->priority;
	p->preempt_count = 1; //disable preemtion until schedule_tail

	p->cpu_context.x19 = fn;
	p->cpu_context.x20 = arg;
	p->cpu_context.pc = (unsigned long)ret_from_fork;
	p->cpu_context.sp = (unsigned long)p + THREAD_SIZE;
	int pid = nr_tasks++;
	task[pid] = p;	
	preempt_enable();
	return 0;
}
