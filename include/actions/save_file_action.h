#ifndef SAVE_FILE_ACTION_H
#define SAVE_FILE_ACTION_H

#include <string_view>
#include <fstream>

#include "action.h"
#include "../editor.h"

namespace Pilo
{
class SaveFileAction : public Action
{
public:
    virtual ~SaveFileAction(){};

    void execute(Editor& editor) override
    {
        std::ofstream file{m_filename.data(), std::ios_base::trunc};
        file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

        for (const auto& row : editor.text())
        {
            file << row;
        }
    }

    std::string_view m_filename;
};
}

#endif  // SAVE_FILE_ACTION_H