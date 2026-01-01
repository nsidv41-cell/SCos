/* ============================================================================
 * SCos 1.3.5 - Shell Commands
 * ============================================================================ */

#include "../include/scos.h"

/* help - Display available commands */
void cmd_help(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    vga_puts("\n");
    vga_put_color("SCos 1.3.5 - Available Commands\n", 0x0A);
    vga_puts("================================\n\n");
    
    vga_put_color("Navigation:\n", 0x0B);
    vga_puts("  ls [path]       List directory contents\n");
    vga_puts("  cd <path>       Change directory\n");
    vga_puts("  pwd             Print working directory\n");
    vga_puts("\n");
    
    vga_put_color("File Operations:\n", 0x0B);
    vga_puts("  cat <file>      Display file contents\n");
    vga_puts("  touch <file>    Create empty file\n");
    vga_puts("  mkdir <dir>     Create directory\n");
    vga_puts("  rm <path>       Remove file or directory\n");
    vga_puts("\n");
    
    vga_put_color("System:\n", 0x0B);
    vga_puts("  clear           Clear screen\n");
    vga_puts("  echo <text>     Display text\n");
    vga_puts("  date            Show current date/time\n");
    vga_puts("  whoami          Display current user\n");
    vga_puts("  version         Show system version\n");
    vga_puts("  uptime          Show system uptime\n");
    vga_puts("  sysinfo         Display system information\n");
    vga_puts("  ps              List running processes\n");
    vga_puts("  kill <pid>      Kill a process\n");
    vga_puts("  shutdown        Shutdown the system\n");
    vga_puts("  reboot          Reboot the system\n");
    vga_puts("\n");
    
    vga_put_color("Applications:\n", 0x0B);
    vga_puts("  notepad [file]  Text editor\n");
    vga_puts("  calc [expr]     Calculator\n");
    vga_puts("  calendar        Display calendar\n");
    vga_puts("  ping <host>     Ping a host\n");
    vga_puts("\n");
    
    vga_put_color("Shell:\n", 0x0B);
    vga_puts("  history         Show command history\n");
    vga_puts("  alias [n=cmd]   Create/list aliases\n");
    vga_puts("\n");
}

/* ls - List directory */
void cmd_ls(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : fs_getcwd();
    int long_format = 0;
    
    /* Check for -l flag */
    if (argc > 1 && strcmp(argv[1], "-l") == 0) {
        long_format = 1;
        path = (argc > 2) ? argv[2] : fs_getcwd();
    }
    
    if (!fs_exists(path)) {
        vga_puts("ls: cannot access '");
        vga_puts(path);
        vga_puts("': No such file or directory\n");
        return;
    }
    
    if (!fs_is_dir(path)) {
        /* It's a file, just show it */
        if (long_format) {
            uint32_t size;
            fs_stat(path, &size, NULL);
            char buf[128];
            sprintf(buf, "-rw-r--r--  1 user user  %8d  ", size);
            vga_puts(buf);
        }
        vga_puts(path);
        vga_puts("\n");
        return;
    }
    
    char listing[4096];
    if (fs_list(path, listing, sizeof(listing)) < 0) {
        vga_puts("ls: cannot read directory\n");
        return;
    }
    
    if (long_format) {
        vga_puts("total ");
        /* Count entries */
        int count = 0;
        char *p = listing;
        while (*p) {
            if (*p == '\n') count++;
            p++;
        }
        char buf[16];
        itoa(count, buf, 10);
        vga_puts(buf);
        vga_puts("\n");
    }
    
    /* Parse and display each entry */
    char *line = strtok(listing, "\n");
    while (line) {
        char full_path[FS_MAX_PATH];
        if (strcmp(path, "/") == 0) {
            sprintf(full_path, "/%s", line);
        } else {
            sprintf(full_path, "%s/%s", path, line);
        }
        
        uint32_t size;
        uint8_t type;
        fs_stat(full_path, &size, &type);
        
        if (long_format) {
            char buf[128];
            if (type == FS_TYPE_DIR) {
                sprintf(buf, "drwxr-xr-x  2 user user  %8d  ", 0);
            } else {
                sprintf(buf, "-rw-r--r--  1 user user  %8d  ", size);
            }
            vga_puts(buf);
        }
        
        if (type == FS_TYPE_DIR) {
            vga_put_color(line, 0x09);  /* Blue for directories */
        } else {
            vga_puts(line);
        }
        
        if (long_format) {
            vga_puts("\n");
        } else {
            vga_puts("  ");
        }
        
        line = strtok(NULL, "\n");
    }
    
    if (!long_format) {
        vga_puts("\n");
    }
}

/* cd - Change directory */
void cmd_cd(int argc, char **argv) {
    const char *path;
    
    if (argc < 2 || strcmp(argv[1], "~") == 0) {
        path = "/home/user";
    } else if (strcmp(argv[1], "-") == 0) {
        /* Go to previous directory - not implemented */
        path = "/home/user";
    } else {
        path = argv[1];
    }
    
    if (fs_chdir(path) < 0) {
        vga_puts("cd: ");
        vga_puts(path);
        vga_puts(": No such directory\n");
    }
}

/* pwd - Print working directory */
void cmd_pwd(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    vga_puts(fs_getcwd());
    vga_puts("\n");
}

/* cat - Display file contents */
void cmd_cat(int argc, char **argv) {
    if (argc < 2) {
        vga_puts("Usage: cat <filename>\n");
        return;
    }
    
    char content[FS_MAX_CONTENT];
    int len = fs_read(argv[1], content, sizeof(content));
    
    if (len < 0) {
        vga_puts("cat: ");
        vga_puts(argv[1]);
        vga_puts(": No such file or is a directory\n");
        return;
    }
    
    vga_puts(content);
    
    /* Ensure newline at end */
    if (len > 0 && content[len - 1] != '\n') {
        vga_puts("\n");
    }
}

/* clear - Clear screen */
void cmd_clear(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    vga_clear();
}

/* echo - Display text */
void cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) {
            vga_puts(" ");
        }
        
        /* Handle special variables */
        if (argv[i][0] == '$') {
            if (strcmp(argv[i], "$HOME") == 0) {
                vga_puts("/home/user");
            } else if (strcmp(argv[i], "$USER") == 0) {
                vga_puts("user");
            } else if (strcmp(argv[i], "$PWD") == 0) {
                vga_puts(fs_getcwd());
            } else if (strcmp(argv[i], "$?") == 0) {
                vga_puts("0");
            } else {
                vga_puts(argv[i]);
            }
        } else {
            vga_puts(argv[i]);
        }
    }
    vga_puts("\n");
}

/* date - Display date/time */
void cmd_date(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    time_t_rtc time;
    rtc_read_time(&time);
    
    char buf[64];
    sprintf(buf, "%s %s %d %02d:%02d:%02d UTC %d\n",
            rtc_get_weekday_name(time.weekday % 7),
            rtc_get_month_name(time.month),
            time.day,
            time.hour,
            time.minute,
            time.second,
            time.year);
    vga_puts(buf);
}

/* mkdir - Create directory */
void cmd_mkdir(int argc, char **argv) {
    if (argc < 2) {
        vga_puts("Usage: mkdir <directory>\n");
        return;
    }
    
    if (fs_mkdir(argv[1]) < 0) {
        vga_puts("mkdir: cannot create directory '");
        vga_puts(argv[1]);
        vga_puts("': File exists or invalid path\n");
    }
}

/* touch - Create file */
void cmd_touch(int argc, char **argv) {
    if (argc < 2) {
        vga_puts("Usage: touch <filename>\n");
        return;
    }
    
    if (fs_touch(argv[1]) < 0) {
        vga_puts("touch: cannot touch '");
        vga_puts(argv[1]);
        vga_puts("'\n");
    }
}

/* rm - Remove file or directory */
void cmd_rm(int argc, char **argv) {
    if (argc < 2) {
        vga_puts("Usage: rm <path>\n");
        return;
    }
    
    int recursive = 0;
    const char *path = argv[1];
    
    if (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "-rf") == 0) {
        recursive = 1;
        if (argc < 3) {
            vga_puts("Usage: rm -r <path>\n");
            return;
        }
        path = argv[2];
    }
    
    (void)recursive;  /* Not fully implemented */
    
    if (fs_delete(path) < 0) {
        vga_puts("rm: cannot remove '");
        vga_puts(path);
        vga_puts("': No such file or directory not empty\n");
    }
}

/* whoami - Display current user */
void cmd_whoami(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    vga_puts("user\n");
}

/* version - Display system version */
void cmd_version(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    vga_puts("\n");
    vga_put_color("SCos 1.3.5", 0x0A);
    vga_puts(" \"Terminal\"\n");
    vga_puts("32-bit x86 Operating System\n");
    vga_puts("Build Date: " __DATE__ " " __TIME__ "\n");
    vga_puts("\n");
}

/* history - Show command history */
void cmd_history(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    int start = 0;
    extern char history[CMD_HISTORY_SIZE][CMD_MAX_LENGTH];
    extern int history_count;
    
    if (history_count > CMD_HISTORY_SIZE) {
        start = history_count - CMD_HISTORY_SIZE;
    }
    
    for (int i = start; i < history_count; i++) {
        char buf[16];
        sprintf(buf, "%4d  ", i + 1);
        vga_puts(buf);
        vga_puts(history[i % CMD_HISTORY_SIZE]);
        vga_puts("\n");
    }
}

/* alias - Create or list aliases */
void cmd_alias(int argc, char **argv) {
    extern alias_t aliases[MAX_ALIASES];
    
    if (argc < 2) {
        /* List all aliases */
        for (int i = 0; i < MAX_ALIASES; i++) {
            if (aliases[i].in_use) {
                vga_puts("alias ");
                vga_puts(aliases[i].name);
                vga_puts("='");
                vga_puts(aliases[i].command);
                vga_puts("'\n");
            }
        }
        return;
    }
    
    /* Parse alias definition */
    char *eq = strchr(argv[1], '=');
    if (eq) {
        *eq = '\0';
        shell_set_alias(argv[1], eq + 1);
    } else {
        /* Show specific alias */
        const char *cmd = shell_get_alias(argv[1]);
        if (cmd) {
            vga_puts("alias ");
            vga_puts(argv[1]);
            vga_puts("='");
            vga_puts(cmd);
            vga_puts("'\n");
        } else {
            vga_puts("alias: ");
            vga_puts(argv[1]);
            vga_puts(": not found\n");
        }
    }
}

/* ps - List processes */
void cmd_ps(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    process_list();
}

/* kill - Kill process */
void cmd_kill(int argc, char **argv) {
    if (argc < 2) {
        vga_puts("Usage: kill <pid>\n");
        return;
    }
    
    int pid = atoi(argv[1]);
    if (pid <= 0) {
        vga_puts("kill: invalid pid\n");
        return;
    }
    
    process_kill(pid);
}

/* uptime - Show system uptime */
void cmd_uptime(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    uint32_t uptime = timer_get_uptime();
    uint32_t hours = uptime / 3600;
    uint32_t minutes = (uptime % 3600) / 60;
    uint32_t seconds = uptime % 60;
    
    char buf[64];
    sprintf(buf, "up %d:%02d:%02d\n", hours, minutes, seconds);
    vga_puts(buf);
}

/* sysinfo - Display system information */
void cmd_sysinfo(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    vga_puts("\n");
    vga_put_color("System Information\n", 0x0A);
    vga_puts("==================\n\n");
    
    vga_puts("OS:           SCos 1.3.5 \"Terminal\"\n");
    vga_puts("Kernel:       SCos Kernel (monolithic)\n");
    vga_puts("Architecture: x86 (IA-32)\n");
    vga_puts("Mode:         32-bit Protected Mode\n");
    
    char buf[64];
    sprintf(buf, "Memory Free:  %d KB\n", memory_get_free() / 1024);
    vga_puts(buf);
    sprintf(buf, "Memory Used:  %d KB\n", memory_get_used() / 1024);
    vga_puts(buf);
    sprintf(buf, "Processes:    %d\n", process_count());
    vga_puts(buf);
    
    uint32_t uptime = timer_get_uptime();
    sprintf(buf, "Uptime:       %d seconds\n", uptime);
    vga_puts(buf);
    
    time_t_rtc time;
    rtc_read_time(&time);
    sprintf(buf, "System Time:  %02d:%02d:%02d\n", time.hour, time.minute, time.second);
    vga_puts(buf);
    sprintf(buf, "System Date:  %04d-%02d-%02d\n", time.year, time.month, time.day);
    vga_puts(buf);
    
    vga_puts("\n");
}

/* shutdown - Shutdown system */
void cmd_shutdown(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    vga_puts("System is going down for shutdown NOW!\n");
    timer_sleep(500);
    
    /* Use syscall for shutdown */
    __asm__ volatile (
        "movl %0, %%eax\n"
        "int $0x80\n"
        : : "i"(SYS_SHUTDOWN)
    );
}

/* reboot - Reboot system */
void cmd_reboot(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    vga_puts("System is going down for reboot NOW!\n");
    timer_sleep(500);
    
    /* Use syscall for reboot */
    __asm__ volatile (
        "movl %0, %%eax\n"
        "int $0x80\n"
        : : "i"(SYS_REBOOT)
    );
}
