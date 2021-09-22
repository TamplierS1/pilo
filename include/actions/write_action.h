#ifndef WRITE_ACTION_H
#define WRITE_ACTION_H

#include "../editor.h"
#include "../vec2.h"
#include "action.h"

namespace Pilo
{
class WriteAction : public Action
{
public:
    virtual ~WriteAction(){};

    void execute(Editor& editor) override
    {
        switch (m_ch)
        {
            case '\n':
                if (write(editor, editor.cur_pos_in_editor(), '\n'))
                {
                    editor.cursor_pos().y++;
                    if (editor.cursor_pos().y >= editor.win_size().y)
                    {
                        if (editor.cur_pos_in_editor().y < editor.text().size())
                            editor.starting_line()++;

                        editor.cursor_pos().y = editor.win_size().y - 1;
                    }
                    editor.cursor_pos().x = 0;
                }
                break;
            case '\t':
                if (write(editor, editor.cur_pos_in_editor(), ' '))
                {
                    write(editor, editor.cur_pos_in_editor(), ' ');
                    write(editor, editor.cur_pos_in_editor(), ' ');
                    write(editor, editor.cur_pos_in_editor(), ' ');
                }
                break;
            default:
                write(editor, m_pos, m_ch);
                break;
        }
    }

    bool write(Editor& editor, Vec2 pos, char ch)
    {
        if (pos.y >= editor.text().size())
        {
            return false;
        }
        else if (pos.x >= editor.text()[pos.y].size())
        {
            return false;
        }

        editor.text()[pos.y].insert(pos.x, 1, ch);

        if (ch == '\n')
        {
            editor.text().insert(editor.text().begin() + pos.y + 1,
                                 editor.text()[pos.y].substr(pos.x + 1));
            // Remove the contents that were appended from the previous line.
            editor.text()[pos.y] = editor.text()[pos.y].substr(0, pos.x + 1);
        }

        editor.cursor_pos().x++;

        return true;
    }

    Vec2 m_pos;
    char m_ch;
};
}

#endif  // WRITE_ACTION_H