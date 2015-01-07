//
//  NamedObject.h
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-01-06.
//
//

#include <stdio.h>
#include <unordered_map>

namespace oculon
{
    template<class T>
    class NamedObject
    {
    public:
        typedef std::shared_ptr<T> Ref;
        typedef std::unordered_map<std::string, Ref> NamedObjectMap;
        
        NamedObject( const std::string& name ) : mName(name)
        {
        }
        
        const std::string& getName() const { return mName; }
        
    private:
        std::string mName;
    };
}