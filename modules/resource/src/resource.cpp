#include "resource/resource.hpp"

void Resource::unload()
{
    if(state != State::Loaded)
    {
        return;
    }
    
    doUnload();
    state = State::Released;
}