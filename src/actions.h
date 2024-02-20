#ifndef ACTIONS_H
#define ACTIONS_H

#include <span>

enum class Action {
    Unknown,
    EnterInsertMode,
    EnterNormalMode,
    EnterCommandMode
};

struct Viewport;
struct ActionController{
    Viewport* view;

    ActionController(Viewport*);
    void action_event(const Action);
    void action_events(const std::span<Action>);
};

#endif //ACTIONS_H
