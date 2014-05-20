//
//  TextureSelector.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-19.
//
//

#pragma once

#include "Interface.h"
#include "cinder/gl/Texture.h"
#include <vector>

class TextureSelector
{
public:
    TextureSelector() : mCurTextureIndex(0) {}
    ~TextureSelector() {}
    
    void addTexture( const std::string& name, const std::string& filename );
    void addTexture( const std::string& name, const std::string& filename, ci::gl::Texture::Format format );
    void addTexture( const std::string& name, ci::gl::Texture tex );
    void setupInterface( Interface* interface, const std::string& prefix, const std::string &name );
    
    ci::gl::Texture& getTexture() { return mTextures[mCurTextureIndex].second; }
    
private:
    typedef std::pair<std::string, ci::gl::Texture> tNamedTexture;
    std::vector<tNamedTexture> mTextures;
    int mCurTextureIndex;
};
