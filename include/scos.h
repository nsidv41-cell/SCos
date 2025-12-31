#ifndef SCOS_H
#define SCOS_H

/* Standard type definitions */
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef uint32_t           size_t;
typedef int32_t            ssize_t;
typedef int32_t            pid_t;

#define NULL ((void*)0)
#define true 1
#define false 0
typedef int bool;

/* SCos Theme Colors */
#define SCOS_GREEN      0x0A    /* Bright green on black */
#define SCOS_BLACK      0x00
#define SCOS_ATTR       0x0A    /* Green text, black background */
#define SCOS_PROMPT_ATTR 0x0A

/* VGA Constants */
#define VGA_WIDTH       80
#define VGA_HEIGHT      25
#define VGA_MEMORY      0xB8000

/* Keyboard Scancodes */
#define KEY_BACKSPACE   0x0E
#define KEY_TAB         0x0F
#define KEY_ENTER       0x1C
#define KEY_LCTRL       0x1D
#define KEY_LSHIFT      0x2A
#define KEY_RSHIFT      0x36
#define KEY_LALT        0x38
#define KEY_CAPSLOCK    0x3A
#define KEY_F1          0x3B
#define KEY_F2          0x3C
#define KEY_F3          0x3D
#define KEY_F10         0x44
#define KEY_ESC         0x01
#define KEY_UP          0x48
#define KEY_DOWN        0x50
#define KEY_LEFT        0x4B
#define KEY_RIGHT       0x4D
#define KEY_HOME        0x47
#define KEY_END         0x4F
#define KEY_PGUP        0x49
#define KEY_PGDN        0x51
#define KEY_DEL         0x53

/* Memory Constants */
#define KERNEL_HEAP_START   0x200000    /* 2MB */
#define KERNEL_HEAP_SIZE    0x400000    /* 4MB heap */
#define PAGE_SIZE           4096

/* Filesystem Constants */
#define FS_MAX_FILES        256
#define FS_MAX_NAME         64
#define FS_MAX_PATH         256
#define FS_MAX_CONTENT      8192
#define FS_TYPE_FILE        1
#define FS_TYPE_DIR         2

/* Process Constants */
#define MAX_PROCESSES       32
#define PROCESS_STACK_SIZE  4096
#define PROCESS_STATE_UNUSED    0
#define PROCESS_STATE_READY     1
#define PROCESS_STATE_RUNNING   2
#define PROCESS_STATE_BLOCKED   3
#define PROCESS_STATE_ZOMBIE    4

/* Shell Constants */
#define CMD_MAX_LENGTH      256
#define CMD_HISTORY_SIZE    20
#define MAX_ARGS            16
#define MAX_ALIASES         16
#define ALIAS_NAME_LEN      32
#define ALIAS_CMD_LEN       128

/* System Call Numbers */
#define SYS_EXIT        0
#define SYS_READ        1
#define SYS_WRITE       2
#define SYS_OPEN        3
#define SYS_CLOSE       4
#define SYS_GETPID      5
#define SYS_FORK        6
#define SYS_EXEC        7
#define SYS_WAIT        8
#define SYS_MKDIR       9
#define SYS_RMDIR       10
#define SYS_UNLINK      11
#define SYS_CHDIR       12
#define SYS_GETCWD      13
#define SYS_STAT        14
#define SYS_GETTIME     15
#define SYS_SLEEP       16
#define SYS_KILL        17
#define SYS_UPTIME      18
#define SYS_REBOOT      19
#define SYS_SHUTDOWN    20

/* Structures */

/* Time structure */
typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t weekday;
} time_t_rtc;

/* Filesystem node */
typedef struct fs_node {
    char name[FS_MAX_NAME];
    uint8_t type;
    uint32_t size;
    uint32_t parent_idx;
    uint32_t created;
    uint32_t modified;
    char content[FS_MAX_CONTENT];
    uint8_t in_use;
} fs_node_t;

/* Process control block */
typedef struct {
    pid_t pid;
    char name[32];
    uint8_t state;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t stack[PROCESS_STACK_SIZE / 4];
    uint32_t start_time;
    uint32_t cpu_time;
    int32_t exit_code;
} process_t;

/* Command alias */
typedef struct {
    char name[ALIAS_NAME_LEN];
    char command[ALIAS_CMD_LEN];
    uint8_t in_use;
} alias_t;

/* GDT Entry */
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

/* GDT Pointer */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

/* IDT Entry */
typedef struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

/* IDT Pointer */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

/* Registers structure for interrupt handlers */
typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

/* Memory block header */
typedef struct mem_block {
    uint32_t size;
    uint8_t is_free;
    struct mem_block *next;
    struct mem_block *prev;
    uint32_t magic;
} mem_block_t;

#define MEM_MAGIC 0xDEADBEEF

/* ============ Function Declarations ============ */

/* VGA Functions - vga.c */
void vga_init(void);
void vga_clear(void);
void vga_putchar(char c);
void vga_puts(const char *str);
void vga_put_color(const char *str, uint8_t color);
void vga_set_cursor(int x, int y);
void vga_get_cursor(int *x, int *y);
void vga_scroll(void);
void vga_set_attr(uint8_t attr);
uint8_t vga_get_attr(void);
void vga_enable_cursor(void);
void vga_disable_cursor(void);
void vga_putchar_at(char c, int x, int y, uint8_t attr);
void vga_clear_line(int line);

/* Keyboard Functions - keyboard.c */
void keyboard_init(void);
void keyboard_handler(registers_t *regs);
char keyboard_getchar(void);
char keyboard_getchar_nonblock(void);
int keyboard_gets(char *buffer, int max_len);
int keyboard_is_key_pressed(uint8_t scancode);
void keyboard_set_leds(uint8_t leds);

/* Timer Functions - timer.c */
void timer_init(uint32_t frequency);
void timer_handler(registers_t *regs);
uint32_t timer_get_ticks(void);
void timer_sleep(uint32_t ms);
uint32_t timer_get_uptime(void);

/* RTC Functions - rtc.c */
void rtc_init(void);
void rtc_read_time(time_t_rtc *time);
uint32_t rtc_get_timestamp(void);
void rtc_format_time(time_t_rtc *time, char *buffer);
void rtc_format_date(time_t_rtc *time, char *buffer);
int rtc_get_days_in_month(int month, int year);
int rtc_is_leap_year(int year);
const char* rtc_get_weekday_name(int weekday);
const char* rtc_get_month_name(int month);

/* Memory Functions - memory.c */
void memory_init(void);
void *kmalloc(size_t size);
void *kmalloc_aligned(size_t size, size_t alignment);
void kfree(void *ptr);
void *krealloc(void *ptr, size_t size);
size_t memory_get_free(void);
size_t memory_get_used(void);
void memory_dump(void);

/* Interrupt Functions - interrupts.c */
void interrupts_init(void);
void isr_handler(registers_t *regs);
void irq_handler(registers_t *regs);
void irq_install_handler(int irq, void (*handler)(registers_t *));
void irq_uninstall_handler(int irq);

/* Scheduler Functions - scheduler.c */
void scheduler_init(void);
pid_t process_create(const char *name, void (*entry)(void));
void process_exit(int code);
void process_kill(pid_t pid);
process_t *process_get_current(void);
process_t *process_get_by_pid(pid_t pid);
void process_list(void);
void schedule(void);
void process_block(void);
void process_unblock(pid_t pid);
int process_count(void);

/* Syscall Functions - syscalls.c */
void syscalls_init(void);
int32_t syscall_handler(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

/* Filesystem Functions - fs.c */
void fs_init(void);
int fs_create(const char *path, uint8_t type);
int fs_delete(const char *path);
int fs_read(const char *path, char *buffer, size_t size);
int fs_write(const char *path, const char *data, size_t size);
int fs_append(const char *path, const char *data, size_t size);
int fs_exists(const char *path);
int fs_is_dir(const char *path);
int fs_list(const char *path, char *buffer, size_t size);
int fs_chdir(const char *path);
char *fs_getcwd(void);
int fs_mkdir(const char *path);
int fs_touch(const char *path);
int fs_stat(const char *path, uint32_t *size, uint8_t *type);
void fs_save(void);
void fs_load(void);
int fs_get_parent_path(const char *path, char *parent);
int fs_get_basename(const char *path, char *name);
void fs_normalize_path(const char *path, char *normalized);

/* String Functions - string.c */
size_t strlen(const char *str);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strchr(const char *str, int c);
char *strrchr(const char *str, int c);
char *strstr(const char *haystack, const char *needle);
void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t num);
void *memmove(void *dest, const void *src, size_t num);
int memcmp(const void *s1, const void *s2, size_t n);
int atoi(const char *str);
void itoa(int value, char *str, int base);
void utoa(uint32_t value, char *str, int base);
int isdigit(int c);
int isalpha(int c);
int isalnum(int c);
int isspace(int c);
int toupper(int c);
int tolower(int c);
char *strtok(char *str, const char *delim);
int sprintf(char *buf, const char *fmt, ...);

/* Shell Functions - shell.c */
void shell_init(void);
void shell_run(void);
void shell_execute(const char *cmdline);
void shell_add_history(const char *cmd);
const char *shell_get_history(int index);
void shell_set_alias(const char *name, const char *command);
const char *shell_get_alias(const char *name);
void shell_print_prompt(void);

/* Command Functions - commands.c */
void cmd_help(int argc, char **argv);
void cmd_ls(int argc, char **argv);
void cmd_cd(int argc, char **argv);
void cmd_pwd(int argc, char **argv);
void cmd_cat(int argc, char **argv);
void cmd_clear(int argc, char **argv);
void cmd_echo(int argc, char **argv);
void cmd_date(int argc, char **argv);
void cmd_mkdir(int argc, char **argv);
void cmd_touch(int argc, char **argv);
void cmd_rm(int argc, char **argv);
void cmd_whoami(int argc, char **argv);
void cmd_version(int argc, char **argv);
void cmd_history(int argc, char **argv);
void cmd_alias(int argc, char **argv);
void cmd_ps(int argc, char **argv);
void cmd_kill(int argc, char **argv);
void cmd_uptime(int argc, char **argv);
void cmd_sysinfo(int argc, char **argv);
void cmd_shutdown(int argc, char **argv);
void cmd_reboot(int argc, char **argv);

/* Application Functions */
void notepad_run(const char *filename);      /* notepad.c */
void calendar_run(int argc, char **argv);    /* calendar.c */
void calc_run(int argc, char **argv);        /* calc.c */
void ping_run(int argc, char **argv);        /* ping.c */

/* Port I/O Functions (inline assembly) */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

static inline void cli(void) {
    __asm__ volatile("cli");
}

static inline void sti(void) {
    __asm__ volatile("sti");
}

static inline void hlt(void) {
    __asm__ volatile("hlt");
}

/* External assembly functions */
extern void gdt_flush(uint32_t);
extern void idt_flush(uint32_t);
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);
extern void syscall_entry(void);

/* Kernel main and panic */
void kernel_main(void);
void kernel_panic(const char *message);

#endif /* SCOS_H */
