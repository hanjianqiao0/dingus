#include "kernel.h"

struct TASK_CTL *task_ctl;
struct CLOCK *task_clock;

struct TASK *first_task(void){
	struct TASK *task;
	task = task_alloc();
	load_tr(task->sel);
	task_clock = clock_alloc();
	clock_set_time(task_clock, task->priority);
	return task;
}

struct TASK *task_now(void){
	return task_ctl->tasks[task_ctl->now_task];
}

void task_add(struct TASK *task){
	task_ctl->tasks[task_ctl->running] = task;
	task_ctl->running++;
	task->flags = 2;
	return;
}

void task_remove(struct TASK *task){
	int i;

	for (i = 0; i < task_ctl->running; i++){
		if (task_ctl->tasks[i] == task){
			break;
		}
	}

	task->flags = 1;
	task_ctl->running--;
	if (task_ctl->now_task < task_ctl->running){
		for (; i < task_ctl->running; i++){
			task_ctl->tasks[i] = task_ctl->tasks[i + 1];
		}		
	}

	return;
}

void task_init(struct MEMMAN *memman){
	int i;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;

	task_ctl = (struct TASK_CTL *) memman_alloc_4k(memman, sizeof (struct TASK_CTL));
	for (i = 0; i < MAX_TASKS; i++){
		task_ctl->tasks0[i].flags = 0;
		task_ctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		task_ctl->tasks0[i].tss.ldtr = (TASK_GDT0 + MAX_TASKS + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &task_ctl->tasks0[i].tss, AR_TSS32);
		set_segmdesc(gdt + TASK_GDT0 + MAX_TASKS + i, 15, (int) task_ctl->tasks0[i].ldt, AR_LDT);
	}
	return;
}

struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++){
		if (task_ctl->tasks0[i].flags == 0){
			task = &task_ctl->tasks0[i];
			task->flags = 1;
			task->tss.eflags = 0x00000202;
			task->tss.eax = 0;
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.iomap = 0x40000000;
			task->tss.ss0 = 0;
			return task;
		}
	}
	return 0;
}

void task_run(struct TASK *task, int priority)
{
	if (priority > 0){
		task->priority = priority;
	}else{
		return;
	}

	if (task->flags != 2){
		task_add(task);
	}

	return;
}

void task_sleep(struct TASK *task){
	struct TASK *now_task;
	if (task->flags == 2){
		now_task = task_now();
		task_remove(task);
		if (task == now_task){
			task_switch();
			now_task = task_now();
			far_jmp(0, now_task->sel);
		}
	}
	return;
}

void task_switch(void){
	struct TASK *new_task, *now_task = task_ctl->tasks[task_ctl->now_task];
	task_ctl->now_task++;
	if (task_ctl->now_task >= task_ctl->running){
		task_ctl->now_task = 0;
	}
	new_task = task_ctl->tasks[task_ctl->now_task];
	clock_set_time(task_clock, new_task->priority);
	if (new_task != now_task){
		far_jmp(0, new_task->sel);
	}
	return;
}
