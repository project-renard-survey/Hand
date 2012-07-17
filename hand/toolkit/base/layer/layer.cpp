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

#include "layer.h"
#include "theme.h"
#include "node.h"
#include "rect.h"
#include "drawer.h"
#include "layout.h"
#include "layermanager.h"


using namespace std;


Layer::Layer() : VirtualSurface("Layer")
{
    ParentLayer = NULL;
    Set(new Link("Content", "Link"));
    Updated = false;
    IsVisible = true;
    IsExpanded = false;
    _Node = NULL;
    SetType(TYPE_LAYER);
}


Layer::~Layer()
{
//    _Theme->TakeBack(Get("Layout"));
    // "Detach" content
    Get("Content")->Set(NULL);
    ReleaseCommand();
    Collapse();
}


bool Layer::Exit()
{
    // Check if the default location is currently active
    return ParentLayer->Exit();
}

/*
bool Layer::Update(bool forced)
{
    if(!IsVisible)
    {
        return false;
    }
    if(_Layout->Update(forced))
    {
        forced = true;
    }
    return forced;
}
*/

bool Layer::Update(bool forced)
{
    if(Animations.Update(this))
        Changed = true;

    if(Changed || forced)
        Draw(forced);

    return Changed;
}


void Layer::SetContent(Functoid* data)
{
    // ReleaseContent
    Get("Content")->Set(data);
    Changed = true;
}


Functoid* Layer::GetContent()
{
    return Get("Content")->Get(1);
}


void Layer::SetCommand(Node* cmd)
{
    ReleaseCommand();
    if(cmd != NULL)
    {
        _Node = cmd;
        _Node->SetLayer(this);
    }
}


void Layer::ReleaseCommand()
{
    if(_Node != NULL)
    {
        //_Node_->ReleaseList();
        _Node->SetLayer(NULL);
        _Node = NULL;
    }
}


void Layer::Collapse()
{
    vector<Layer*>::iterator curr = SubLayers.begin();
    Layer* tmp;
    // Recursively deletes sub-layers
    while(curr!=SubLayers.end())
    {
        tmp = (*curr);
        curr = SubLayers.erase(curr);
        delete tmp;
    }
    IsExpanded = false;
}


// --- Getter and setter for external objects ---
void Layer::SetParentLayer(Layer* l)
{
    ParentLayer = l;
}

// TODO: any use for this?
Layer* Layer::GetParentLayer()
{
    return ParentLayer;
}


void Layer::PressGui()
{
    // TODO: modify layout (IsPressed = true)
    //Draw();
}

void Layer::ReleaseGui()
{
    // TODO: reset layout (IsPressed = false)
    //Draw();
}


void Layer::NotifyChanged()
{
    Changed = true;
}


uint Layer::GetLevel()
{
    if(ParentLayer != NULL)
        return (1 + ParentLayer->GetLevel());
    return 0;
}


bool Layer::Request(Functoid* req)
{
    return ParentLayer->Request(req);
}


Layer* Layer::Insert(Functoid* data, string position)
{
    // TODO: check if there is already a Layer at searched
    // position and if it has the right type
    // TODO: delete / (re)create layer if needed
    Layout* layout = dynamic_cast<Layout*>(Get("Layout")->Get(1));
    // Is the layer type restricted by the current layout?
    if(!layout)
        return NULL;

    FunctoidNode* field = layout->GetField(position);
    if(!field)
        return NULL;

    Layer* sub_layer = GetLayerManager()->CreateLayer(data, field->GetType());
    if(!sub_layer)
        return NULL;

    // Connect the components
    // on Layer/VS level
    Add(TAG_RELATION_CHILD, sub_layer);
    sub_layer->Set(TAG_RELATION_PARENT, this);
    sub_layer->SetParent(this);
    // on Layout level
    Layout* field_layout = dynamic_cast<Layout*>(field->Get("Layout")->Get(1));
    if(field_layout)
        field_layout->AddForUpdate(sub_layer->Get("Layout")->Get(1));

    return sub_layer;
}


Layer* Layer::GetAttachedLayer(Functoid* target)
{
    FunctoidSearch search;
    search.MaxDepth = 2;
    search.SetSearchType(TYPE_LAYER);
    search.SetSearchRelation(TAG_RELATION_LAYER);
    if(search.Search(target))
        return dynamic_cast<Layer*>(search.GetFindings());
    return NULL;
}


void Layer::SetTheme(Theme* theme)
{
    // TODO: generic recursive Set() on a sub-tree defined by a "relation"
    // (possibly using Search() with MultipleFindings)
    SetLayout(theme);
    Functoid* children = Get(TAG_RELATION_CHILD);
    if(!children)
        return;

    Layer* layer;
    Functoid* child;
    uint i = 0;
    while((child=children->Get(++i)) != NULL)
    {
        layer = dynamic_cast<Layer*>(child);
        if(layer)
            layer->SetTheme(theme);
    }
}


void Layer::SetLayout(Functoid* drawer_lib) // string type, uint position
{
    // Delete from the layout items owned by the previous theme
    Functoid* layout = Get("Layout");
    if(!layout)
    {
        layout = new Layout("Layout", GetType()+"_Layout");
        Set("Layout", layout);
    }
    else
        layout = layout->Get(1);
//        _Theme->TakeBack(layout);
    if(!drawer_lib->Execute(layout))
    {
        FunctoidSearch search;
        search.SetSearchName(GetType());
        search.SetSearchType(TYPE_DRAWOBJ_DRAWER);
        search.MaxDepth = 4;
        if(search.Search(drawer_lib))
            layout->Set(search.GetFindings());
    }
}


void Layer::Draw(bool forced)
{
/*  if(BufferType == NONE)
    {
        // TODO: this crashes in LayerManager during "write through"
        Parent->MapSurface(_Layout->SizeAndPosition,
                            SizeAndPositionOnBuffer,
                            GetBuffer());
        Updated = true; // ?
    }*/
    // Call the Theme function for drawing with the current settings
    Layout* layout = dynamic_cast<Layout*>(Get("Layout")->Get(1));
    if(!layout)
        return;

    layout->Execute(this);

    if(BufferType == COLLECTOR)
        // Draw first the child on the buffer
        DrawChilds(forced);

    Rect tmp;
    Rect* sap = GetRect(GUI_RECT_SIZEANDPOS, layout);
    if(sap)
        tmp.Init(sap->x, sap->y, sap->w, sap->h);

    Show(&SizeAndPositionOnBuffer, &tmp);

    if(BufferType == OVERLAY)
    {
        // Draw childs afterwards (not buffered)
        DrawChilds(forced);
//        Updated = true;
    }
    // Clean-up temporary size and position values
    layout->Reset();
}


void Layer::DrawChilds(bool forced)
{
    Functoid* children = Get(TAG_RELATION_CHILD);
    if(!children)
        return;

    Layer* layer;
    Functoid* child;
    uint i = 0;
    while((child=children->Get(++i)) != NULL)
    {
        layer = dynamic_cast<Layer*>(child);
        if(!layer)
            continue;
        layer->SetSize(GetSize());
        layer->Update(forced);
    }
}