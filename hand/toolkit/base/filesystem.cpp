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

#include "base/filesystem.h"
#include "base/handserver.h"
#include "graph/data.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>


using namespace std;
using namespace boost::filesystem;


bool FileVertex_Factory::IsValidInput(Vertex* input)
{
    // Very basic check here
    Note* d_path = dynamic_cast<Note*>(input);
    if(!d_path)
        return false;
    string s_path = d_path->Get();
    if(s_path.find(URI_FILE) == 0)
        return true;
    return false;
}


Vertex* FileVertex_Factory::Produce(Vertex* descriptor)
{
    Note* d_path = dynamic_cast<Note*>(descriptor);
    if(!d_path)
        return NULL;
    return new FileVertex(d_path->Get());
}


void FileVertex_Factory::TakeBack(Vertex* product)
{
    if(dynamic_cast<FileVertex*>(product))
        delete(product);
}


// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------


FileVertex::FileVertex(string file_name) : List(file_name)
{
    SetType(FILEVERTEX);
    if(file_name.find(URI_FILE) == 0)
        // Remove the URI string from the name
        file_name.erase(0, sizeof(URI_FILE)-1);

    path file_path(file_name);
    if(is_regular_file(file_path))
    {
        if(file_path.has_parent_path())
            Get(RELATION_PARENT_PATH)->Set(new FileVertex(file_path.parent_path().string()));

        Name(file_path.filename().string());
    }
    else // is directory
        Name(file_name);
}


string FileVertex::GetFullPath()
{
    return GetPath().string();
}


path FileVertex::GetPath()
{
    path _path;
    // Get directory information from parent path
    Vertex* dir_rel = Get(RELATION_PARENT_PATH);
    if(dir_rel)
    {
        FileVertex* parent_path = dynamic_cast<FileVertex*>(dir_rel->Get());
        if(parent_path)
            _path = parent_path->GetPath();
    }
    // Check if it's still a relative path
    _path /= Name();
    if(_path.is_relative())
        system_complete(_path);

    return _path;
}


// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------


DirectoryLoader::~DirectoryLoader()
{
//    Unload();
}


bool  DirectoryLoader::IsValidInput(Vertex* entry)
{
    FileVertex* ff = dynamic_cast<FileVertex*>(entry);
    if(ff && is_directory(ff->GetPath()))
        return true;
    return false;
}


Vertex*  DirectoryLoader::Produce(Vertex* dir_obj)
{
    Vertex* ret = NULL;
    FileVertex* ff_dir = dynamic_cast<FileVertex*>(dir_obj);
    if(!ff_dir)
        return ret;
    path dir_path = ff_dir->GetPath();
    if(!is_directory(dir_path))
        return ret;

    directory_iterator end;
    // (Re-)read the themes in
    ff_dir->Reset();
    FileVertex* file;
    for(directory_iterator dir_it(dir_path); dir_it!=end; dir_it++)
    {
        if(is_regular_file(dir_it->status()))
        {
            if(dir_it->path().extension() == LIBRARY_FILE_EXTENSION)
            {
                file = new FileVertex(dir_it->path().filename().string());
                file->Get(RELATION_PARENT_PATH)->Set(ff_dir);
                ff_dir->Add(file);
                // Set return value != NULL
                ret = dir_obj;
            }
        }
        else if(is_directory(dir_it->status()))
        {
            // TODO: check or add dir
        }
    }
    return ret;
}


void DirectoryLoader::TakeBack(Vertex* dir_obj)
{
    FileVertex* ff_dir = dynamic_cast<FileVertex*>(dir_obj);
    if(ff_dir && is_directory(ff_dir->GetPath()))
        ff_dir->Reset();
}

