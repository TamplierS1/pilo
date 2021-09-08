#include "editor.h"

#include <ncurses.h>

#include <chrono>
#include <ctime>
#include <iostream>

#include "fmt/core.h"

namespace Pilo
{
constexpr char ctrl_key(uint8_t key)
{
    // Clear the upper 3 bits of the pressed character.
    // This mirrors what ctrl key does when you press it with other characters.
    // You can also switch between lowercase and uppercase by clearing and
    // setting bit 5 apparently.
    return static_cast<char>(key & 0x1f);
}

void Editor::enable_raw_mode()
{
    termios term_attribs;
    if (tcgetattr(STDIN_FILENO, &term_attribs) == -1)
        die("tcgetattr");
    // BRKINT - send SIGINT when break condition is met.
    // INPCK - enable parity checking.
    // ISTIP - strip the 8th bit of every input byte.
    // IXON - software flow control (ctrl-s, ctrl-q)
    // ICRNL - turn \r into \n
    // OPOST - output processing.
    // CS8 - set the character size to 8.
    // ECHO - turn on echoing.
    // ICANON - turn on canonical mode.
    // IEXTEN - enables you to input characters literally.
    // ISIG - turn on signals.
    term_attribs.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON | ICRNL);
    term_attribs.c_oflag &= ~(OPOST);
    term_attribs.c_cflag |= CS8;
    term_attribs.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // The minimum number of chars to read.
    // The editor refuses to process any key strokes
    // when this line is uncommented.
    //    term_attribs.c_cc[VMIN] = 0;

    // The maximum amount of time to wait (in ms) before 'std::cin.read()' returns.
    term_attribs.c_cc[VTIME] = 1;
    // TODO: setup better error handling instead of just dying.
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attribs) == -1)
        die("tcsetattr");
}

winsize Editor::get_window_size()
{
    winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1 || size.ws_col == 0)
    {
        // If ioctl didn't work, use this fallback method
        // to get window size.
        fmt::print("\x1b[999C\x1b[999B");
        Vec2 pos = get_cursor_pos();
        size.ws_row = pos.y;
        size.ws_col = pos.x;
    }

    return size;
}

Vec2 Editor::get_cursor_pos()
{
    std::string response;
    while (true)
    {
        char c;
        std::cin.read(&c, 1);
        response += c;
        if (c == 'R')
            break;
    }

    // TODO: do something when an ill-formed response is received.
    //    if (response[0] != '\x1b' || response[1] != '[')
    Vec2 pos;
    // TODO: also do some error handling.
    // I could something with stringstreams, but they take
    // way more lines of code than sscanf does.
    sscanf(response.c_str(), "\x1b[%d;%d", &pos.y, &pos.x);
    return pos;
}

void Editor::start()
{
    if (tcgetattr(STDIN_FILENO, &m_original) == -1)
        die("tcgetattr");

    m_started = true;
    enable_raw_mode();

    m_window_size = get_window_size();

    while (true)
    {
        refresh_screen();
        process_input();
    }
}

char Editor::read_key()
{
    char c = '\0';
    while (!std::cin.read(&c, 1).good())
    {
        if (std::cin.bad())
            die("read");
    }

    // Read escape sequences.
    if (c == '\x1b')
    {
        std::string seq;
        // Read two additional chars.
        if (!std::cin.read(&c, 1).good())
            return '\x1b';
        seq += c;
        if (!std::cin.read(&c, 1).good())
            return '\x1b';
        seq += c;

        if (seq[0] == '[')
        {
            switch (seq[1])
            {
                case 'A':
                    return 'w';
                case 'B':
                    return 's';
                case 'C':
                    return 'd';
                case 'D':
                    return 'a';
            }
        }

        return '\x1b';
    }
    return c;
}

void Editor::process_input()
{
    char c = read_key();

    switch (c)
    {
        case ctrl_key('q'):
            exit();
            break;
        case 'w':
            m_cursor_pos.y--;
            break;
        case 's':
            m_cursor_pos.y++;
            break;
        case 'a':
            m_cursor_pos.x--;
            break;
        case 'd':
            m_cursor_pos.x++;
            break;
        default:
            break;
    }
}

void Editor::write(const std::string& str)
{
    m_draw_buffer += str;
}

void Editor::issue_command(CnSeq seq)
{
    fmt::print("{}", m_control_commands_map[seq]);
}

void Editor::move_cursor(Vec2 pos)
{
    // The terminal starts counting from 1, not 0.
    fmt::print("\x1b[{};{}H", pos.y + 1, pos.x + 1);
}

void Editor::exit()
{
    // Add any cleanup code here.

    issue_command(CnSeq::ClearScreen);
    issue_command(CnSeq::CursorToOrigin);

    std::exit(EXIT_FAILURE);
}

void Editor::die(std::string_view msg)
{
    issue_command(CnSeq::ClearScreen);
    issue_command(CnSeq::CursorToOrigin);

    std::perror(msg.data());
    std::exit(EXIT_FAILURE);
}

std::string Editor::get_current_time() const
{
    using namespace std;
    time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
    return std::ctime(&now);
}

void Editor::refresh_screen()
{
    issue_command(CnSeq::HideCursor);
    issue_command(CnSeq::CursorToOrigin);

    draw_rows();
    issue_command(CnSeq::CursorToOrigin);

    fmt::print("{}", m_draw_buffer);
    move_cursor(m_cursor_pos);

    issue_command(CnSeq::ShowCursor);
}

void Editor::draw_rows()
{
    for (int y = 0; y < m_window_size.ws_row; y++)
    {
        if (y == m_window_size.ws_row - 1)
        {
            write("Pilo editor -- ");
            write(get_current_time());
            continue;
        }

        write("~");

        issue_command(CnSeq::EraseLineRight);

        // Don't print \r\n on the last line.
        if (y < m_window_size.ws_row - 1)
            write("\r\n");
    }
}

}
