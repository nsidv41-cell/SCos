// SCos Window System
// Window class for desktop environment

#ifndef WINDOW_H
#define WINDOW_H

#include "../kernel/kernel.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"

// Window style flags
#define WS_TITLEBAR     0x01
#define WS_BORDER       0x02
#define WS_CLOSABLE     0x04
#define WS_MOVABLE      0x08
#define WS_RESIZABLE    0x10
#define WS_DEFAULT      (WS_TITLEBAR | WS_BORDER | WS_CLOSABLE | WS_MOVABLE)

// Window colors
#define WIN_BG          49      // Window background
#define WIN_BORDER      32      // Neon green border
#define WIN_TITLEBAR    50      // Titlebar background
#define WIN_TITLE_TEXT  32      // Title text color
#define WIN_CLOSE_BTN   46      // Close button
#define WIN_INACTIVE    34      // Inactive window border

class Window {
protected:
    int x, y;
    int width, height;
    const char* title;
    uint8_t style;
    bool visible;
    bool focused;
    bool dragging;
    int drag_offset_x, drag_offset_y;

public:
    Window();
    virtual ~Window();
    
    // Initialization
    virtual void init(int x, int y, int width, int height, const char* title = "Window", uint8_t style = WS_DEFAULT);
    
    // Drawing
    virtual void draw();
    virtual void draw_frame();
    virtual void draw_content();
    
    // Event handling
    virtual void handle_mouse(int mx, int my, bool left, bool right);
    virtual void handle_key(const KeyEvent& key);
    
    // Update (called each frame)
    virtual void update();
    
    // Geometry
    bool contains(int px, int py);
    bool title_bar_contains(int px, int py);
    bool close_button_contains(int px, int py);
    
    // Properties
    int get_x() const { return x; }
    int get_y() const { return y; }
    int get_width() const { return width; }
    int get_height() const { return height; }
    const char* get_title() const { return title; }
    
    void set_position(int nx, int ny);
    void set_size(int nw, int nh);
    void set_title(const char* new_title);
    void set_visible(bool vis) { visible = vis; }
    void set_focused(bool foc) { focused = foc; }
    
    bool is_visible() const { return visible; }
    bool is_focused() const { return focused; }
    
    // Close the window
    virtual void close();
};

#endif // WINDOW_H
