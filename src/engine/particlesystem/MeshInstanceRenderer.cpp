//
//  MeshInstanceRenderer.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-18.
//
//

#include "MeshInstanceRenderer.h"
#include "MeshHelper.h"
#include "Utils.h"
#include "cinder/Camera.h"

using namespace ci;
using namespace ci::app;
using namespace std;


MeshInstanceRenderer::MeshInstanceRenderer()
: ParticleRenderer("meshes")
{
    // params
    mAdditiveBlending       = true;
    mColor                  = ColorAf( 0.5f, 0.5f, 0.6f, 0.5f );
    mAudioReactive          = true;
    mWireframe              = false;
    
    mMeshType               = 0;
    
    // load textures
    mCurPointTexture        = 0;
    gl::Texture pointTex;
    gl::Texture::Format format;
    format.setWrap( GL_REPEAT, GL_REPEAT );
    
    pointTex = gl::Texture( loadImage( loadResource( "particle_white.png" ) ), format );
    mPointTextures.push_back( make_pair("glow", pointTex) );
    pointTex = gl::Texture( loadImage( loadResource( "glitter.png" ) ), format );
    mPointTextures.push_back( make_pair("solid", pointTex) );
    pointTex = gl::Texture( loadImage( loadResource( "parsec-sparkle.png" ) ), format );
    mPointTextures.push_back( make_pair("sparkle", pointTex) );
}

MeshInstanceRenderer::~MeshInstanceRenderer()
{
    ParticleRenderer::~ParticleRenderer();
}

void MeshInstanceRenderer::setup(int fboSize)
{
    // load shader
    mShader = Utils::loadVertAndFragShaders("meshinstance_render_vert.glsl",  "meshinstance_render_frag.glsl");
    
    // setup VBO
    //setupVBO(fboSize, GL_POINTS);
    createMeshes();
}

void MeshInstanceRenderer::createMeshes()
{
    gl::VboMesh icosahedron	= gl::VboMesh( MeshHelper::createIcosahedron( 1 ) );
    mMeshes.push_back( make_pair( "icosahedron", icosahedron ) );
    
//	gl::VboMesh cube = gl::VboMesh( MeshHelper::createCube( Vec3i( 4, 4, 4 ) ) );
//    mMeshes.push_back( make_pair( "cube", cube ) );
//    
//    gl::VboMesh sphere = gl::VboMesh( MeshHelper::createSphere( Vec2i( 8, 8 ) ) );
//    mMeshes.push_back( make_pair( "sphere", sphere ) );
//    
//    gl::VboMesh torus = gl::VboMesh( MeshHelper::createTorus( Vec2i( mResolution, mResolution ) ) );
//    mMeshes.push_back( make_pair( "torus", torus ) );
}

void MeshInstanceRenderer::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + mName;
    
    interface->gui()->addLabel(getName());
    interface->addParam(CreateBoolParam("meshes/audioreactive", &mAudioReactive)
                        .oscReceiver(oscName));
    
    interface->addParam(CreateColorParam("meshes/color", &mColor, kMinColor, kMaxColor)
                        .oscReceiver(oscName));
    vector<string> pointTexNames;
    for( tNamedTexture namedTex : mPointTextures )
    {
        pointTexNames.push_back(namedTex.first);
    }
    interface->addEnum(CreateEnumParam( "meshes/point_tex", (int*)(&mCurPointTexture) )
                       .maxValue(pointTexNames.size())
                       .oscReceiver(oscName)
                       .isVertical(), pointTexNames);
    
    
    interface->addParam(CreateBoolParam("meshes/wireframe", &mWireframe)
                        .oscReceiver(oscName));
    
    vector<string> meshTypeNames;
    for( tNamedMesh namedMesh : mMeshes )
    {
        meshTypeNames.push_back(namedMesh.first);
    }
    interface->addEnum(CreateEnumParam( "meshes/mesh", (int*)(&mMeshType) )
                       .maxValue(mMeshes.size())
                       .oscReceiver(oscName)
                       .isVertical(), meshTypeNames);
}

const ci::gl::VboMesh& MeshInstanceRenderer::getMesh()
{
    assert(mMeshType < mMeshes.size());
    
    return mMeshes[mMeshType].second;
}

void MeshInstanceRenderer::draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler )
{
    // pre-render - set state
    gl::pushMatrices();
    gl::setMatrices( cam );
    
    preRender();
    
    // bind textures
    particlesFbo.bindTexture(0);//pos
    particlesFbo.bindTexture(1);//vel
    particlesFbo.bindTexture(2);//info
    
//    mPointTextures[mCurPointTexture].second.bind(3);
//    
//    if (audioInputHandler.hasTexture())
//    {
//        audioInputHandler.getFbo().bindTexture(4);
//    }
    
    // bind shader
    mShader.bind();
    mShader.uniform("texSize", particlesFbo.getSize().x);
    mShader.uniform("posMap", 0 );
    //mShader.uniform("velocityMap", 1);
    //mShader.uniform("pointSpriteTex", 3);
    //mShader.uniform("intensityMap", 4);
    //mShader.uniform("screenWidth", (float)screenSize.x*2.0f);
    //mShader.uniform("MV", cam.getModelViewMatrix());
    //mShader.uniform("P", cam.getProjectionMatrix());
    //mShader.uniform("eyePoint", cam.getEyePoint());
    //mShader.uniform("colorScale", mColor);
    //mShader.uniform("audioReactive", mAudioReactive);
    //mShader.uniform("gain", audioInputHandler.getGain());
    //mShader.uniform( "lightingEnabled",	mLightEnabled );
    //mShader.uniform( "textureEnabled",	mTextureEnabled );
    
    // do magic
    size_t instanceCount = particlesFbo.getSize().x * particlesFbo.getSize().y;
    drawInstanced( getMesh(), instanceCount );
    
    // cleanup
    mShader.unbind();
    particlesFbo.unbindTexture();
    
    mPointTextures[mCurPointTexture].second.unbind();
    if (audioInputHandler.hasTexture())
    {
        audioInputHandler.getFbo().unbindTexture();
    }
    
    // post-render - restore state
    if ( mWireframe )
    {
		gl::disableWireframe();
        glLineWidth( 1.0f );
	}
    
    glPopAttrib();
    gl::popMatrices();
}

// draw VBO instanced
void MeshInstanceRenderer::drawInstanced( const gl::VboMesh &vbo, size_t count )
{
	vbo.enableClientStates();
	vbo.bindAllData();
	glDrawElementsInstancedARB( vbo.getPrimitiveType(), vbo.getNumIndices(), GL_UNSIGNED_INT, (GLvoid*)( sizeof(uint32_t) * 0 ), count );
	//glDrawElementsInstancedEXT( vbo.getPrimitiveType(), vbo.getNumIndices(), GL_UNSIGNED_INT, (GLvoid*)( sizeof(uint32_t) * 0 ), count ); // Try this if ARB doesn't work
	gl::VboMesh::unbindBuffers();
	vbo.disableClientStates();
}

void MeshInstanceRenderer::preRender()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    gl::enable(GL_TEXTURE_2D);
    
    glShadeModel( GL_FLAT );
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	
    // blend mode
	if(mAdditiveBlending)
    {
        gl::enableAdditiveBlending();
	}
    else
    {
		gl::enableAlphaBlending();
	}
    
    if ( mWireframe )
    {
        //glLineWidth( (GLfloat)mLineWidth );
		gl::enableWireframe();
	}
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
}

void MeshInstanceRenderer::postRender()
{
    
}
