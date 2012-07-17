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

#ifndef HAND_LAYOUTS_H
#define HAND_LAYOUTS_H

#include "factory.h"


using namespace std;


#define BGRD_TYPE_SOLID_COLOR "BGRD_TYPE_SOLID_COLOR"
#define BGRD_TYPE_IMAGE "BGRD_TYPE_IMAGE"


#define TYPE_DRAWOBJ_DRAWER "Drawer"


class Theme;
class VirtualSurface;

class DrawerFactory : public Factory
{
    public:
        DrawerFactory(Functoid* draw_func);
        ~DrawerFactory();
        Functoid* Produce(Functoid* ignore);
        void TakeBack(Functoid* product){};
        bool IsValidInput(Functoid* input);
    protected:
        Functoid* DrawFunction;
};

#endif /* HAND_LAYOUTS_H */