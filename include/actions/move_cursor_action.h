#ifndef MOVE_CURSOR_ACTION_H
#define MOVE_CURSOR_ACTION_H

#include "action.h"
#include "../editor.h"

namespace Pilo
{
class MoveCursorAction : public Action
{
public:
    enum Direction
    {
        Up,
        Down,
        Right,
        Left
    };

    virtual ~MoveCursorAction(){};

    void execute(Editor& editor) override
    {
        switch (m_dir)
        {
            case Direction::Up:
                move_cursor_up(editor);
                break;
            case Direction::Down:
                move_cursor_down(editor);
                break;
            case Direction::Left:
                move_cursor_left(editor);
                break;
            case Direction::Right:
                move_cursor_right(editor);
                break;
        }
    }

    void move_cursor_up(Editor& editor)
    {
        editor.cursor_pos().y--;
        if (editor.cursor_pos().y < 0)
        {
            if (editor.starting_line() != 0)
                editor.starting_line()--;

            editor.cursor_pos().y = 0;
        }
        else if (editor.text()[editor.cur_pos_in_editor().y].size() <=
                 editor.cur_pos_in_editor().x)
        {
            editor.cursor_pos().x =
                editor.text()[editor.cur_pos_in_editor().y].size() - 1;
        }
    }
    void move_cursor_down(Editor& editor)
    {
        editor.cursor_pos().y++;
        if (editor.cursor_pos().y >= editor.win_size().y)
        {
            if (editor.starting_line() + editor.cursor_pos().y < editor.text().size())
                editor.starting_line()++;

            editor.cursor_pos().y = editor.win_size().y - 1;
        }
        else if (editor.text()[editor.cur_pos_in_editor().y].size() <=
                 editor.cur_pos_in_editor().x)
        {
            editor.cursor_pos().x =
                editor.text()[editor.cur_pos_in_editor().y].size() - 1;
        }
    }
    void move_cursor_left(Editor& editor)
    {
        if (editor.cursor_pos().y == 0 && editor.cursor_pos().x == 0)
            return;

        editor.cursor_pos().x--;
        if (editor.cursor_pos().x < 0)
        {
            move_cursor_up(editor);
        }
    }
    void move_cursor_right(Editor& editor)
    {
        editor.cursor_pos().x++;
        if (editor.cur_pos_in_editor().x >=
            editor.text()[editor.cur_pos_in_editor().y].size())
        {
            move_cursor_down(editor);
            editor.cursor_pos().x = 0;
        }
    }

    Direction m_dir;
};
}

#endif  //  MOVE_CURSOR_ACTION_H