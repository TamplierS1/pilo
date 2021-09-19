#include "editor.h"

#include <ncurses.h>

#include <fstream>
#include <sstream>

namespace Pilo
{
Editor::Editor()
{
    init_terminal();
    init_colors();
    init_windows();
}

Editor::~Editor()
{
    delete_window(m_status_window);
    delete_window(m_side_window);
    endwin();
}

void Editor::run(const std::string& filename)
{
    m_filename = filename;
    read_file(filename);

    while (m_editor_state == State::Alive)
    {
        process_input();
        refresh_screen();
    }
}

void Editor::init_terminal()
{
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    halfdelay(1);
}

void Editor::init_colors()
{
    start_color();
    init_pair(ColorStatusWin, COLOR_BLUE, COLOR_BLACK);
}

void Editor::init_windows()
{
    m_editor_win_size = get_window_size(stdscr);
    wresize(stdscr, --m_editor_win_size.y, m_editor_win_size.x - m_side_win_width);
    mvwin(stdscr, 0, m_side_win_width);

    m_status_window = create_window({m_editor_win_size.x, m_status_win_height},
                                    {0, m_editor_win_size.y});
    m_side_window = create_window({m_side_win_width, m_editor_win_size.y}, {0, 0});
}

void Editor::process_input()
{
    int ch = getch();

    switch (ch)
    {
        case ctrl_key('q'):
            m_editor_state = State::Dead;
            break;
        case ctrl_key('s'):
            save_to_file();
            break;
        case KEY_UP:
            m_cursor_pos.y--;
            if (m_cursor_pos.y < 0)
            {
                if (m_starting_line_num != 0)
                    m_starting_line_num--;

                m_cursor_pos.y = 0;
            }

            break;
        case KEY_DOWN:
            m_cursor_pos.y++;
            if (m_cursor_pos.y >= m_editor_win_size.y)
            {
                if (m_starting_line_num + m_cursor_pos.y < m_rows.size())
                    m_starting_line_num++;

                m_cursor_pos.y = m_editor_win_size.y - 1;
            }
            break;
        case KEY_LEFT:
            m_cursor_pos.x--;
            if (m_cursor_pos.x < 0)
                m_cursor_pos.x = 0;
            break;
        case KEY_RIGHT:
            m_cursor_pos.x++;
            if (m_cursor_pos.x >= m_editor_win_size.x)
                m_cursor_pos.x = m_editor_win_size.x - 1;
            break;
        case 8:  // Backspace
        case 127:
            if (del(m_cursor_pos))
                m_cursor_pos.x--;
            break;
        case '\n':
            if (write(m_cursor_pos, ch))
            {
                m_cursor_pos.y++;
                m_cursor_pos.x = 0;
            }
            break;
        case ERR:  // No character has been typed.
            break;
        default:
            if (write(m_cursor_pos, ch))
                m_cursor_pos.x++;
            break;
    }
}

void Editor::render_editor_window()
{
    move_cursor(stdscr, {0, 0});

    for (int y = 0; y < m_editor_win_size.y; y++)
    {
        if (y + m_starting_line_num < m_rows.size())
        {
            wprintw(stdscr, "%s", m_rows[y + m_starting_line_num].c_str());
        }
    }
}

void Editor::render_side_window()
{
    move_cursor(m_side_window, {0, 0});

    for (int y = 0; y < m_editor_win_size.y; y++)
    {
        // Line numbers.
        wprintw(m_side_window, "%d\n", y + m_starting_line_num);
    }
}

void Editor::render_status_window()
{
    move_cursor(m_status_window, {0, 0});

    wattron(m_status_window, COLOR_PAIR(ColorStatusWin) | A_BOLD);
    wprintw(m_status_window, "%s -- %s -- %d;%d %s\n", m_filename.c_str(),
            get_current_time().c_str(), m_cursor_pos.x, m_cursor_pos.y,
            m_last_error.c_str());
    wattroff(m_status_window, COLOR_PAIR(ColorStatusWin) | A_BOLD);
}

void Editor::refresh_screen()
{
    render_editor_window();
    render_side_window();
    render_status_window();

    wrefresh(stdscr);
    wrefresh(m_status_window);
    wrefresh(m_side_window);

    move_cursor(stdscr, m_cursor_pos);
}

void Editor::read_file(std::string_view filename)
{
    std::ifstream file_stream{filename.data()};
    file_stream.exceptions(std::ifstream::badbit | std::ifstream::failbit);

    std::ostringstream sstream;
    sstream << file_stream.rdbuf();
    std::string file = sstream.str();

    int y = 0;
    m_rows.push_back("");
    for (const auto& c : file)
    {
        m_rows[y] += c;

        if (c == '\n')
        {
            m_rows.push_back("");
            y++;
        }
    }
}

void Editor::save_to_file()
{
    std::ofstream file{m_filename, std::ios_base::trunc};
    file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

    for (const auto& row : m_rows)
    {
        file << row;
    }
}

void Editor::move_cursor(WINDOW* win, Vec2 pos)
{
    wmove(win, pos.y, pos.x);
}

bool Editor::write(Vec2 pos, char ch)
{
    if (pos.y >= m_rows.size())
    {
        // ! Maybe use {fmt} here instead of sprintf.
        sprintf(m_last_error.data(), "Failed to write %c to %d;%d.", ch, pos.x, pos.y);
        return false;
    }
    else if (pos.x >= m_rows[pos.y].size())
    {
        sprintf(m_last_error.data(), "Failed to write %c to %d;%d.", ch, pos.x, pos.y);
        return false;
    }

    m_rows[pos.y].insert(pos.x, 1, ch);

    return true;
}

bool Editor::del(Vec2 pos)
{
    if (pos.y >= m_rows.size())
    {
        // ! Maybe use {fmt} here instead of sprintf.
        sprintf(m_last_error.data(), "Failed to delete a char in %d;%d.", pos.x, pos.y);
        return false;
    }
    else if (pos.x >= m_rows[pos.y].size() || pos.x <= 0)
    {
        sprintf(m_last_error.data(), "Failed to delete a char in %d;%d.", pos.x, pos.y);
        return false;
    }

    m_rows[pos.y].erase(pos.x - 1, 1);

    return true;
}

WINDOW* Editor::create_window(Vec2 size, Vec2 pos) const
{
    auto win = newwin(size.y, size.x, pos.y, pos.x);
    return win;
}

void Editor::delete_window(WINDOW* win)
{
    delwin(win);
}
}
