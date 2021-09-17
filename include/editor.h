#ifndef EDITOR_H
#define EDITOR_H

#include <cctype>
#include <chrono>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "ncurses.h"

namespace Pilo
{
struct Vec2
{
    int x = 0;
    int y = 0;
};

enum class State
{
    Alive,
    Dead
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

    void run(const std::string& filename);

private:
    void process_input();
    void refresh_screen();
    // If no arguments were provided `m_filename` will be used
    // to read the file into `m_rows`.
    void read_file(std::string_view filename = "");

    void draw_rows();
    void move_cursor(Vec2 pos);

    Vec2 m_window_size;
    Vec2 m_cursor_pos = {0, 0};
    State m_editor_state = State::Alive;

    std::vector<std::string> m_rows;
    std::string m_filename;
    // The line of the file to start drawing text from.
    // Used to scroll the file.
    int m_starting_line_num = 0;
};
}

#endif  // EDITOR_H
