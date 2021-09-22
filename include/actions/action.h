#ifndef ACTION_H
#define ACTION_H

namespace Pilo
{
class Editor;
class Action
{
public:
    virtual ~Action(){};
    virtual void execute(Editor& editor) = 0;
};
}

#endif  // ACTION_H