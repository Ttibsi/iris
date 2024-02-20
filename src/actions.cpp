#include "actions.h"
#include "viewport.h"

ActionController::ActionController(Viewport* v) : view(v) {}

void ActionController::action_event(const Action a){
    switch (a) {
        case Action::Unknown:
            break;
        case Action::EnterInsertMode:
            view->switch_to_insert();
            break;
        case Action::EnterNormalMode:
        case Action::EnterCommandMode:
          break;
        };
}

void ActionController::action_events(const std::span<Action> actions) {
    for (auto&& a: actions) { ActionController::action_event(a); }
    // redraw after all events
}

