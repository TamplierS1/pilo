#include <iostream>

#include "fmt/printf.h"

#include "editor.h"

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
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attribs) == -1)
        die("tcsetattr");
}

winsize Editor::get_window_size()
{
    winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1 || size.ws_col == 0)
        die("ioctl (get_window_size)");

    return size;
}

void Editor::start()
{
    if (tcgetattr(STDIN_FILENO, &m_original) == -1)
        die("tcgetattr");

    m_started = true;
    enable_raw_mode();

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
        default:
            break;
    }
}

void Editor::exit()
{
    // Add any cleanup code here.

    clear_screen();
    cursor_to_origin();

    std::exit(EXIT_FAILURE);
}

void Editor::die(std::string_view msg)
{
    clear_screen();
    cursor_to_origin();

    std::perror(msg.data());
    std::exit(EXIT_FAILURE);
}

void Editor::refresh_screen()
{
    clear_screen();
    cursor_to_origin();
    draw_tildes();
    cursor_to_origin();
}

void Editor::draw_tildes()
{
    for (int y = 0; y < get_window_size().ws_row; y++)
    {
        std::cout.write("~\r\n", 3);
    }
}

void Editor::cursor_to_origin()
{
    std::cout.write("\x1b[H", 3);
}

void Editor::clear_screen()
{
    std::cout.write("\x1b[2J", 4);
}

}
