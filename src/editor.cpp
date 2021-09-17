#include "editor.h"

#include <ncurses.h>

#include <ctime>
#include <iostream>

#include "fmt/core.h"

namespace Pilo
{
void Editor::start()
{
    while (true)
    {
        process_input();
        refresh_screen();
    }
}

void Editor::process_input()
{
    char c = getch();

    switch (c)
    {
        case ctrl_key('q'):
            // TODO: create an exit event.
            std::exit(EXIT_FAILURE);
            break;
        case 'w':
            m_cursor_pos.y--;
            if (m_cursor_pos.y < 0)
                m_cursor_pos.y = 0;
            break;
        case 's':
            m_cursor_pos.y++;
            if (m_cursor_pos.y >= m_window_size.y)
                m_cursor_pos.y = m_window_size.y - 1;
            break;
        case 'a':
            m_cursor_pos.x--;
            if (m_cursor_pos.x < 0)
                m_cursor_pos.x = 0;
            break;
        case 'd':
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
            printw("Pilo editor -- ");
            printw(get_current_time().c_str());
            continue;
        }

        printw("~");

        // Don't print \n on the last line.
        if (y < m_window_size.y - 1)
        {
            printw("\n");
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

void Editor::move_cursor(Vec2 pos)
{
    move(pos.y, pos.x);
}
}
