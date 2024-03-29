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

#include <SDL/SDL.h>
#include "input/device.h"
#include "input/inputstate.h"
#include "graph/method.h"
#include "view/datatypes/layout.h"


using namespace std;


Device::Device() : HandApp("settings:Keyboard::0")
{
    type(DEVICE);
    StateMachine = NULL;
    numberOfKeys = NUMBER_OF_BUTTONS;
}


Device::~Device()
{
    delete(StateMachine);
}


Vertex* Device::get(string name)
{
    Vertex* ret = List::get(ANY, name);
    if(ret || (name!=VIEW))
        return ret;

    ret = List::get(VIEW);
    // Two main entries: a keylist and the description
    ret->add(new Note(DESCRIPTION, "Press 5 keys on the keyboard"));

    Vertex* keys_data_tree = Vertex::get(KEYLIST);
    Vertex* keys_view_tree = ret->get(KEYLIST);

    keys_view_tree->Vertex::get(LAYOUT)->get(LIST)->set(
            new LayoutFactory<Device>(LIST, this, &Device::GetKeyListLayout));

    // Load un-initialized keys
    Vertex* factory = Vertex::get(FACTORY, THEMES)->get(DEFAULT)
                            ->get(LAYOUT)->get(LIST)->get(FRAMEDLIST);
    Vertex* id;
    Vertex* frame;
    for(uint i=0; i < numberOfKeys; ++i)
    {
        id = new Note("Keydata", "");
        keys_data_tree->add(id);

        frame = new List("");
        frame->Vertex::get(LAYOUT)->get(LIST)->set(factory);
        frame->add(id);
        keys_view_tree->add(frame);
    }
    return ret;
}


bool Device::GetKeyListLayout(Vertex* layout)
{
    if(Vertex::get(FACTORY, THEMES)->get(DEFAULT)->get(LAYOUT)->get(LIST)->execute(layout))
    {
        layout->get(COORDINATES)->Vertex::get(REQUEST)->get(RECT)->get()->name(SCALED);
        layout->get(ALIGNMENT)->Vertex::get(REQUEST)->get(ALIGNMENT)->get()->name(HORIZONTAL);
        return true;
    }
    return false;
}


bool Device::Init()
{
    string  key_str;
    Note* data;

    for(uint i=1; i<=numberOfKeys; i++)
    {
        // Do we have keys to load?
        data = GetKey(i);
        if(!data)
            return false;
        key_str = data->get();
        if(key_str == "")
            return false;
        // Translate list entries to device keys
        Keys.push_back(atoi(key_str.c_str()));
    }
    return true;
}


bool Device::Press(int k)
{
    if(Keys.size() < numberOfKeys)
        // Device is during initialization
        AddKey(k);

    int index = GetKeyIndex(k);
    if(index == -1)
        return false;

    StateMachine->Press(index);
    return true;
}


bool Device::Release(int k)
{
    int index = GetKeyIndex(k);
    if(index == -1)
        return false;

    if(Keys.size() < numberOfKeys)
    {
        // Device is during initialization
        DeleteKey(index);
        StateMachine->reset();
        for(uint i=1; i<=Keys.size(); ++i)
            StateMachine->Press(i);
        return true;
    }
    return StateMachine->Release(index);
}


int Device::GetKeyIndex(int k)
{
    int i = 1;
    for(currentKey=Keys.begin(); currentKey!=Keys.end(); currentKey++, i++)
        if((*currentKey) == k)
            return i;

    return -1;
}


Note* Device::GetKey(uint pos)
{
    return dynamic_cast<Note*>(Vertex::get(KEYLIST)->get(pos));
}


bool Device::IsUnused()
{
    if(Keys.size() == 0)
        return true;
    return false;
}


void Device::AddKey(int k)
{
    Keys.push_back(k);
    // Display the key on the initialization screen
    int index = GetKeyIndex(k);
    currentKey = Keys.begin() + index - 1;

    GetKey(index)->set(SDL_GetKeyName((SDLKey)(*currentKey)));
}


void Device::DeleteKey(uint index)
{
    Note* curr;
    Note* next;
    uint i = index;
    string key;
    while((curr=GetKey(i)) != NULL)
    {
        next = GetKey(++i);
        if(next)
            key = next->get();
        else
            key = "";
        curr->set(key);
    }

    currentKey = Keys.begin() + index - 1;
    if(currentKey != Keys.end())
        currentKey = Keys.erase(currentKey);
}


uint Device::GetNumberOfKeys()
{
    return numberOfKeys;
}


InputState* Device::GetInputState()
{
    // Create inputstate object
    if(StateMachine == NULL)
        StateMachine = new InputState(this);
    return StateMachine;
}
