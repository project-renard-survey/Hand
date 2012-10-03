/*
 *  Copyright 2012 Alex Stefaniuc
 *
 *  This file is part of Hand.
 *
 *  Hand is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3
 *  of the License, or (at your option) any later version.
 *
 *  Hand is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with Hand. If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/handserver.h"
#include "view/layer/layer.h"
#include "view/layer/layermanager.h"
#include "input/device.h"
#include "input/inputstate.h"
#include "view/screen.h"
#include "graph/data.h"


using namespace std;


HandServer* HandServer::Instance = NULL;


HandServer* HandServer::GetInstance()
{
    if(Instance == NULL)
        Instance = new HandServer();
    return Instance;
}


HandServer::HandServer()
{
    ExecNotFinished = false;
    // DrawObject
    CreateNewUserOnOrphanKeyPress = true;
    DeleteDeviceIfEmpty = true;
    Timer = NULL;
    _Screen = new Screen();
}


HandServer::~HandServer()
{
    delete _Screen;
}


void HandServer::Pump()
{
    // Executed 25x per sec
    if(ExecNotFinished)
        return;
    ExecNotFinished = true;
    // Wait till next cycle before setting the next content
    // because this deletes the calling object
    GetUserInput();
    if(!_Screen->ShowSurface())
        // Nothing to show, the normal exit
        exit(0);

    ExecNotFinished = false;
}


LayerManager* HandServer::GetLayerManager()
{
    // Start the layer manager
    LayerManager* layer_mgr = new LayerManager();
    _Screen->Vertex::Get(LAYERMANAGER)->Add(layer_mgr);
//    layer_mgr->Init();

    // Create device object with input state
    Device* device = new Device();
    layer_mgr->SetDevice(device);
    Get("Devices")->Add(device);
    layer_mgr->SetScreen(_Screen->GetSurface());

    return layer_mgr;
}


bool HandServer::Present(string file)
{
    Vertex* app = Produce(new Note("Command line input", file), "");
    if(app && app->Is(HANDAPP))
    {
        GetLayerManager()->LoadAppInterface(app, true);
        return true;
    }
    return false;
}


// -----------------------------------------------
// Device Manager


// C Methods

// Removes all devices but the keyboard from the event queue
int EventFilter(const SDL_Event *event)
{
    if((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP))
        return 1;
    return 0;
}

// C++ methods can't be called directly as a callback
Uint32 CallServerMethod(Uint32 i, void* server)
{
    HandServer* hand = reinterpret_cast<HandServer*>(server);
    hand->Pump();
    return i;
}


void HandServer::Beat()
{
    // Start only once
    if(Timer != NULL)
        return;
    // Only for speed
    SDL_SetEventFilter(EventFilter);
    // 25 pix per sec
    Uint32 interval = 1000/25;
    Timer = SDL_AddTimer(interval, &CallServerMethod, (void*)(this));
    if(Timer == NULL)
        exit(1);
    // Stop the main execution line
    pause();
}


void HandServer::StopBeating()
{
    // TODO: probably never ever used
    if(Timer != NULL)
        SDL_RemoveTimer(Timer);
}


void HandServer::GetUserInput()
{
    while(SDL_PollEvent(&Event))
    {
        switch(Event.type)
        {
            case SDL_KEYDOWN:
                Press(Event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                Release(Event.key.keysym.sym);
                break;
        }
    }
}


void HandServer::Press(SDLKey k)
{
    Vertex* all_dev = Get("Devices");
    Device* dev;
    uint i = 0;
    while((dev=dynamic_cast<Device*>(all_dev->Get(++i))) != NULL)
        if(dev->Press(k))
            return;

    if(CreateNewUserOnOrphanKeyPress)
        // Create a new device if the pressed key doesn't fits to an existing one
        GetLayerManager()->GetDevice()->Press(k);
}


void HandServer::Release(SDLKey k)
{
    Vertex* lm = _Screen->Vertex::Get(LAYERMANAGER);
    // Gets the device
    Vertex* all_dev = Get("Devices");
    Device* dev;
    uint i = 0;
    while((dev=dynamic_cast<Device*>(all_dev->Get(++i))) != NULL)
    {
        if(dev->Release(k))
        {
            if(DeleteDeviceIfEmpty
               && dev->IsUnused()
               && (lm->Size() > 1))
            {
                // Exit last layer
                delete(lm->Get(lm->Size()));
            }
            return;
        }
    }
}
// End Device Manager
