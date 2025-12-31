/* ============================================================================
 * SCos 1.3.5 - Real Time Clock Driver
 * ============================================================================ */

#include "../include/scos.h"

#define RTC_ADDRESS     0x70
#define RTC_DATA        0x71

#define RTC_SECONDS     0x00
#define RTC_MINUTES     0x02
#define RTC_HOURS       0x04
#define RTC_WEEKDAY     0x06
#define RTC_DAY         0x07
#define RTC_MONTH       0x08
#define RTC_YEAR        0x09
#define RTC_STATUS_A    0x0A
#define RTC_STATUS_B    0x0B

static const char *weekday_names[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", 
    "Thursday", "Friday", "Saturday"
};

static const char *month_names[] = {
    "January", "February", "March", "April",
    "May", "June", "July", "August",
    "September", "October", "November", "December"
};

static const int days_in_month[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* Read RTC register */
static uint8_t rtc_read(uint8_t reg) {
    outb(RTC_ADDRESS, reg);
    return inb(RTC_DATA);
}

/* Check if RTC update is in progress */
static int rtc_update_in_progress(void) {
    outb(RTC_ADDRESS, RTC_STATUS_A);
    return inb(RTC_DATA) & 0x80;
}

/* Convert BCD to binary */
static uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

/* Initialize RTC */
void rtc_init(void) {
    /* Nothing special needed for reading RTC */
}

/* Read current time */
void rtc_read_time(time_t_rtc *time) {
    /* Wait for update to complete */
    while (rtc_update_in_progress());
    
    uint8_t second = rtc_read(RTC_SECONDS);
    uint8_t minute = rtc_read(RTC_MINUTES);
    uint8_t hour = rtc_read(RTC_HOURS);
    uint8_t day = rtc_read(RTC_DAY);
    uint8_t month = rtc_read(RTC_MONTH);
    uint8_t year = rtc_read(RTC_YEAR);
    uint8_t weekday = rtc_read(RTC_WEEKDAY);
    
    /* Check if RTC is in BCD mode */
    uint8_t status_b = rtc_read(RTC_STATUS_B);
    
    if (!(status_b & 0x04)) {
        /* BCD mode - convert to binary */
        second = bcd_to_bin(second);
        minute = bcd_to_bin(minute);
        hour = bcd_to_bin(hour & 0x7F) | (hour & 0x80);
        day = bcd_to_bin(day);
        month = bcd_to_bin(month);
        year = bcd_to_bin(year);
    }
    
    /* Handle 12-hour mode */
    if (!(status_b & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }
    
    time->second = second;
    time->minute = minute;
    time->hour = hour;
    time->day = day;
    time->month = month;
    time->year = 2000 + year;  /* Assume 21st century */
    time->weekday = weekday;
}

/* Get Unix-like timestamp */
uint32_t rtc_get_timestamp(void) {
    time_t_rtc time;
    rtc_read_time(&time);
    
    /* Simplified timestamp - days since epoch */
    uint32_t days = 0;
    
    /* Years since 2000 */
    for (int y = 2000; y < time.year; y++) {
        days += rtc_is_leap_year(y) ? 366 : 365;
    }
    
    /* Months in current year */
    for (int m = 1; m < time.month; m++) {
        days += rtc_get_days_in_month(m, time.year);
    }
    
    /* Days in current month */
    days += time.day - 1;
    
    /* Convert to seconds */
    uint32_t seconds = days * 86400;
    seconds += time.hour * 3600;
    seconds += time.minute * 60;
    seconds += time.second;
    
    return seconds;
}

/* Format time as HH:MM:SS */
void rtc_format_time(time_t_rtc *time, char *buffer) {
    sprintf(buffer, "%02d:%02d:%02d", time->hour, time->minute, time->second);
}

/* Format date as YYYY-MM-DD */
void rtc_format_date(time_t_rtc *time, char *buffer) {
    sprintf(buffer, "%04d-%02d-%02d", time->year, time->month, time->day);
}

/* Get days in month */
int rtc_get_days_in_month(int month, int year) {
    if (month < 1 || month > 12) {
        return 0;
    }
    
    int days = days_in_month[month - 1];
    
    if (month == 2 && rtc_is_leap_year(year)) {
        days++;
    }
    
    return days;
}

/* Check if year is a leap year */
int rtc_is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/* Get weekday name */
const char* rtc_get_weekday_name(int weekday) {
    if (weekday >= 0 && weekday < 7) {
        return weekday_names[weekday];
    }
    return "Unknown";
}

/* Get month name */
const char* rtc_get_month_name(int month) {
    if (month >= 1 && month <= 12) {
        return month_names[month - 1];
    }
    return "Unknown";
}
