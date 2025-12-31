/* ============================================================================
 * SCos 1.3.5 - Process Scheduler
 * ============================================================================ */

#include "../include/scos.h"

/* Process table */
static process_t processes[MAX_PROCESSES];
static pid_t current_pid = 0;
static pid_t next_pid = 1;
static int scheduler_enabled = 0;

/* Initialize scheduler */
void scheduler_init(void) {
    /* Clear process table */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].state = PROCESS_STATE_UNUSED;
        processes[i].pid = 0;
    }
    
    /* Create kernel/init process (PID 0) */
    processes[0].pid = 0;
    strcpy(processes[0].name, "kernel");
    processes[0].state = PROCESS_STATE_RUNNING;
    processes[0].start_time = timer_get_ticks();
    processes[0].cpu_time = 0;
    
    current_pid = 0;
    scheduler_enabled = 1;
}

/* Find a free process slot */
static int find_free_slot(void) {
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_STATE_UNUSED) {
            return i;
        }
    }
    return -1;
}

/* Create a new process */
pid_t process_create(const char *name, void (*entry)(void)) {
    int slot = find_free_slot();
    if (slot < 0) {
        return -1;  /* No free slots */
    }
    
    process_t *proc = &processes[slot];
    proc->pid = next_pid++;
    strncpy(proc->name, name, 31);
    proc->name[31] = '\0';
    proc->state = PROCESS_STATE_READY;
    proc->start_time = timer_get_ticks();
    proc->cpu_time = 0;
    proc->eip = (uint32_t)entry;
    proc->esp = (uint32_t)&proc->stack[PROCESS_STACK_SIZE / 4 - 1];
    proc->ebp = proc->esp;
    
    return proc->pid;
}

/* Exit current process */
void process_exit(int code) {
    if (current_pid == 0) {
        kernel_panic("Kernel process cannot exit");
        return;
    }
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == current_pid) {
            processes[i].state = PROCESS_STATE_ZOMBIE;
            processes[i].exit_code = code;
            break;
        }
    }
    
    schedule();
}

/* Kill a process by PID */
void process_kill(pid_t pid) {
    if (pid == 0) {
        vga_puts("Cannot kill kernel process\n");
        return;
    }
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == pid && processes[i].state != PROCESS_STATE_UNUSED) {
            processes[i].state = PROCESS_STATE_ZOMBIE;
            processes[i].exit_code = -1;
            return;
        }
    }
}

/* Get current process */
process_t *process_get_current(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == current_pid) {
            return &processes[i];
        }
    }
    return &processes[0];  /* Default to kernel process */
}

/* Get process by PID */
process_t *process_get_by_pid(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == pid && processes[i].state != PROCESS_STATE_UNUSED) {
            return &processes[i];
        }
    }
    return NULL;
}

/* List all processes */
void process_list(void) {
    vga_puts("PID   STATE      CPU TIME   NAME\n");
    vga_puts("----  ---------  ---------  ----------------\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state != PROCESS_STATE_UNUSED) {
            char buf[80];
            const char *state_str;
            
            switch (processes[i].state) {
                case PROCESS_STATE_RUNNING: state_str = "RUNNING  "; break;
                case PROCESS_STATE_READY:   state_str = "READY    "; break;
                case PROCESS_STATE_BLOCKED: state_str = "BLOCKED  "; break;
                case PROCESS_STATE_ZOMBIE:  state_str = "ZOMBIE   "; break;
                default: state_str = "UNKNOWN  "; break;
            }
            
            sprintf(buf, "%-4d  %s  %-9d  %s\n",
                    processes[i].pid,
                    state_str,
                    processes[i].cpu_time,
                    processes[i].name);
            vga_puts(buf);
        }
    }
}

/* Simple round-robin scheduler */
void schedule(void) {
    if (!scheduler_enabled) {
        return;
    }
    
    /* Clean up zombie processes */
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_STATE_ZOMBIE) {
            processes[i].state = PROCESS_STATE_UNUSED;
            processes[i].pid = 0;
        }
    }
    
    /* For now, just stay with current process (shell) */
    /* A full context switch would require saving/restoring registers */
}

/* Block current process */
void process_block(void) {
    process_t *proc = process_get_current();
    if (proc != NULL && proc->pid != 0) {
        proc->state = PROCESS_STATE_BLOCKED;
        schedule();
    }
}

/* Unblock a process */
void process_unblock(pid_t pid) {
    process_t *proc = process_get_by_pid(pid);
    if (proc != NULL && proc->state == PROCESS_STATE_BLOCKED) {
        proc->state = PROCESS_STATE_READY;
    }
}

/* Count active processes */
int process_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state != PROCESS_STATE_UNUSED &&
            processes[i].state != PROCESS_STATE_ZOMBIE) {
            count++;
        }
    }
    return count;
}
