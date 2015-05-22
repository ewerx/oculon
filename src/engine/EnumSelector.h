//
//  EnumSelector.h
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-20.
//
//

#pragma once

#include <vector>

class EnumSelector
{
public:
    EnumSelector() : mIndex(0)
    {}
    
    /// return current index
    int operator()() const              { return mIndex; }
    
    /// return current name
    const std::string name() const {
        if (mIndex < mNames.size()) {
            return mNames[mIndex];
        }
        return "none";
    }
    
    void addValue(const std::string& name) {
        mNames.push_back(name);
    }
    
protected:
    std::vector<std::string> mNames;
    int mIndex;
};

