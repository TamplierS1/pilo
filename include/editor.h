#ifndef EDITOR_H
#define EDITOR_H

#include <cctype>
#include <chrono>
#include <string>
#include <string_view>
#include <unordered_map>

#include "ncurses.h"

namespace Pilo
{
struct Vec2
{
    int x = 0;
    int y = 0;
};

constexpr char ctrl_key(char key)
{
    return key & 0x1f;
}

inline Vec2 get_window_size()
{
    Vec2 winsize;
    getmaxyx(stdscr, winsize.y, winsize.x);
    return winsize;
}

inline Vec2 get_cursor_pos()
{
    Vec2 pos;
    getyx(stdscr, pos.y, pos.x);
    return pos;
}

inline std::string get_current_time()
{
    using namespace std;
    time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
    return std::ctime(&now);
}

class Editor
{
public:
    Editor()
    {
        initscr();
        raw();
        noecho();
        keypad(stdscr, TRUE);
        halfdelay(1);

        m_window_size = get_window_size();
    }

    ~Editor()
    {
        endwin();
    }

    void start();

private:
    void process_input();
    void refresh_screen();

    void draw_rows();
    void move_cursor(Vec2 pos);

    Vec2 m_window_size;
    Vec2 m_cursor_pos = {0, 0};
};
}

#endif  // EDITOR_H
