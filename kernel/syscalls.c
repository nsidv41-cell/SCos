/* ============================================================================
 * SCos 1.3.5 - System Calls
 * ============================================================================ */

#include "../include/scos.h"

/* System call table */
typedef int32_t (*syscall_fn)(uint32_t, uint32_t, uint32_t);

static int32_t sys_exit(uint32_t code, uint32_t unused1, uint32_t unused2);
static int32_t sys_read(uint32_t fd, uint32_t buf, uint32_t count);
static int32_t sys_write(uint32_t fd, uint32_t buf, uint32_t count);
static int32_t sys_open(uint32_t path, uint32_t flags, uint32_t mode);
static int32_t sys_close(uint32_t fd, uint32_t unused1, uint32_t unused2);
static int32_t sys_getpid(uint32_t unused1, uint32_t unused2, uint32_t unused3);
static int32_t sys_mkdir(uint32_t path, uint32_t unused1, uint32_t unused2);
static int32_t sys_unlink(uint32_t path, uint32_t unused1, uint32_t unused2);
static int32_t sys_chdir(uint32_t path, uint32_t unused1, uint32_t unused2);
static int32_t sys_getcwd(uint32_t buf, uint32_t size, uint32_t unused);
static int32_t sys_gettime(uint32_t timep, uint32_t unused1, uint32_t unused2);
static int32_t sys_sleep(uint32_t ms, uint32_t unused1, uint32_t unused2);
static int32_t sys_uptime(uint32_t unused1, uint32_t unused2, uint32_t unused3);
static int32_t sys_reboot(uint32_t unused1, uint32_t unused2, uint32_t unused3);
static int32_t sys_shutdown(uint32_t unused1, uint32_t unused2, uint32_t unused3);

static syscall_fn syscall_table[32] = {
    [SYS_EXIT]     = sys_exit,
    [SYS_READ]     = sys_read,
    [SYS_WRITE]    = sys_write,
    [SYS_OPEN]     = sys_open,
    [SYS_CLOSE]    = sys_close,
    [SYS_GETPID]   = sys_getpid,
    [SYS_MKDIR]    = sys_mkdir,
    [SYS_UNLINK]   = sys_unlink,
    [SYS_CHDIR]    = sys_chdir,
    [SYS_GETCWD]   = sys_getcwd,
    [SYS_GETTIME]  = sys_gettime,
    [SYS_SLEEP]    = sys_sleep,
    [SYS_UPTIME]   = sys_uptime,
    [SYS_REBOOT]   = sys_reboot,
    [SYS_SHUTDOWN] = sys_shutdown,
};

/* Initialize system calls */
void syscalls_init(void) {
    /* System call handler is already set up in IDT (int 0x80) */
}

/* Main system call handler - called from assembly */
int32_t syscall_handler(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    if (num >= 32 || syscall_table[num] == NULL) {
        return -1;  /* Invalid syscall */
    }
    
    return syscall_table[num](arg1, arg2, arg3);
}

/* Exit process */
static int32_t sys_exit(uint32_t code, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    process_exit((int)code);
    return 0;
}

/* Read from file descriptor */
static int32_t sys_read(uint32_t fd, uint32_t buf, uint32_t count) {
    if (fd == 0) {
        /* stdin - read from keyboard */
        char *buffer = (char *)buf;
        return keyboard_gets(buffer, (int)count);
    }
    
    /* For file descriptors, we'd need an fd table */
    return -1;
}

/* Write to file descriptor */
static int32_t sys_write(uint32_t fd, uint32_t buf, uint32_t count) {
    if (fd == 1 || fd == 2) {
        /* stdout/stderr - write to VGA */
        const char *buffer = (const char *)buf;
        for (uint32_t i = 0; i < count && buffer[i]; i++) {
            vga_putchar(buffer[i]);
        }
        return (int32_t)count;
    }
    
    return -1;
}

/* Open file */
static int32_t sys_open(uint32_t path, uint32_t flags, uint32_t mode) {
    (void)flags;
    (void)mode;
    
    const char *filepath = (const char *)path;
    if (fs_exists(filepath)) {
        /* Return a simple fd (we'd need proper fd management) */
        return 3;  /* First available fd after stdin/stdout/stderr */
    }
    
    return -1;
}

/* Close file descriptor */
static int32_t sys_close(uint32_t fd, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    if (fd > 2) {
        return 0;  /* Success */
    }
    
    return -1;  /* Can't close stdin/stdout/stderr */
}

/* Get current process ID */
static int32_t sys_getpid(uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    process_t *proc = process_get_current();
    return proc ? proc->pid : 0;
}

/* Create directory */
static int32_t sys_mkdir(uint32_t path, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    return fs_mkdir((const char *)path);
}

/* Delete file */
static int32_t sys_unlink(uint32_t path, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    return fs_delete((const char *)path);
}

/* Change directory */
static int32_t sys_chdir(uint32_t path, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    return fs_chdir((const char *)path);
}

/* Get current working directory */
static int32_t sys_getcwd(uint32_t buf, uint32_t size, uint32_t unused) {
    (void)unused;
    
    char *cwd = fs_getcwd();
    if (cwd && buf && size > 0) {
        strncpy((char *)buf, cwd, size - 1);
        ((char *)buf)[size - 1] = '\0';
        return 0;
    }
    
    return -1;
}

/* Get current time */
static int32_t sys_gettime(uint32_t timep, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    if (timep) {
        rtc_read_time((time_t_rtc *)timep);
        return 0;
    }
    
    return -1;
}

/* Sleep for milliseconds */
static int32_t sys_sleep(uint32_t ms, uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    timer_sleep(ms);
    return 0;
}

/* Get system uptime */
static int32_t sys_uptime(uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    return (int32_t)timer_get_uptime();
}

/* Reboot system */
static int32_t sys_reboot(uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    vga_puts("\nRebooting system...\n");
    timer_sleep(1000);
    
    /* Triple fault method - causes CPU reset */
    cli();
    
    /* Load null IDT */
    uint8_t null_idt[6] = {0};
    __asm__ volatile ("lidt %0" : : "m"(null_idt));
    
    /* Trigger interrupt - causes triple fault */
    __asm__ volatile ("int $0x03");
    
    /* Alternative: Use keyboard controller reset */
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);
    
    /* Should never reach here */
    while (1) {
        hlt();
    }
    
    return 0;
}

/* Shutdown system */
static int32_t sys_shutdown(uint32_t unused1, uint32_t unused2, uint32_t unused3) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    
    vga_clear();
    vga_puts("\n\n");
    vga_puts("  System is shutting down...\n\n");
    vga_puts("  Saving filesystem...\n");
    fs_save();
    
    timer_sleep(500);
    
    vga_puts("  Stopping all processes...\n");
    timer_sleep(500);
    
    vga_puts("\n  It is now safe to turn off your computer.\n");
    vga_puts("  (Or press any key to halt)\n");
    
    /* Try ACPI shutdown (works on QEMU) */
    outw(0x604, 0x2000);  /* QEMU specific */
    outw(0xB004, 0x2000); /* Bochs/older QEMU */
    
    /* If ACPI didn't work, just halt */
    cli();
    while (1) {
        hlt();
    }
    
    return 0;
}
