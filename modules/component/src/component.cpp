#include "component/component.hpp"

void Component::initialize()
{
    if(state == State::Uninitialized)
    {
        state = State::Initializing;
        onInitialize();
        state = State::Active;
    }
}