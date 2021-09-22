#ifndef QUIT_ACTION_H
#define QUIT_ACTION_H

#include "action.h"
#include "../editor.h"

namespace Pilo
{
class QuitAction : public Action
{
public:
    virtual ~QuitAction(){};

    void execute(Editor& editor) override
    {
        editor.die();
    }
};
}

#endif  // QUIT_ACTION_H