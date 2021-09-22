#include <filesystem>
#include <fstream>
#include <sstream>

#include <ncurses.h>

#include "editor.h"
#include "actions/write_action.h"
#include "actions/quit_action.h"
#include "actions/save_file_action.h"
#include "actions/move_cursor_action.h"
#include "actions/delete_action.h"

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
        execute_actions();
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
        {
            QuitAction action;
            m_actions.emplace_back(std::make_unique<QuitAction>(action));
            break;
        }
        case ctrl_key('s'):
        {
            SaveFileAction action;
            action.m_filename = m_filename;
            m_actions.emplace_back(std::make_unique<SaveFileAction>(action));
            break;
        }
        case g_key_ctrl_backspace:
        {
            DeleteAction action;
            action.m_mode = DeleteAction::Mode::Word;
            m_actions.emplace_back(std::make_unique<DeleteAction>(action));
            break;
        }
        case KEY_UP:
        {
            MoveCursorAction action;
            action.m_dir = MoveCursorAction::Direction::Up;
            m_actions.push_back(std::make_unique<MoveCursorAction>(action));
            break;
        }
        case KEY_DOWN:
        {
            MoveCursorAction action;
            action.m_dir = MoveCursorAction::Direction::Down;
            m_actions.push_back(std::make_unique<MoveCursorAction>(action));
            break;
        }
        case KEY_LEFT:
        {
            MoveCursorAction action;
            action.m_dir = MoveCursorAction::Direction::Left;
            m_actions.push_back(std::make_unique<MoveCursorAction>(action));
            break;
        }
        case KEY_RIGHT:
        {
            MoveCursorAction action;
            action.m_dir = MoveCursorAction::Direction::Right;
            m_actions.push_back(std::make_unique<MoveCursorAction>(action));
            break;
        }
        case g_key_backspace:
        {
            DeleteAction action;
            action.m_mode = DeleteAction::Mode::Char;
            m_actions.emplace_back(std::make_unique<DeleteAction>(action));
            break;
        }
        case ERR:  // No character has been typed.
            break;
        default:
            WriteAction action;
            action.m_pos = cur_pos_in_editor();
            action.m_ch = ch;
            m_actions.emplace_back(std::make_unique<WriteAction>(action));
            break;
    }
}

void Editor::render_editor_window()
{
    move_cursor(stdscr, {0, 0});

    for (int y = 0; y < m_editor_win_size.y; y++)
    {
        if (y + m_starting_line_num < m_text.size())
        {
            wprintw(stdscr, "%s", m_text[y + m_starting_line_num].c_str());
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
    // wprintw(m_status_window, "%s -- %s\n", m_filename.c_str(),
    //         get_current_time().c_str());
    // ! Debug information.
    wprintw(m_status_window, "%d;%d", cur_pos_in_editor().x, cur_pos_in_editor().y);
    if (m_text.size() > cur_pos_in_editor().y)
        wprintw(m_status_window, " -- %s", m_text[cur_pos_in_editor().y].c_str());
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

void Editor::execute_actions()
{
    for (auto& action : m_actions)
    {
        action->execute(*this);
        m_actions.erase(m_actions.begin());
    }
}

void Editor::read_file(std::string_view filename)
{
    if (!std::filesystem::exists(filename))
    {
        std::ofstream new_file{filename.data()};
        new_file << " ";
    }

    std::ifstream file_stream{filename.data()};
    file_stream.exceptions(std::ifstream::badbit | std::ifstream::failbit);

    std::ostringstream sstream;
    sstream << file_stream.rdbuf();
    std::string file = sstream.str();

    int y = 0;
    m_text.push_back("");
    for (const auto& c : file)
    {
        m_text[y] += c;

        if (c == '\n')
        {
            m_text.push_back("");
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
