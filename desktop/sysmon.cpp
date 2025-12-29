// SCos System Monitor Implementation

#include "sysmon.h"
#include "../kernel/string.h"

SystemMonitor::SystemMonitor() : Window() {
    last_update = 0;
    update_interval = 500;  // Update every 500ms
    cpu_usage = 0;
    mem_used = 0;
    mem_total = 0;
    uptime_secs = 0;
    history_index = 0;
    
    for (int i = 0; i < 60; i++) {
        cpu_history[i] = 0;
        mem_history[i] = 0;
    }
}

SystemMonitor::~SystemMonitor() {
}

void SystemMonitor::init(int wx, int wy, int ww, int wh) {
    Window::init(wx, wy, ww, wh, "System Monitor", WS_DEFAULT);
    update_stats();
}

void SystemMonitor::update() {
    uint32_t current_time = Timer::get_milliseconds();
    
    if (current_time - last_update >= update_interval) {
        update_stats();
        last_update = current_time;
    }
}

void SystemMonitor::update_stats() {
    // Get memory stats
    mem_total = Memory::get_total_memory();
    mem_used = Memory::get_used_memory();
    
    // Calculate memory percentage
    uint32_t mem_percent = 0;
    if (mem_total > 0) {
        mem_percent = (mem_used * 100) / mem_total;
    }
    
    // Simulate CPU usage (in a real OS, this would measure actual CPU time)
    // For demo purposes, we'll use a pseudo-random value based on timer
    uint32_t ticks = Timer::get_ticks();
    cpu_usage = ((ticks * 7) % 100);  // Pseudo-random 0-99
    
    // Update uptime
    uptime_secs = Timer::get_seconds();
    
    // Update history
    cpu_history[history_index] = (uint8_t)cpu_usage;
    mem_history[history_index] = (uint8_t)mem_percent;
    history_index = (history_index + 1) % 60;
}

void SystemMonitor::draw_content() {
    int content_x = x + 4;
    int content_y = y + 16;
    int content_w = width - 8;
    int content_h = height - 20;
    
    // Title area - CPU
    draw_label(content_x, content_y, "CPU");
    
    // CPU bar
    draw_bar(content_x + 25, content_y, content_w - 30, 8, cpu_usage, 32);
    
    content_y += 14;
    
    // Memory label
    draw_label(content_x, content_y, "MEM");
    
    // Memory bar
    uint32_t mem_percent = 0;
    if (mem_total > 0) {
        mem_percent = (mem_used * 100) / mem_total;
    }
    draw_bar(content_x + 25, content_y, content_w - 30, 8, mem_percent, 33);
    
    content_y += 14;
    
    // CPU Graph
    draw_label(content_x, content_y, "CPU:");
    content_y += 10;
    draw_graph(content_x, content_y, content_w, 20, cpu_history, 60, 32);
    
    content_y += 24;
    
    // Uptime
    draw_label(content_x, content_y, "UP:");
    
    // Format uptime
    uint32_t hours = uptime_secs / 3600;
    uint32_t mins = (uptime_secs % 3600) / 60;
    uint32_t secs = uptime_secs % 60;
    
    // Draw time values as colored dots (simplified display)
    int time_x = content_x + 20;
    
    // Hours indicator
    for (uint32_t i = 0; i < hours && i < 10; i++) {
        VGA::fill_rect(time_x + i * 4, content_y, 3, 6, 32);
    }
    
    // Separator
    VGA::put_pixel(time_x + 44, content_y + 1, 34);
    VGA::put_pixel(time_x + 44, content_y + 4, 34);
    
    // Minutes indicator
    for (uint32_t i = 0; i < mins / 6; i++) {
        VGA::fill_rect(time_x + 48 + i * 4, content_y, 3, 6, 33);
    }
}

void SystemMonitor::draw_graph(int gx, int gy, int gw, int gh, uint8_t* data, int count, uint8_t color) {
    // Draw graph background
    VGA::fill_rect(gx, gy, gw, gh, 1);
    VGA::draw_rect(gx, gy, gw, gh, 34);
    
    // Draw grid lines
    for (int i = 0; i < 4; i++) {
        int line_y = gy + (gh * i) / 4;
        for (int lx = gx; lx < gx + gw; lx += 4) {
            VGA::put_pixel(lx, line_y, 1);
        }
    }
    
    // Draw data
    if (count <= 0 || gw <= 0) return;
    
    int step = gw / count;
    if (step < 1) step = 1;
    
    int prev_x = gx;
    int prev_y = gy + gh - 1;
    
    for (int i = 0; i < count && i * step < gw; i++) {
        int data_index = (history_index + i) % count;
        int value = data[data_index];
        
        // Scale value to graph height
        int bar_height = (value * (gh - 2)) / 100;
        int cur_x = gx + i * step;
        int cur_y = gy + gh - 1 - bar_height;
        
        // Draw line segment
        if (i > 0) {
            VGA::draw_line(prev_x, prev_y, cur_x, cur_y, color);
        }
        
        prev_x = cur_x;
        prev_y = cur_y;
    }
}

void SystemMonitor::draw_bar(int bx, int by, int bw, int bh, int percent, uint8_t color) {
    // Clamp percent
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    // Draw background
    VGA::fill_rect(bx, by, bw, bh, 1);
    VGA::draw_rect(bx, by, bw, bh, 34);
    
    // Draw filled portion
    int fill_width = (bw - 2) * percent / 100;
    if (fill_width > 0) {
        VGA::fill_rect(bx + 1, by + 1, fill_width, bh - 2, color);
    }
    
    // Draw percentage markers
    for (int i = 1; i < 4; i++) {
        int marker_x = bx + (bw * i) / 4;
        VGA::draw_line(marker_x, by, marker_x, by + 2, 48);
        VGA::draw_line(marker_x, by + bh - 2, marker_x, by + bh, 48);
    }
}

void SystemMonitor::draw_label(int lx, int ly, const char* text) {
    // Simple label drawing (just colored pixels representing text)
    // In a full implementation, we'd use a bitmap font
    
    int char_width = 4;
    int i = 0;
    
    while (text[i] != '\0' && i < 10) {
        // Draw a simple representation of each character
        int cx = lx + i * char_width;
        
        switch (text[i]) {
            case 'C':
                VGA::draw_line(cx, ly, cx + 2, ly, 32);
                VGA::draw_line(cx, ly, cx, ly + 4, 32);
                VGA::draw_line(cx, ly + 4, cx + 2, ly + 4, 32);
                break;
            case 'P':
                VGA::draw_line(cx, ly, cx, ly + 4, 32);
                VGA::draw_line(cx, ly, cx + 2, ly, 32);
                VGA::draw_line(cx + 2, ly, cx + 2, ly + 2, 32);
                VGA::draw_line(cx, ly + 2, cx + 2, ly + 2, 32);
                break;
            case 'U':
                VGA::draw_line(cx, ly, cx, ly + 4, 32);
                VGA::draw_line(cx + 2, ly, cx + 2, ly + 4, 32);
                VGA::draw_line(cx, ly + 4, cx + 2, ly + 4, 32);
                break;
            case 'M':
                VGA::draw_line(cx, ly, cx, ly + 4, 32);
                VGA::draw_line(cx + 2, ly, cx + 2, ly + 4, 32);
                VGA::draw_line(cx, ly, cx + 1, ly + 2, 32);
                VGA::draw_line(cx + 1, ly + 2, cx + 2, ly, 32);
                break;
            case 'E':
                VGA::draw_line(cx, ly, cx, ly + 4, 32);
                VGA::draw_line(cx, ly, cx + 2, ly, 32);
                VGA::draw_line(cx, ly + 2, cx + 2, ly + 2, 32);
                VGA::draw_line(cx, ly + 4, cx + 2, ly + 4, 32);
                break;
            case ':':
                VGA::put_pixel(cx, ly + 1, 32);
                VGA::put_pixel(cx, ly + 3, 32);
                break;
            default:
                // Draw a small rectangle for unknown characters
                VGA::draw_rect(cx, ly, 2, 4, 34);
                break;
        }
        
        i++;
    }
}
