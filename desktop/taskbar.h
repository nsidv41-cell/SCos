// SCos Taskbar
// Bottom taskbar with start menu and system tray

#ifndef TASKBAR_H
#define TASKBAR_H

#include "../kernel/types.h"
#include "../drivers/vga.h"

#define TASKBAR_HEIGHT  20
#define TASKBAR_COLOR   50      // Dark background
#define TASKBAR_BORDER  32      // Neon green

class Taskbar {
private:
    int x, y;
    int width, height;
    bool start_menu_open;
    
    void draw_digit(int x, int y, int digit);
    void draw_menu_icon(int x, int y, int icon_type);

public:
    Taskbar();
    
    void init();
    void draw();
    void update();
    
    void handle_click(int mx, int my);
    bool contains(int px, int py);
    
    void toggle_start_menu();
    void draw_start_menu();
    void draw_start_button();
    void draw_system_tray();
};

#endif // TASKBAR_H
