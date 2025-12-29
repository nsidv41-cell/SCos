// SCos System Monitor
// Real-time system statistics display

#ifndef SYSMON_H
#define SYSMON_H

#include "window.h"
#include "../kernel/types.h"
#include "../kernel/memory.h"
#include "../drivers/timer.h"

class SystemMonitor : public Window {
private:
    uint32_t last_update;
    uint32_t update_interval;
    
    // Stats
    uint32_t cpu_usage;
    uint32_t mem_used;
    uint32_t mem_total;
    uint32_t uptime_secs;
    
    // Graph data
    uint8_t cpu_history[60];
    uint8_t mem_history[60];
    int history_index;
    
    void update_stats();
    void draw_graph(int x, int y, int w, int h, uint8_t* data, int count, uint8_t color);
    void draw_bar(int x, int y, int w, int h, int percent, uint8_t color);
    void draw_label(int x, int y, const char* text);

public:
    SystemMonitor();
    virtual ~SystemMonitor();
    
    void init(int x, int y, int width, int height);
    
    virtual void draw_content() override;
    virtual void update() override;
};

#endif // SYSMON_H
