#include "editor.h"

#include <ncurses.h>

#include <fstream>
#include <sstream>

namespace Pilo
{
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
            if (m_cursor_pos.y >= m_window_size.y)
            {
                if (m_starting_line_num + m_cursor_pos.y < m_rows.size())
                    m_starting_line_num++;

                m_cursor_pos.y = m_window_size.y - 1;
            }
            break;
        case KEY_LEFT:
            m_cursor_pos.x--;
            if (m_cursor_pos.x < 0)
                m_cursor_pos.x = 0;
            break;
        case KEY_RIGHT:
            m_cursor_pos.x++;
            if (m_cursor_pos.x >= m_window_size.x)
                m_cursor_pos.x = m_window_size.x - 1;
            break;
        case ERR:  // No character has been typed.
        default:
            break;
    }
}

void Editor::draw_rows()
{
    for (int y = 0; y < m_window_size.y; y++)
    {
        if (y == m_window_size.y - 1)
        {
            printw(m_filename.c_str());
            printw(" -- ");
            printw(get_current_time().c_str());
            break;
        }

        if (y + m_starting_line_num < m_rows.size())
        {
            printw(m_rows[y + m_starting_line_num].c_str());
        }
    }
}

void Editor::refresh_screen()
{
    move_cursor({0, 0});
    draw_rows();
    move_cursor({0, 0});
    refresh();
    move_cursor(m_cursor_pos);
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

void Editor::move_cursor(Vec2 pos)
{
    move(pos.y, pos.x);
}
}
