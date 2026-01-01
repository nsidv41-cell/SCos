/* ============================================================================
 * SCos 1.3.5 - Notepad (Terminal Text Editor)
 * ============================================================================ */

#include "../include/scos.h"

#define NOTEPAD_MAX_LINES   100
#define NOTEPAD_MAX_COLS    256
#define NOTEPAD_VIEW_LINES  (VGA_HEIGHT - 2)  /* Reserve for status bar */

/* Editor state */
static char lines[NOTEPAD_MAX_LINES][NOTEPAD_MAX_COLS];
static int line_count = 0;
static int cursor_row = 0;
static int cursor_col = 0;
static int scroll_offset = 0;
static int modified = 0;
static char filename[FS_MAX_PATH] = "";

/* Clear editor state */
static void notepad_clear(void) {
    for (int i = 0; i < NOTEPAD_MAX_LINES; i++) {
        lines[i][0] = '\0';
    }
    line_count = 1;
    cursor_row = 0;
    cursor_col = 0;
    scroll_offset = 0;
    modified = 0;
}

/* Draw status bar */
static void notepad_draw_status(void) {
    vga_set_cursor(0, VGA_HEIGHT - 1);
    
    /* Draw status bar background */
    for (int i = 0; i < VGA_WIDTH; i++) {
        vga_putchar_at(' ', i, VGA_HEIGHT - 1, 0x70);  /* White on gray */
    }
    
    /* Status text */
    char status[VGA_WIDTH];
    sprintf(status, " %s%s  Line %d/%d  Col %d  %s",
            filename[0] ? filename : "[New File]",
            modified ? " *" : "",
            cursor_row + 1,
            line_count,
            cursor_col + 1,
            "Ctrl+S:Save Ctrl+Q:Quit Ctrl+G:Goto");
    
    for (int i = 0; status[i] && i < VGA_WIDTH - 1; i++) {
        vga_putchar_at(status[i], i, VGA_HEIGHT - 1, 0x70);
    }
}

/* Draw editor content */
static void notepad_draw(void) {
    for (int y = 0; y < NOTEPAD_VIEW_LINES; y++) {
        int line_idx = y + scroll_offset;
        
        vga_set_cursor(0, y);
        
        if (line_idx < line_count) {
            /* Draw line number */
            char num[6];
            sprintf(num, "%3d ", line_idx + 1);
            vga_put_color(num, 0x08);  /* Gray */
            
            /* Draw line content */
            for (int x = 0; x < VGA_WIDTH - 4 && lines[line_idx][x]; x++) {
                vga_putchar(lines[line_idx][x]);
            }
        } else {
            vga_put_color("  ~ ", 0x08);  /* Gray tilde for empty lines */
        }
        
        /* Clear rest of line */
        int x, dummy;
        vga_get_cursor(&x, &dummy);
        while (x < VGA_WIDTH) {
            vga_putchar(' ');
            x++;
        }
    }
    
    notepad_draw_status();
    
    /* Position cursor */
    int screen_row = cursor_row - scroll_offset;
    if (screen_row >= 0 && screen_row < NOTEPAD_VIEW_LINES) {
        vga_set_cursor(cursor_col + 4, screen_row);  /* +4 for line numbers */
    }
}

/* Ensure cursor is visible */
static void notepad_ensure_visible(void) {
    if (cursor_row < scroll_offset) {
        scroll_offset = cursor_row;
    } else if (cursor_row >= scroll_offset + NOTEPAD_VIEW_LINES) {
        scroll_offset = cursor_row - NOTEPAD_VIEW_LINES + 1;
    }
}

/* Insert character at cursor */
static void notepad_insert_char(char c) {
    if (cursor_col < NOTEPAD_MAX_COLS - 1) {
        int len = strlen(lines[cursor_row]);
        
        /* Shift characters right */
        for (int i = len + 1; i > cursor_col; i--) {
            lines[cursor_row][i] = lines[cursor_row][i - 1];
        }
        
        lines[cursor_row][cursor_col] = c;
        cursor_col++;
        modified = 1;
    }
}

/* Delete character before cursor */
static void notepad_backspace(void) {
    if (cursor_col > 0) {
        cursor_col--;
        int len = strlen(lines[cursor_row]);
        
        for (int i = cursor_col; i < len; i++) {
            lines[cursor_row][i] = lines[cursor_row][i + 1];
        }
        
        modified = 1;
    } else if (cursor_row > 0) {
        /* Join with previous line */
        int prev_len = strlen(lines[cursor_row - 1]);
        
        if (prev_len + strlen(lines[cursor_row]) < NOTEPAD_MAX_COLS - 1) {
            strcat(lines[cursor_row - 1], lines[cursor_row]);
            
            /* Shift lines up */
            for (int i = cursor_row; i < line_count - 1; i++) {
                strcpy(lines[i], lines[i + 1]);
            }
            
            line_count--;
            cursor_row--;
            cursor_col = prev_len;
            modified = 1;
        }
    }
}

/* Delete character at cursor */
static void notepad_delete(void) {
    int len = strlen(lines[cursor_row]);
    
    if (cursor_col < len) {
        for (int i = cursor_col; i < len; i++) {
            lines[cursor_row][i] = lines[cursor_row][i + 1];
        }
        modified = 1;
    } else if (cursor_row < line_count - 1) {
        /* Join with next line */
        if (len + strlen(lines[cursor_row + 1]) < NOTEPAD_MAX_COLS - 1) {
            strcat(lines[cursor_row], lines[cursor_row + 1]);
            
            for (int i = cursor_row + 1; i < line_count - 1; i++) {
                strcpy(lines[i], lines[i + 1]);
            }
            
            line_count--;
            modified = 1;
        }
    }
}

/* Insert new line */
static void notepad_newline(void) {
    if (line_count < NOTEPAD_MAX_LINES) {
        /* Shift lines down */
        for (int i = line_count; i > cursor_row + 1; i--) {
            strcpy(lines[i], lines[i - 1]);
        }
        
        /* Split current line */
        strcpy(lines[cursor_row + 1], lines[cursor_row] + cursor_col);
        lines[cursor_row][cursor_col] = '\0';
        
        line_count++;
        cursor_row++;
        cursor_col = 0;
        modified = 1;
    }
}

/* Load file */
static int notepad_load(const char *path) {
    notepad_clear();
    
    char content[FS_MAX_CONTENT];
    int len = fs_read(path, content, sizeof(content));
    
    if (len < 0) {
        return -1;
    }
    
    strcpy(filename, path);
    
    /* Parse content into lines */
    line_count = 0;
    int col = 0;
    
    for (int i = 0; i < len && line_count < NOTEPAD_MAX_LINES; i++) {
        if (content[i] == '\n' || content[i] == '\r') {
            lines[line_count][col] = '\0';
            line_count++;
            col = 0;
            
            /* Handle \r\n */
            if (content[i] == '\r' && content[i + 1] == '\n') {
                i++;
            }
        } else if (col < NOTEPAD_MAX_COLS - 1) {
            lines[line_count][col++] = content[i];
        }
    }
    
    /* Handle last line without newline */
    if (col > 0 || line_count == 0) {
        lines[line_count][col] = '\0';
        line_count++;
    }
    
    modified = 0;
    return 0;
}

/* Save file */
static int notepad_save(const char *path) {
    char content[FS_MAX_CONTENT];
    int offset = 0;
    
    for (int i = 0; i < line_count; i++) {
        int len = strlen(lines[i]);
        
        if (offset + len + 1 < FS_MAX_CONTENT) {
            memcpy(content + offset, lines[i], len);
            offset += len;
            content[offset++] = '\n';
        }
    }
    
    if (fs_write(path, content, offset) < 0) {
        return -1;
    }
    
    strcpy(filename, path);
    modified = 0;
    return 0;
}

/* Prompt for filename */
static int notepad_prompt_filename(char *buffer, const char *prompt) {
    /* Draw prompt at bottom */
    vga_set_cursor(0, VGA_HEIGHT - 1);
    for (int i = 0; i < VGA_WIDTH; i++) {
        vga_putchar_at(' ', i, VGA_HEIGHT - 1, 0x70);
    }
    
    int pos = 0;
    for (int i = 0; prompt[i]; i++) {
        vga_putchar_at(prompt[i], pos++, VGA_HEIGHT - 1, 0x70);
    }
    
    int input_start = pos;
    int input_pos = 0;
    buffer[0] = '\0';
    
    vga_set_cursor(input_start, VGA_HEIGHT - 1);
    
    while (1) {
        char c = keyboard_getchar();
        
        if (c == '\x1B') {  /* Escape */
            return -1;
        } else if (c == '\n' || c == '\r') {
            return (input_pos > 0) ? 0 : -1;
        } else if (c == '\b' && input_pos > 0) {
            input_pos--;
            buffer[input_pos] = '\0';
            vga_putchar_at(' ', input_start + input_pos, VGA_HEIGHT - 1, 0x70);
            vga_set_cursor(input_start + input_pos, VGA_HEIGHT - 1);
        } else if (c >= ' ' && c <= '~' && input_pos < FS_MAX_PATH - 1) {
            buffer[input_pos] = c;
            vga_putchar_at(c, input_start + input_pos, VGA_HEIGHT - 1, 0x70);
            input_pos++;
            buffer[input_pos] = '\0';
            vga_set_cursor(input_start + input_pos, VGA_HEIGHT - 1);
        }
    }
}

/* Goto line prompt */
static void notepad_goto_line(void) {
    char buffer[16];
    if (notepad_prompt_filename(buffer, "Goto line: ") == 0) {
        int line = atoi(buffer);
        if (line >= 1 && line <= line_count) {
            cursor_row = line - 1;
            cursor_col = 0;
            notepad_ensure_visible();
        }
    }
}

/* Main editor loop */
void notepad_run(const char *file) {
    vga_clear();
    notepad_clear();
    
    if (file != NULL) {
        if (notepad_load(file) < 0) {
            /* New file */
            strcpy(filename, file);
        }
    }
    
    notepad_draw();
    
    int running = 1;
    while (running) {
        char c = keyboard_getchar();
        
        /* Handle escape sequences */
        if (c == '\x1B') {
            char seq1 = keyboard_getchar();
            if (seq1 == '[') {
                char seq2 = keyboard_getchar();
                
                switch (seq2) {
                    case 'A':  /* Up */
                        if (cursor_row > 0) {
                            cursor_row--;
                            if (cursor_col > (int)strlen(lines[cursor_row])) {
                                cursor_col = strlen(lines[cursor_row]);
                            }
                            notepad_ensure_visible();
                        }
                        break;
                    case 'B':  /* Down */
                        if (cursor_row < line_count - 1) {
                            cursor_row++;
                            if (cursor_col > (int)strlen(lines[cursor_row])) {
                                cursor_col = strlen(lines[cursor_row]);
                            }
                            notepad_ensure_visible();
                        }
                        break;
                    case 'C':  /* Right */
                        if (cursor_col < (int)strlen(lines[cursor_row])) {
                            cursor_col++;
                        } else if (cursor_row < line_count - 1) {
                            cursor_row++;
                            cursor_col = 0;
                            notepad_ensure_visible();
                        }
                        break;
                    case 'D':  /* Left */
                        if (cursor_col > 0) {
                            cursor_col--;
                        } else if (cursor_row > 0) {
                            cursor_row--;
                            cursor_col = strlen(lines[cursor_row]);
                            notepad_ensure_visible();
                        }
                        break;
                    case 'H':  /* Home */
                        cursor_col = 0;
                        break;
                    case 'F':  /* End */
                        cursor_col = strlen(lines[cursor_row]);
                        break;
                    case '5':  /* Page Up */
                        keyboard_getchar();  /* Consume ~ */
                        cursor_row -= NOTEPAD_VIEW_LINES;
                        if (cursor_row < 0) cursor_row = 0;
                        notepad_ensure_visible();
                        break;
                    case '6':  /* Page Down */
                        keyboard_getchar();  /* Consume ~ */
                        cursor_row += NOTEPAD_VIEW_LINES;
                        if (cursor_row >= line_count) cursor_row = line_count - 1;
                        notepad_ensure_visible();
                        break;
                    case '3':  /* Delete */
                        keyboard_getchar();  /* Consume ~ */
                        notepad_delete();
                        break;
                }
            }
        }
        /* Ctrl+S - Save */
        else if (c == 19) {
            if (filename[0] == '\0') {
                char new_name[FS_MAX_PATH];
                if (notepad_prompt_filename(new_name, "Save as: ") == 0) {
                    if (notepad_save(new_name) < 0) {
                        /* Show error */
                    }
                }
            } else {
                notepad_save(filename);
            }
        }
        /* Ctrl+Q - Quit */
        else if (c == 17) {
            if (modified) {
                char response[8];
                if (notepad_prompt_filename(response, "Save changes? (y/n): ") == 0) {
                    if (response[0] == 'y' || response[0] == 'Y') {
                        if (filename[0] == '\0') {
                            char new_name[FS_MAX_PATH];
                            if (notepad_prompt_filename(new_name, "Save as: ") == 0) {
                                notepad_save(new_name);
                            }
                        } else {
                            notepad_save(filename);
                        }
                    }
                }
            }
            running = 0;
        }
        /* Ctrl+G - Goto line */
        else if (c == 7) {
            notepad_goto_line();
        }
        /* Enter */
        else if (c == '\n' || c == '\r') {
            notepad_newline();
            notepad_ensure_visible();
        }
        /* Backspace */
        else if (c == '\b' || c == 127) {
            notepad_backspace();
        }
        /* Tab */
        else if (c == '\t') {
            for (int i = 0; i < 4; i++) {
                notepad_insert_char(' ');
            }
        }
        /* Printable character */
        else if (c >= ' ' && c <= '~') {
            notepad_insert_char(c);
        }
        
        notepad_draw();
    }
    
    vga_clear();
    vga_set_attr(SCOS_ATTR);
}
