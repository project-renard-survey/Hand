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


#ifndef GRAPH_SEARCH_H
#define GRAPH_SEARCH_H

#include "graph/vertex.h"
#include "graph/search/regularexpression.h"


class SearchCookie;
class Pool;

class Search : public Vertex
{
    public:
        Search(std::string name);
        virtual ~Search();

    public:
        // Overloaded base class methods
        void reset();
        bool execute(Vertex* target);

        // Search setters and getters
        // The setters are called from "outside", from the application code
        void SetSearchname(std::string s, bool make_regex=false);
        // The getters are called from inside, the Find methods
        RegularExpression* GetSearchName();
        void SetSearchType(std::string s, bool make_regex=false);
        RegularExpression* GetSearchType();
        void SetSearchLink(std::string s, bool make_regex=false);
        RegularExpression* GetSearchLink();
        std::string GetCookieName();
    protected:
        bool Matches(Vertex* target);
        bool Step(Vertex* path);
        bool SearchAllChilds(Vertex* path_end);
        RegularExpression* AddSearchRegex(std::string name,
                                          std::string s,
                                          bool make_regex);
    private:
        // Quick links
        RegularExpression* SearchName;
        RegularExpression* SearchType;
        RegularExpression* SearchLink;
        Vertex* Findings;
    public:
        // TODO: GUI display?
        uint MaxDepth;
        bool MultipleFinds;
};

#endif /* GRAPH_SEARCH_H */
