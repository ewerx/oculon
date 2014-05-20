//
//  TextureSelector.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-19.
//
//

#include "TextureSelector.h"
#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void TextureSelector::addTexture(const std::string &name, const std::string &filename)
{
    mTextures.push_back( make_pair( name, gl::Texture( loadImage( loadResource( filename ) ) ) ) );
}

void TextureSelector::addTexture(const std::string &name, const std::string &filename, ci::gl::Texture::Format format)
{
    mTextures.push_back( make_pair( name, gl::Texture( loadImage( loadResource( filename ) ), format ) ) );
}

void TextureSelector::addTexture(const std::string &name, ci::gl::Texture tex)
{
    mTextures.push_back( make_pair( name, tex ) );
}

void TextureSelector::setupInterface(Interface *interface, const std::string &prefix, const std::string &name)
{
    vector<string> texNames;
    for( tNamedTexture namedTex : mTextures )
    {
        texNames.push_back(namedTex.first);
    }
    interface->addEnum(CreateEnumParam( name, (int*)(&mCurTextureIndex) )
                       .maxValue(texNames.size())
                       .oscReceiver(prefix)
                       .isVertical(), texNames);
}
