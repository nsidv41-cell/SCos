/* ============================================================================
 * SCos 1.3.5 - Shell
 * ============================================================================ */

#include "../include/scos.h"

/* Command history */
static char history[CMD_HISTORY_SIZE][CMD_MAX_LENGTH];
static int history_count = 0;
static int history_index = 0;

/* Aliases */
static alias_t aliases[MAX_ALIASES];

/* Current command line */
static char cmdline[CMD_MAX_LENGTH];
static int cmdline_pos = 0;
static int cursor_pos = 0;

/* Command structure */
typedef struct {
    const char *name;
    void (*handler)(int argc, char **argv);
    const char *description;
} command_t;

/* Command table */
static command_t commands[] = {
    {"help",     cmd_help,     "Display available commands"},
    {"ls",       cmd_ls,       "List directory contents"},
    {"cd",       cmd_cd,       "Change directory"},
    {"pwd",      cmd_pwd,      "Print working directory"},
    {"cat",      cmd_cat,      "Display file contents"},
    {"clear",    cmd_clear,    "Clear screen"},
    {"echo",     cmd_echo,     "Display text"},
    {"date",     cmd_date,     "Display current date/time"},
    {"mkdir",    cmd_mkdir,    "Create directory"},
    {"touch",    cmd_touch,    "Create empty file"},
    {"rm",       cmd_rm,       "Remove file or directory"},
    {"whoami",   cmd_whoami,   "Display current user"},
    {"version",  cmd_version,  "Display system version"},
    {"history",  cmd_history,  "Show command history"},
    {"alias",    cmd_alias,    "Create command alias"},
    {"ps",       cmd_ps,       "List processes"},
    {"kill",     cmd_kill,     "Kill a process"},
    {"uptime",   cmd_uptime,   "Show system uptime"},
    {"sysinfo",  cmd_sysinfo,  "Display system information"},
    {"shutdown", cmd_shutdown, "Shutdown system"},
    {"reboot",   cmd_reboot,   "Reboot system"},
    {"notepad",  NULL,         "Text editor"},
    {"calendar", NULL,         "Display calendar"},
    {"calc",     NULL,         "Calculator"},
    {"ping",     NULL,         "Ping a host"},
    {NULL, NULL, NULL}
};

/* Initialize shell */
void shell_init(void) {
    memset(history, 0, sizeof(history));
    memset(aliases, 0, sizeof(aliases));
    history_count = 0;
    history_index = 0;
    
    /* Set up default aliases */
    shell_set_alias("ll", "ls -l");
    shell_set_alias("clr", "clear");
    shell_set_alias("h", "history");
}

/* Print shell prompt */
void shell_print_prompt(void) {
    vga_put_color("user", 0x0A);
    vga_put_color("@", 0x07);
    vga_put_color("scos", 0x0A);
    vga_put_color(":", 0x07);
    vga_put_color(fs_getcwd(), 0x09);
    vga_put_color("$ ", 0x07);
}

/* Add command to history */
void shell_add_history(const char *cmd) {
    if (strlen(cmd) == 0) {
        return;
    }
    
    /* Don't add duplicates */
    if (history_count > 0 && strcmp(history[(history_count - 1) % CMD_HISTORY_SIZE], cmd) == 0) {
        return;
    }
    
    strcpy(history[history_count % CMD_HISTORY_SIZE], cmd);
    history_count++;
    history_index = history_count;
}

/* Get history entry */
const char *shell_get_history(int index) {
    if (index < 0 || index >= history_count) {
        return NULL;
    }
    return history[index % CMD_HISTORY_SIZE];
}

/* Set alias */
void shell_set_alias(const char *name, const char *command) {
    /* Check if alias exists */
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (aliases[i].in_use && strcmp(aliases[i].name, name) == 0) {
            strncpy(aliases[i].command, command, ALIAS_CMD_LEN - 1);
            return;
        }
    }
    
    /* Add new alias */
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (!aliases[i].in_use) {
            aliases[i].in_use = 1;
            strncpy(aliases[i].name, name, ALIAS_NAME_LEN - 1);
            strncpy(aliases[i].command, command, ALIAS_CMD_LEN - 1);
            return;
        }
    }
}

/* Get alias */
const char *shell_get_alias(const char *name) {
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (aliases[i].in_use && strcmp(aliases[i].name, name) == 0) {
            return aliases[i].command;
        }
    }
    return NULL;
}

/* Parse command line into argc/argv */
static int parse_cmdline(char *line, char **argv) {
    int argc = 0;
    char *p = line;
    
    while (*p && argc < MAX_ARGS - 1) {
        /* Skip whitespace */
        while (*p && isspace(*p)) {
            p++;
        }
        
        if (*p == '\0') {
            break;
        }
        
        /* Handle quoted strings */
        if (*p == '"') {
            p++;
            argv[argc++] = p;
            while (*p && *p != '"') {
                p++;
            }
            if (*p) {
                *p++ = '\0';
            }
        } else {
            argv[argc++] = p;
            while (*p && !isspace(*p)) {
                p++;
            }
            if (*p) {
                *p++ = '\0';
            }
        }
    }
    
    argv[argc] = NULL;
    return argc;
}

/* Execute command line */
void shell_execute(const char *line) {
    char cmdline_copy[CMD_MAX_LENGTH];
    strncpy(cmdline_copy, line, CMD_MAX_LENGTH - 1);
    cmdline_copy[CMD_MAX_LENGTH - 1] = '\0';
    
    /* Expand aliases */
    char *argv[MAX_ARGS];
    int argc = parse_cmdline(cmdline_copy, argv);
    
    if (argc == 0) {
        return;
    }
    
    const char *alias = shell_get_alias(argv[0]);
    if (alias) {
        char expanded[CMD_MAX_LENGTH];
        strcpy(expanded, alias);
        
        /* Append remaining arguments */
        for (int i = 1; i < argc; i++) {
            strcat(expanded, " ");
            strcat(expanded, argv[i]);
        }
        
        strncpy(cmdline_copy, expanded, CMD_MAX_LENGTH - 1);
        argc = parse_cmdline(cmdline_copy, argv);
    }
    
    /* Look up command */
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            /* Special handling for apps */
            if (strcmp(argv[0], "notepad") == 0) {
                notepad_run(argc > 1 ? argv[1] : NULL);
                return;
            } else if (strcmp(argv[0], "calendar") == 0) {
                calendar_run(argc, argv);
                return;
            } else if (strcmp(argv[0], "calc") == 0) {
                calc_run(argc, argv);
                return;
            } else if (strcmp(argv[0], "ping") == 0) {
                ping_run(argc, argv);
                return;
            }
            
            if (commands[i].handler) {
                commands[i].handler(argc, argv);
            }
            return;
        }
    }
    
    /* Try to run as file */
    if (fs_exists(argv[0])) {
        vga_puts("Cannot execute: not a program\n");
    } else {
        vga_puts(argv[0]);
        vga_puts(": command not found\n");
    }
}

/* Clear current line display */
static void clear_line_display(void) {
    int x, y;
    vga_get_cursor(&x, &y);
    
    /* Move to start of input */
    vga_set_cursor(0, y);
    vga_clear_line(y);
    shell_print_prompt();
}

/* Redraw current command line */
static void redraw_cmdline(void) {
    clear_line_display();
    vga_puts(cmdline);
    
    /* Position cursor correctly */
    int x, y;
    vga_get_cursor(&x, &y);
    x = cursor_pos + strlen("user@scos:") + strlen(fs_getcwd()) + 2;
    vga_set_cursor(x, y);
}

/* Main shell loop */
void shell_run(void) {
    char c;
    int escape_seq = 0;
    char escape_buf[8];
    int escape_idx = 0;
    
    cmdline[0] = '\0';
    cmdline_pos = 0;
    cursor_pos = 0;
    
    shell_print_prompt();
    
    while (1) {
        c = keyboard_getchar();
        
        /* Handle escape sequences (arrow keys, etc.) */
        if (c == '\x1B') {
            escape_seq = 1;
            escape_idx = 0;
            continue;
        }
        
        if (escape_seq) {
            escape_buf[escape_idx++] = c;
            
            if (escape_idx >= 2) {
                escape_seq = 0;
                
                if (escape_buf[0] == '[') {
                    switch (escape_buf[1]) {
                        case 'A':  /* Up arrow - history previous */
                            if (history_index > 0) {
                                history_index--;
                                const char *hist = shell_get_history(history_index);
                                if (hist) {
                                    strcpy(cmdline, hist);
                                    cmdline_pos = strlen(cmdline);
                                    cursor_pos = cmdline_pos;
                                    redraw_cmdline();
                                }
                            }
                            break;
                            
                        case 'B':  /* Down arrow - history next */
                            if (history_index < history_count - 1) {
                                history_index++;
                                const char *hist = shell_get_history(history_index);
                                if (hist) {
                                    strcpy(cmdline, hist);
                                    cmdline_pos = strlen(cmdline);
                                    cursor_pos = cmdline_pos;
                                    redraw_cmdline();
                                }
                            } else {
                                history_index = history_count;
                                cmdline[0] = '\0';
                                cmdline_pos = 0;
                                cursor_pos = 0;
                                redraw_cmdline();
                            }
                            break;
                            
                        case 'C':  /* Right arrow */
                            if (cursor_pos < cmdline_pos) {
                                cursor_pos++;
                                int x, y;
                                vga_get_cursor(&x, &y);
                                vga_set_cursor(x + 1, y);
                            }
                            break;
                            
                        case 'D':  /* Left arrow */
                            if (cursor_pos > 0) {
                                cursor_pos--;
                                int x, y;
                                vga_get_cursor(&x, &y);
                                vga_set_cursor(x - 1, y);
                            }
                            break;
                            
                        case 'H':  /* Home */
                            cursor_pos = 0;
                            redraw_cmdline();
                            break;
                            
                        case 'F':  /* End */
                            cursor_pos = cmdline_pos;
                            redraw_cmdline();
                            break;
                    }
                }
            }
            continue;
        }
        
        /* Handle regular keys */
        if (c == '\n' || c == '\r') {
            vga_putchar('\n');
            
            if (cmdline_pos > 0) {
                shell_add_history(cmdline);
                shell_execute(cmdline);
            }
            
            cmdline[0] = '\0';
            cmdline_pos = 0;
            cursor_pos = 0;
            history_index = history_count;
            
            shell_print_prompt();
        }
        else if (c == '\b' || c == 127) {
            if (cursor_pos > 0) {
                /* Remove character before cursor */
                memmove(cmdline + cursor_pos - 1, cmdline + cursor_pos, 
                        cmdline_pos - cursor_pos + 1);
                cursor_pos--;
                cmdline_pos--;
                redraw_cmdline();
            }
        }
        else if (c == '\x7F') {  /* Delete key */
            if (cursor_pos < cmdline_pos) {
                memmove(cmdline + cursor_pos, cmdline + cursor_pos + 1,
                        cmdline_pos - cursor_pos);
                cmdline_pos--;
                redraw_cmdline();
            }
        }
        else if (c == '\t') {
            /* Tab completion */
            if (cmdline_pos > 0) {
                /* Find the word being completed */
                int word_start = cursor_pos;
                while (word_start > 0 && cmdline[word_start - 1] != ' ') {
                    word_start--;
                }
                
                char prefix[CMD_MAX_LENGTH];
                int prefix_len = cursor_pos - word_start;
                strncpy(prefix, cmdline + word_start, prefix_len);
                prefix[prefix_len] = '\0';
                
                /* First try command completion if at start */
                if (word_start == 0) {
                    for (int i = 0; commands[i].name != NULL; i++) {
                        if (strncmp(commands[i].name, prefix, prefix_len) == 0) {
                            /* Found match - complete it */
                            const char *rest = commands[i].name + prefix_len;
                            int rest_len = strlen(rest);
                            
                            if (cmdline_pos + rest_len < CMD_MAX_LENGTH - 1) {
                                memmove(cmdline + cursor_pos + rest_len,
                                        cmdline + cursor_pos,
                                        cmdline_pos - cursor_pos + 1);
                                memcpy(cmdline + cursor_pos, rest, rest_len);
                                cmdline_pos += rest_len;
                                cursor_pos += rest_len;
                                redraw_cmdline();
                            }
                            break;
                        }
                    }
                } else {
                    /* Try file completion */
                    char path[FS_MAX_PATH];
                    char dir[FS_MAX_PATH];
                    char file_prefix[FS_MAX_NAME];
                    
                    /* Parse the path */
                    if (strchr(prefix, '/')) {
                        fs_get_parent_path(prefix, dir);
                        fs_get_basename(prefix, file_prefix);
                    } else {
                        strcpy(dir, fs_getcwd());
                        strcpy(file_prefix, prefix);
                    }
                    
                    /* List directory and find matches */
                    char listing[4096];
                    if (fs_list(dir, listing, sizeof(listing)) >= 0) {
                        char *line = strtok(listing, "\n");
                        while (line) {
                            if (strncmp(line, file_prefix, strlen(file_prefix)) == 0) {
                                /* Found match */
                                const char *rest = line + strlen(file_prefix);
                                int rest_len = strlen(rest);
                                
                                if (cmdline_pos + rest_len < CMD_MAX_LENGTH - 1) {
                                    memmove(cmdline + cursor_pos + rest_len,
                                            cmdline + cursor_pos,
                                            cmdline_pos - cursor_pos + 1);
                                    memcpy(cmdline + cursor_pos, rest, rest_len);
                                    cmdline_pos += rest_len;
                                    cursor_pos += rest_len;
                                    redraw_cmdline();
                                }
                                break;
                            }
                            line = strtok(NULL, "\n");
                        }
                    }
                }
            }
        }
        else if (c == 3) {  /* Ctrl+C */
            vga_puts("^C\n");
            cmdline[0] = '\0';
            cmdline_pos = 0;
            cursor_pos = 0;
            shell_print_prompt();
        }
        else if (c == 4) {  /* Ctrl+D */
            if (cmdline_pos == 0) {
                vga_puts("logout\n");
                /* In a real system, this would exit the shell */
            }
        }
        else if (c == 12) {  /* Ctrl+L - clear screen */
            vga_clear();
            shell_print_prompt();
            vga_puts(cmdline);
        }
        else if (c == 21) {  /* Ctrl+U - clear line */
            cmdline[0] = '\0';
            cmdline_pos = 0;
            cursor_pos = 0;
            redraw_cmdline();
        }
        else if (c == 23) {  /* Ctrl+W - delete word */
            if (cursor_pos > 0) {
                int word_start = cursor_pos;
                while (word_start > 0 && cmdline[word_start - 1] == ' ') {
                    word_start--;
                }
                while (word_start > 0 && cmdline[word_start - 1] != ' ') {
                    word_start--;
                }
                
                memmove(cmdline + word_start, cmdline + cursor_pos,
                        cmdline_pos - cursor_pos + 1);
                cmdline_pos -= (cursor_pos - word_start);
                cursor_pos = word_start;
                redraw_cmdline();
            }
        }
        else if (c >= ' ' && c <= '~') {
            /* Printable character */
            if (cmdline_pos < CMD_MAX_LENGTH - 1) {
                if (cursor_pos < cmdline_pos) {
                    /* Insert in middle */
                    memmove(cmdline + cursor_pos + 1, cmdline + cursor_pos,
                            cmdline_pos - cursor_pos + 1);
                }
                cmdline[cursor_pos] = c;
                cmdline_pos++;
                cursor_pos++;
                cmdline[cmdline_pos] = '\0';
                redraw_cmdline();
            }
        }
    }
}
