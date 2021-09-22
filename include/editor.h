#ifndef EDITOR_H
#define EDITOR_H

#include <cctype>
#include <chrono>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <memory>

#include "ncurses.h"

#include "actions/action.h"
#include "vec2.h"

namespace Pilo
{
// ncurses' KEY_BACKSPACE doesn't work for some reason.
constexpr int g_key_backspace = 127;
constexpr int g_key_ctrl_backspace = 263;

enum class State
{
    Alive,
    Dead
};

// This enum is often converted to int, short, etc.
// So it's easier to use a plain enum instead of `enum class`.
enum ColorPairs
{
    // You can't use 0 as a color pair in ncurses.
    ColorStatusWin = 1,
};

enum Colors
{
    ColorBlack,
    ColorGrey,
    ColorGreen,
};

constexpr char ctrl_key(char key)
{
    return key & 0x1f;
}

inline Vec2 get_window_size(WINDOW* win)
{
    Vec2 winsize;
    getmaxyx(win, winsize.y, winsize.x);
    return winsize;
}

inline Vec2 get_cursor_pos(WINDOW* win)
{
    Vec2 pos;
    getyx(win, pos.y, pos.x);
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
    Editor();
    ~Editor();

    void run(const std::string& filename);
    void die()
    {
        m_editor_state = State::Dead;
    }

    std::vector<std::string>& text()
    {
        return m_text;
    }
    Vec2& cursor_pos()
    {
        return m_cursor_pos;
    }
    // Convert cursor screen position to editor position.
    constexpr Vec2 cur_pos_in_editor() const
    {
        return {m_cursor_pos.x, m_cursor_pos.y + m_starting_line_num};
    }
    int& starting_line()
    {
        return m_starting_line_num;
    }
    Vec2 win_size()
    {
        return m_editor_win_size;
    }

private:
    void init_terminal();
    void init_colors();
    void init_windows();

    void process_input();
    void refresh_screen();
    void execute_actions();
    // If no arguments were provided `m_filename` will be used
    // to read the file into `m_rows`.
    void read_file(std::string_view filename = "");

    void render_editor_window();
    void render_side_window();
    void render_status_window();

    void move_cursor(WINDOW* win, Vec2 pos);
    WINDOW* create_window(Vec2 size, Vec2 pos) const;
    void delete_window(WINDOW* win);

    Vec2 m_editor_win_size;
    Vec2 m_cursor_pos = {0, 0};

    State m_editor_state = State::Alive;
    std::vector<std::unique_ptr<Action>> m_actions;

    WINDOW* m_status_window = nullptr;
    WINDOW* m_side_window = nullptr;

    // Editor window
    std::vector<std::string> m_text;
    std::string m_filename;
    // The line of the file to start drawing text from.
    // Used to scroll the file.
    int m_starting_line_num = 0;

    // Side window
    int m_side_win_width = 5;

    // Status window
    int m_status_win_height = 1;
};
}

#endif  // EDITOR_H
