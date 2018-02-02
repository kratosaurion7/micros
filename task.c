#include "task.h"
#include "timer.h"

void init_kernel_scheduler()
{
    sched = kmalloc(sizeof(struct kernel_scheduler_module));
    sched->ts = kmalloc(sizeof(struct threadset));
    sched->ts->list = vector_create();
    sched->current = NULL;
    sched->currentIndex = 0;
    
    sched->max_run_time = 200;
}

struct task_t* ks_get_current()
{
    return sched->current;
}

void ks_suspend_stage2()
{
    struct regs_t myregs = ks_get_stacked_registers();
    
    struct task_t* t = ks_get_current();

    /* Few things to suspend a thread
     * 1. Save the registers
     * 2. Mark thread as suspended
     * 3. Put it ad the end of the waitlist
     */
    t->regs = myregs;

    t->state = T_SUSPENDED;

    // Tell scheduler to pull up the next thread
    uint32_t nextIndex = 0;
    struct task_t* next = ks_get_next_thread(&nextIndex);
    ks_activate(next);
}

void ks_activate(struct task_t* next)
{
    sched->current->state = T_SUSPENDED;
    sched->current->ms_count_running = 0;
    
    sched->current = next;

    if(next->regs.eip == 0)
    {
        next->regs.eip = next->entryAddr;
    }

    /* To activate a thread
     * 1. Mark as RUNNING
     * 2. Load the saved registers
     *    At this point, we can't use the stack
     * 3. JMP to the saved EIP
     *    Or do 'push eax; ret'
     */

    ks_do_activate(next);

    // Open question
    // How to change the stack pointer and keep a ref to
    // the 'next' parameter to use EIP ?
    // Maybe call an interrupt ? Interrupt + replace stack to iret to new task
}

struct task_t* ks_create_thread(uint32_t entrypoint)
{
    struct task_t* newTask = (struct task_t*)kmalloc(sizeof(struct task_t));
    newTask->ms_count_running = 0;
    newTask->ms_count_total = 0;

    newTask->entryAddr = entrypoint;
    newTask->state = T_WAITING;

    size_t stackSize = 4096;
    newTask->stackAddr = (uint32_t)malloc(stackSize); // Create a stack
    newTask->regs.esp = newTask->stackAddr + 4096;

    // Setting the regs to testable values
    newTask->regs.eax = 1;
    newTask->regs.ecx = 2;
    newTask->regs.edx = 3;
    newTask->regs.ebx = 4;
    newTask->regs.ebp = newTask->regs.esp;
    newTask->regs.esi = 7;
    newTask->regs.edi = 8;
    newTask->regs.eip = 0;
    newTask->regs.cs = 10;
    newTask->regs.flags = 0;

    vector_add(sched->ts->list, newTask);

    return newTask;
}

struct task_t* ks_get_next_thread(uint32_t* nextIndex)
{
    size_t next = (sched->currentIndex + 1) % sched->ts->list->count;
    struct task_t* t = vector_get_at(sched->ts->list, next);

    *nextIndex = next;

    return t;
}

void ks_update_task()
{
    struct task_t* c = ks_get_current();
    
    uint32_t timer_tickrate = get_timer_rate();
    c->ms_count_total += timer_tickrate;
    c->ms_count_running += timer_tickrate;
}

BOOL ks_should_preempt_current()
{
    struct task_t* c = ks_get_current();
    
    BOOL timeout = c->ms_count_running > sched->max_run_time;
    BOOL hasOtherTasks = sched->ts->list->count > 1;
    
    if(
        timeout && 
        hasOtherTasks)
    {
        Debugger();
        
        return TRUE;
    }
    
    return FALSE;
}

struct task_t* ks_preempt_current(registers_t* from)
{
    struct task_t* currentTask = ks_get_current();
    currentTask->state = T_SUSPENDED;
    currentTask->regs.eax = from->eax;
    currentTask->regs.ecx = from->ecx;
    currentTask->regs.edx = from->edx;
    currentTask->regs.ebx = from->ebx;
    currentTask->regs.esp = from->esp + 16;
    currentTask->regs.ebp = from->ebp;
    currentTask->regs.esi = from->esi;
    currentTask->regs.edi = from->edi;
    currentTask->regs.flags = from->eflags;
    currentTask->entryAddr = from->eip; // TODO : Replace
    
    currentTask->ms_count_running = 0;

    uint32_t nextIndex = 0;
    struct task_t* nextTask = ks_get_next_thread(&nextIndex);
    nextTask->state = T_RUNNING;

    sched->current = nextTask;
    sched->currentIndex = nextIndex;

    return nextTask;
}
