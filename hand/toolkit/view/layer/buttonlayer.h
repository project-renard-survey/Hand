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

#ifndef HAND_BUTTONLAYER_H
#define HAND_BUTTONLAYER_H

#include "view/layer/layer.h"


class ButtonLayer : public Layer
{
    public:
        ButtonLayer();
        ~ButtonLayer(void);
        void SetContent(Functoid*);
        // Reacting on user input
        void PressGui(void);
        void ReleaseGui(void);
    protected:
        virtual void Configure(Functoid* parent_layout);

    private:
        void SetExpandable(void);
        bool ExpandList(Functoid* ignore);

        uint VisibleItems;
        Functoid* FunctionBox;
};


class ButtonLayerFactory : public Factory
{
    public:
        ButtonLayerFactory() : Factory(LAYER_TOPOGRAPHY,
                                       "",
                                       BUTTONLAYER, ""){};
        ~ButtonLayerFactory(){};

        bool IsValidInput(Functoid* input);

        Functoid* Produce(Functoid* input);
        void TakeBack(Functoid* product);
};

#endif /* HAND_BUTTONLAYER_H */