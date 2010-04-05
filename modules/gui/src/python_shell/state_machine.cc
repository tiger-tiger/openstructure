#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include "state.hh"
#include "state_machine.hh"

namespace ost { namespace gui {

StateMachine::StateMachine(QObject* parent):
  QObject(parent)
{
  parent->installEventFilter(this);
}

bool StateMachine::eventFilter (QObject * watched, QEvent * event)
{
  if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
    QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
    return active_state_->checkEvent(key_event);
  } else if(event->type() == QEvent::MouseButtonDblClick ||
            event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseMove){
    QMouseEvent *mouse_event = dynamic_cast<QMouseEvent*>(event);
    return active_state_->checkEvent(mouse_event);
  } else {
    return false;
  }
}

void StateMachine::addState(State* state)
{
  state->setParent(this);
}
void StateMachine::setInitialState(State* state)
{
  active_state_=state;
}
void StateMachine::start()
{
  active_state_->onEntry();
}
bool StateMachine::isActive(State* state)
{
  return state==active_state_;
}
void StateMachine::setActive(State* state)
{
  active_state_->onExit();
  active_state_=state;
  active_state_->onEntry();
  active_state_->checkAutomaticTransitions();
}

}}//ns
