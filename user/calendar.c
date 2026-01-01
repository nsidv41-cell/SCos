/* ============================================================================
 * SCos 1.3.5 - Calendar Application
 * ============================================================================ */

#include "../include/scos.h"

static const char *month_names[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

static const int days_per_month[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* Calculate day of week for first day of month */
/* Using Zeller's congruence */
static int get_first_day_of_month(int year, int month) {
    int q = 1;  /* First day of month */
    int m = month;
    int k = year % 100;
    int j = year / 100;
    
    if (m < 3) {
        m += 12;
        if (month == 1 || month == 2) {
            k--;
            if (k < 0) {
                k = 99;
                j--;
            }
        }
    }
    
    int h = (q + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
    
    /* Convert from Zeller (0=Saturday) to standard (0=Sunday) */
    int day = ((h + 6) % 7);
    return day;
}

/* Check if leap year */
static int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/* Get days in month */
static int get_days_in_month(int year, int month) {
    int days = days_per_month[month - 1];
    if (month == 2 && is_leap_year(year)) {
        days++;
    }
    return days;
}

/* Draw calendar for a month */
static void draw_calendar(int year, int month, int highlight_day) {
    char buf[64];
    
    /* Header */
    vga_puts("\n");
    sprintf(buf, "    %s %d\n", month_names[month - 1], year);
    vga_put_color(buf, 0x0E);  /* Yellow */
    
    vga_put_color(" Su Mo Tu We Th Fr Sa\n", 0x0B);  /* Cyan */
    
    /* Get first day of month and number of days */
    int first_day = get_first_day_of_month(year, month);
    int num_days = get_days_in_month(year, month);
    
    /* Print leading spaces */
    for (int i = 0; i < first_day; i++) {
        vga_puts("   ");
    }
    
    /* Print days */
    for (int day = 1; day <= num_days; day++) {
        sprintf(buf, "%3d", day);
        
        if (day == highlight_day) {
            vga_put_color(buf, 0x70);  /* Inverted */
        } else {
            vga_puts(buf);
        }
        
        if ((first_day + day) % 7 == 0) {
            vga_puts("\n");
        }
    }
    
    /* Ensure newline at end */
    if ((first_day + num_days) % 7 != 0) {
        vga_puts("\n");
    }
}

/* Main calendar function */
void calendar_run(int argc, char **argv) {
    time_t_rtc now;
    rtc_read_time(&now);
    
    int year = now.year;
    int month = now.month;
    int today = now.day;
    int highlight = today;
    
    /* Parse arguments */
    if (argc >= 2) {
        month = atoi(argv[1]);
        highlight = 0;  /* Don't highlight if viewing different month */
        
        if (argc >= 3) {
            year = atoi(argv[2]);
        }
        
        /* Validate */
        if (month < 1 || month > 12) {
            vga_puts("Invalid month. Usage: calendar [month] [year]\n");
            return;
        }
        if (year < 1970 || year > 2100) {
            vga_puts("Invalid year. Use 1970-2100.\n");
            return;
        }
        
        /* Re-enable highlight if viewing current month */
        if (month == now.month && year == now.year) {
            highlight = today;
        }
    }
    
    /* Interactive mode if no arguments */
    if (argc < 2) {
        vga_clear();
        vga_put_color("Calendar - Use arrows to navigate, Q to quit\n", 0x0A);
        
        int running = 1;
        while (running) {
            draw_calendar(year, month, highlight);
            
            char buf[64];
            sprintf(buf, "\n  Today: %s %d, %d\n", 
                    month_names[now.month - 1], now.day, now.year);
            vga_put_color(buf, 0x08);
            
            vga_puts("\n  [<-] Prev Month  [->] Next Month  [Q] Quit\n");
            
            char c = keyboard_getchar();
            
            if (c == 'q' || c == 'Q') {
                running = 0;
            } else if (c == '\x1B') {
                char seq1 = keyboard_getchar();
                if (seq1 == '[') {
                    char seq2 = keyboard_getchar();
                    
                    if (seq2 == 'D') {  /* Left - previous month */
                        month--;
                        if (month < 1) {
                            month = 12;
                            year--;
                        }
                        highlight = (month == now.month && year == now.year) ? today : 0;
                    } else if (seq2 == 'C') {  /* Right - next month */
                        month++;
                        if (month > 12) {
                            month = 1;
                            year++;
                        }
                        highlight = (month == now.month && year == now.year) ? today : 0;
                    } else if (seq2 == 'A') {  /* Up - previous year */
                        year--;
                        highlight = (month == now.month && year == now.year) ? today : 0;
                    } else if (seq2 == 'B') {  /* Down - next year */
                        year++;
                        highlight = (month == now.month && year == now.year) ? today : 0;
                    }
                }
            }
            
            /* Clear and redraw */
            if (running) {
                vga_clear();
                vga_put_color("Calendar - Use arrows to navigate, Q to quit\n", 0x0A);
            }
        }
    } else {
        /* Non-interactive mode - just display the month */
        draw_calendar(year, month, highlight);
    }
}
