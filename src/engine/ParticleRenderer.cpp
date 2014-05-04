//
//  ParticleRenderer.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2/20/2014.
//
//

#include "ParticleRenderer.h"
#include "Utils.h"
#include <vector>

using namespace ci;
using namespace std;

ParticleRenderer::ParticleRenderer(const std::string& name)
: mName(name)
{
}

ParticleRenderer::~ParticleRenderer()
{
}

void ParticleRenderer::setupVBO(int fboSize, GLuint primitiveType)
{
    unsigned int numVertices = fboSize * fboSize;
    vector<Vec2f> texCoords;
    vector<uint32_t> indices;
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    layout.setStaticNormals();
    
    mVboMesh = gl::VboMesh( numVertices, numVertices, layout, primitiveType);
    for( int y = 0; y < fboSize; ++y ) {
        for( int x = 0; x < fboSize; ++x ) {
            indices.push_back( y * fboSize + x );
            texCoords.push_back( Vec2f( x/(float)fboSize, y/(float)fboSize ) );
        }
    }
    mVboMesh.bufferIndices( indices );
    mVboMesh.bufferTexCoords2d( 0, texCoords );
    mVboMesh.unbindBuffers();
}

void ParticleRenderer::draw(PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera &cam, AudioInputHandler& audioInputHandler, float gain)
{
    // pre-render - set state
    gl::pushMatrices();
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
    
    gl::enableAdditiveBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enable(GL_TEXTURE_2D);
    
    gl::setMatrices( cam );
    
    // bind textures
    particlesFbo.bindTexture(0);//pos
    particlesFbo.bindTexture(1);//vel
    particlesFbo.bindTexture(2);//info
    
    // bind shader
    mShader.bind();
    mShader.uniform("posMap", 0);
    mShader.uniform("velMap", 1);
    mShader.uniform("information", 2);
    
    // do magic
    gl::draw( mVboMesh );
    
    // cleanup
    mShader.unbind();
    particlesFbo.unbindTexture();
    
    // post-render - restore state
    glPopAttrib();
    gl::popMatrices();
}
