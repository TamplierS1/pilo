#ifndef DELETE_ACTION_H
#define DELETE_ACTION_H

#include "action.h"
#include "../editor.h"

namespace Pilo
{
class DeleteAction : public Action
{
public:
    enum Mode
    {
        Char,
        Word,
    };

    virtual ~DeleteAction(){};

    void execute(Editor& editor) override
    {
        switch (m_mode)
        {
            case Mode::Char:
            {
                // If a line gets deleted, the cursor will be moved to this index.
                int new_cursor_pos =
                    editor.text()[editor.cur_pos_in_editor().y - 1].size() - 1;
                if (del(editor, editor.cur_pos_in_editor()))
                {
                    if (editor.cur_pos_in_editor().x == 0)
                    {
                        editor.cursor_pos().y--;
                        editor.cursor_pos().x = new_cursor_pos;
                        return;
                    }
                    editor.cursor_pos().x--;
                }
                break;
            }
            case Mode::Word:
            {
                if (editor.text()[editor.cur_pos_in_editor().y]
                                 [editor.cur_pos_in_editor().x - 1] == ' ')
                {
                    while (editor.text()[editor.cur_pos_in_editor().y]
                                        [editor.cur_pos_in_editor().x - 1] == ' ' &&
                           editor.cur_pos_in_editor().x != 0)
                    {
                        if (del(editor, editor.cur_pos_in_editor()))
                        {
                            editor.cursor_pos().x--;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else
                {
                    while (editor.text()[editor.cur_pos_in_editor().y]
                                        [editor.cur_pos_in_editor().x - 1] != ' ' &&
                           editor.cur_pos_in_editor().x != 0)
                    {
                        if (del(editor, editor.cur_pos_in_editor()))
                        {
                            editor.cursor_pos().x--;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                break;
            }
        }
    }

    bool del(Editor& editor, Vec2 pos)
    {
        if (pos.y >= editor.text().size())
        {
            return false;
        }
        else if (pos.x >= editor.text()[pos.y].size() || pos.x <= 0)
        {
            // Handle deleting lines.
            if (pos.x == 0 && pos.y != 0)
            {
                // Lines that only contain '\n'.
                if (editor.text()[pos.y][pos.x] == '\n')
                {
                    editor.text().erase(editor.text().begin() + pos.y);
                    return true;
                }

                // Delete the last character (it's always \n).
                editor.text()[pos.y - 1] = editor.text()[pos.y - 1].substr(
                    0, editor.text()[pos.y - 1].size() - 1);

                editor.text()[pos.y - 1] += editor.text()[pos.y];
                editor.text().erase(editor.text().begin() + pos.y);
                return true;
            }
            return false;
        }

        editor.text()[pos.y].erase(pos.x - 1, 1);

        return true;
    }

    Mode m_mode;
};
}

#endif  // DELETE_ACTION_H