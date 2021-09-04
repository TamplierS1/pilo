#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>
#include <unistd.h>
#include <string_view>
#include <cctype>
#include <sys/ioctl.h>

namespace Pilo
{
// Return the scancode of `key` pressed with ctrl.
constexpr char ctrl_key(uint8_t key);

class Editor
{
public:
    ~Editor()
    {
        // Make sure m_original is initialized.
        if (m_started)
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_original);
    }

    void start();

private:
    void enable_raw_mode();
    winsize get_window_size();

    char read_key();
    // Returns the new editor state.
    void process_input();

    void exit();
    void die(std::string_view msg);

    void refresh_screen();
    void draw_tildes();

    // Reposition cursor the top-left corner.
    void cursor_to_origin();
    void clear_screen();

    termios m_original;
    bool m_started = false;
};
}

#endif  // EDITOR_H
