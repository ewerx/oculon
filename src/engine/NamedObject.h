//
//  NamedObject.h
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-01-06.
//
//

#pragma once

#include <stdio.h>
#include <unordered_map>
#include <algorithm>

namespace oculon
{
    template<class T>
    class NamedObject
    {
    public:
        using Ref = std::shared_ptr<T>;
        using NamedObjectMap = std::unordered_map<std::string, Ref>;
        
        NamedObject( const std::string& name ) : mName(name)
        {
        }
        
        const std::string& getName() const { return mName; }
        
    private:
        std::string mName;
    };
    
    template <typename T>
    bool vectorContains( const std::vector<T>& vec, const T& element )
    {
        return (std::find(vec.begin(), vec.end(), element) != vec.end());
    }
    
    template <typename T>
    void vectorRemove( std::vector<T>& vec, const T& element )
    {
        vec.erase(std::remove(vec.begin(), vec.end(), element));
    }
}