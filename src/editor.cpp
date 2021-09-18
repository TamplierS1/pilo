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
    m_screen_size = get_window_size(stdscr);
    wresize(stdscr, m_screen_size.y - 1, m_screen_size.x);

    m_status_window = create_window({m_screen_size.x, 1}, {0, m_screen_size.y - 1});
}

void Editor::process_input()
{
    int c = getch();

    switch (c)
    {
        case ctrl_key('q'):
            m_editor_state = State::Dead;
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
            if (m_cursor_pos.y >= m_screen_size.y)
            {
                if (m_starting_line_num + m_cursor_pos.y < m_rows.size())
                    m_starting_line_num++;

                m_cursor_pos.y = m_screen_size.y - 1;
            }
            break;
        case KEY_LEFT:
            m_cursor_pos.x--;
            if (m_cursor_pos.x < 0)
                m_cursor_pos.x = 0;
            break;
        case KEY_RIGHT:
            m_cursor_pos.x++;
            if (m_cursor_pos.x >= m_screen_size.x)
                m_cursor_pos.x = m_screen_size.x - 1;
            break;
        case ERR:  // No character has been typed.
        default:
            break;
    }
}

void Editor::render_editor_window()
{
    move_cursor(stdscr, {0, 0});

    for (int y = 0; y < m_screen_size.y; y++)
    {
        if (y + m_starting_line_num < m_rows.size())
        {
            wprintw(stdscr, m_rows[y + m_starting_line_num].c_str());
        }
    }
}

void Editor::render_status_window()
{
    move_cursor(m_status_window, {0, 0});

    wattron(m_status_window, COLOR_PAIR(ColorStatusWin) | A_BOLD);
    wprintw(m_status_window, m_filename.c_str());
    wprintw(m_status_window, " -- ");
    wprintw(m_status_window, get_current_time().c_str());
    wprintw(m_status_window, "\n");
    wattroff(m_status_window, COLOR_PAIR(ColorStatusWin) | A_BOLD);
}

void Editor::refresh_screen()
{
    render_editor_window();
    render_status_window();

    wrefresh(stdscr);
    wrefresh(m_status_window);

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

void Editor::move_cursor(WINDOW* win, Vec2 pos)
{
    wmove(win, pos.y, pos.x);
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
