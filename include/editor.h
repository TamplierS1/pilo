#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>
#include <unistd.h>
#include <string_view>
#include <cctype>
#include <sys/ioctl.h>
#include <string>
#include <unordered_map>

namespace Pilo
{
// Return the scancode of `key` pressed with ctrl.
constexpr char ctrl_key(uint8_t key);

struct Vec2
{
    int x = 0;
    int y = 0;
};

// Control commands that can be sent to the terminal.
enum class CnSeq
{
    ClearScreen,
    CursorToOrigin,
    ReportCursorPos,
    HideCursor,
    ShowCursor,
    // Erase everything in the line to the right of the cursor
    EraseLineRight
};

class Editor
{
public:
    ~Editor()
    {

    }

    void start();

private:
    void enable_raw_mode();
    winsize get_window_size();
    Vec2 get_cursor_pos();
    std::string get_current_time() const;

    void refresh_screen();
    void process_input();

    void draw_rows();
    char read_key();
    void write(const std::string& str);
    // Use this to issue basic commands that don't require parameters.
    void issue_command(CnSeq seq);
    void move_cursor(Vec2 pos);

    void exit();
    void die(std::string_view msg);

    termios m_original;
    bool m_started = false;
    std::string m_draw_buffer;

    winsize m_window_size;
    Vec2 m_cursor_pos = {0, 0};

    std::unordered_map<CnSeq, std::string> m_control_commands_map = {
        {CnSeq::ClearScreen, "\x1b[2J"},     {CnSeq::CursorToOrigin, "\x1b[H"},
        {CnSeq::ReportCursorPos, "\x1b[6n"}, {CnSeq::HideCursor, "\x1b[?25l"},
        {CnSeq::ShowCursor, "\x1b[?25h"},    {CnSeq::EraseLineRight, "\x1b[K"}};
};
}

#endif  // EDITOR_H
