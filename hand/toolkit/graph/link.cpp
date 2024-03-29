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

#include "graph/link.h"
#include "graph/search/search.h"


using namespace std;


Link::Link(string name) : List(name)
{
    type(LINK);
}


bool Link::isOpen(Search* search)
{
    if(!Vertex::isOpen(search))
        return false;
    // Does it match in its role as relation
    RegularExpression* se = search->GetSearchLink();
    if(se && !se->Matches(name()))
        // Don't look further if it's not the right relation
        return false;
    return true;
}
