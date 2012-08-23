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

#ifndef HAND_FUNCTOID_H
#define HAND_FUNCTOID_H

#include "include/defines.h"

#include <string>
#include <sstream>
#include <typeinfo>
#include <vector>


// Arbitrary search depth (should be big enough to find everything
// and small enough to return fast from circles)
#define FUNCTOID "Functoid"
#define MAX_SEARCH_DEPTH 1024


class FunctoidSearch;
class SearchExpression;

class Functoid : public std::vector<Functoid*>
{
    public:
        Functoid(std::string name);
        virtual ~Functoid();

        // Inserts a sub-item to the current functoid and makes the current
        // functoid the owner of it => 'child' object gets deleted when
        // the current functoid gets deleted; allows multiple objects with
        // the same name
        virtual bool Add(Functoid* child);
        // Inserts a sub-item to the current functoid without changing
        // the ownership of 'item'; it replaces any object with same name
        virtual bool Set(Functoid* sub);
        // Appends a sub-item to the current functoid without changing
        // the ownership of 'item'; allows multiple objects with same name
        virtual bool Attach(Functoid* sub);

        // Removes the reference to 'item' but keeps item alive
        virtual bool Detach(Functoid* item);
        // Removes the reference to 'item' and deletes the object if it
        // is the owner
        virtual bool Delete(Functoid* child);

        // Get the first sub-item by name.
        // If the item doesn't exist it returns a new Relation functoid;
        // use Get(ANY, name) to test if a functoid exists
        virtual Functoid* Get(std::string name);
        // Get the first sub-item by name and type; if the item is not found
        // it returns NULL; the parameters can be set to ANY e.g.
        // Get(ANY, ANY) returns the first stored item
        virtual Functoid* Get(std::string name, std::string type);
        // Interface to method defined in a derived class; returns always
        // NULL in this base class
        virtual Functoid* Get(uint item);

        // Simple iterative deepening depth-first search.
        // Omits OWNER from the search but might fail with any other graph
        // cycles, use the FunctoidSearch engine in case of doubt.
        virtual Functoid* Find(std::string name, int max_depth = 2);
        // Flat search with support for (boost-typed) regular expressions
        // TODO: make it iterative or rename to it "Get()"?
        virtual Functoid* Find(SearchExpression* expression);

        // Set "name" with or without uri scheme
        virtual void SetName(std::string name);
        // Get the name without uri scheme
        virtual std::string& GetName();
        // Get the name with uri scheme
        virtual std::string GetUriString();

        virtual void SetType(std::string type);
        // Returns the last set type
        virtual std::string GetType();
        virtual bool IsType(std::string type);
        virtual bool IsType(SearchExpression* type);

        // Set object owner (for memory management).
        // Owner is stored under OWNER
        virtual void SetOwner(Functoid* owner);
        // Returns true if 'caller' is the owner or if no owner is registered
        virtual bool HasOwner(Functoid* caller);

        // Interface to methods defined in a derived class
        virtual bool Execute(Functoid* func_param);
        virtual void Reset(){};

        // Helper method for the search engine
        virtual bool IsOpen(FunctoidSearch* search);
        // TODO:
//        virtual string GetAsString() = 0;
//        virtual bool NotifyChanged();

    protected:
        virtual Functoid* _Find(std::string name, int depth);

        std::string Name;
};

typedef std::vector<Functoid*>::iterator FunctoidIterator;


#define FUNCTOIDDATA "FUNCTOIDDATA"
// TODO: This string varies on different platforms
#define DATA_STRING "Ss"

template <typename I>
class Data : public Functoid
{
    public:
        Data(std::string name, I val) : Functoid(name)
        {
            Value = val;
            SetType(std::string(typeid(val).name()));
        };

        bool Set(I val)
        {
            Value = val;
            return true;
        };
        I Get()
        {
            return Value;
        };
        std::string GetAsString()
        {
            std::ostringstream s;
            s << Value;
            return s.str();
        };
   protected:
        I Value;
};

typedef Data<std::string> Note;


template <class I>
class Callback : public Functoid
{
    typedef bool (I::*TFunction)(Functoid*);

    public:
        Callback(std::string name, I* obj, TFunction func) : Functoid(name)
        {
            Object = obj;
            Function = func;
            SetType(BUTTON_ITEM);
        };
        // Clean-up object
        void Clean(void)
        {
            delete(Object);
        };

        // Execute the Callback
        bool Execute(Functoid* param)
        {
            if(Function)
                return (Object->*Function)(param);
            return false;
        };

        void Set(TFunction func)
        {
            Function = func;
        };
        virtual I* Get()
        {
            // Only makes sense to return the object
            return Object;
        };

        std::string GetAsString()
        {
            // TODO: what kind of a string to create: for the GUI? for persistent storage? for both?
            return "";
        };
    private:
        TFunction Function;
        I* Object;
};

#endif /* HAND_FUNCTOID_H */