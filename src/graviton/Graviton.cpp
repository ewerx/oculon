/*
 *  Graviton.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Graviton.h"
#include "Resources.h"
#include "AudioInput.h" // compile errors if this is not before App.h
#include "MindWave.h"
#include "OculonApp.h"//TODO: fix this dependency
#include "cinder/Rand.h"
#include "cinder/Easing.h"
#include "Utils.h"
#include "Interface.h"

using namespace ci;
using namespace ci::app;
using namespace std;


Graviton::Graviton()
: Scene("graviton")
{
}

Graviton::~Graviton()
{
}

void Graviton::setup()
{
    Scene::setup();
    
    mTimeStep = 0.00075f;
    mGravity = 50.0f;
    
    mUseInvSquareCalc = true;
    mFlags = PARTICLE_FLAGS_NONE;
    mColorMode = COLOR_MASS;
    mConstrainParticles = false;
    
    mInitialFormation = FORMATION_SPHERE;
    mFormationRadius = 50.0f;
    
    mAdditiveBlending = true;
    mEnableLineSmoothing = false;
    mEnablePointSmoothing = false;
    mUseImageForPoints = true;
    mPointSize = 1.0f;
    //mParticleAlpha = 0.5f;
    mUseMotionBlur = false;
    mColorScale = ColorAf( 0.1f, 0.2f, 0.3f, 0.5f );
    
    mDamping = 1.0f;
    mGravity = 100.0f;
    mEps = 0.001f;//mFormationRadius * 0.5f;
    
    mNumNodes = 0;
    mGravityNodeFormation = NODE_FORMATION_NONE;
    
    // camera
    mCamRadius = mFormationRadius * 3.0f;
    mCamAngle = 0.0f;
    mCamMaxDistance = mFormationRadius * 3.0f;
    mCamLateralPosition = -mCamMaxDistance;
    mCamTarget = Vec3f::zero();
    mCamTurnRate = 0.25f;
    mCamTranslateRate = 1.0f;
    mCamType = CAM_SPLINE;
    
    // neural
    mResetCameraByBlink = false;
    mColorByMindWave = false;
    
    // shaders
    mParticlesShader = loadFragShader("graviton_particle_frag.glsl" );
    mDisplacementShader = loadVertAndFragShaders("graviton_displacement_vert.glsl",  "graviton_displacement_frag.glsl");
    
    setupPingPongFbo();
    // THE VBO HAS TO BE DRAWN AFTER FBO!
    setupVBO();
    
    // textures
    mParticleTexture1 = gl::Texture( loadImage( app::loadResource( "particle_white.png" ) ) );
    mParticleTexture2 = gl::Texture( loadImage( app::loadResource( "glitter.png" ) ) );
    mParticleTexture1.setWrap( GL_REPEAT, GL_REPEAT );
    mParticleTexture2.setWrap( GL_REPEAT, GL_REPEAT );
    
    // renderers
    mMotionBlurRenderer.setup( mApp->getViewportSize(), boost::bind( &Graviton::drawParticles, this ) );
    
    reset();
}

void Graviton::setupPingPongFbo()
{
    //
    // FROM gpuPS
    //
    
    float scale = 80.0f;
    // TODO: Test with more than 2 texture attachments
	std::vector<Surface32f> surfaces;
    // Position 2D texture array
    surfaces.push_back( Surface32f( kStep, kStep, true) );
    Surface32f::Iter pixelIter = surfaces[0].getIter();
    while( pixelIter.line() ) {
        while( pixelIter.pixel() ) {
            /* Initial particle positions are passed in as R,G,B
             float values. Alpha is used as particle invMass. */
            surfaces[0].setPixel(pixelIter.getPos(),
                                 ColorAf(scale*(Rand::randFloat()-0.5f),
                                         scale*(Rand::randFloat()-0.5f),
                                         scale*(Rand::randFloat()-0.5f),
                                         Rand::randFloat(0.2f, 1.0f) ) );
        }
    }
    
    //Velocity 2D texture array
    surfaces.push_back( Surface32f( kStep, kStep, true) );
    pixelIter = surfaces[1].getIter();
    while( pixelIter.line() ) {
        while( pixelIter.pixel() ) {
            /* Initial particle velocities are
             passed in as R,G,B float values. */
            surfaces[1].setPixel( pixelIter.getPos(), ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );
        }
    }
    mParticlesFbo = PingPongFbo( surfaces );
}

void Graviton::setupVBO()
{
    // A dummy VboMesh the same size as the texture to keep the vertices on the GPU
    int totalVertices = kStep * kStep;
    vector<Vec2f> texCoords;
    vector<uint32_t> indices;
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    layout.setStaticNormals();
    //layout.setDynamicColorsRGBA();
    glPointSize(1.0f);
    mVboMesh = gl::VboMesh( totalVertices, totalVertices, layout, GL_POINTS);
    for( int x = 0; x < kStep; ++x ) {
        for( int y = 0; y < kStep; ++y ) {
            indices.push_back( x * kStep + y );
            texCoords.push_back( Vec2f( x/(float)kStep, y/(float)kStep ) );
        }
    }
    mVboMesh.bufferIndices( indices );
    mVboMesh.bufferTexCoords2d( 0, texCoords );
}

void Graviton::computeAttractorPosition()
{
    // The attractor is positioned at the intersection of a ray
    // from the mouse to a plane perpendicular to the camera.
    float t = 0;
    Vec3f right, up;
    const Camera& cam = getCamera();
    cam.getBillboardVectors(&right, &up);
    float u = mMousePos.x / (float) mApp->getViewportWidth();
    float v = mMousePos.y / (float) mApp->getViewportHeight();
    Ray ray = cam.generateRay(u , 1.0f - v, cam.getAspectRatio() );
    if (ray.calcPlaneIntersection(Vec3f(0.0f,0.0f,0.0f), right.cross(up), &t)) {
        mAttractor.set(ray.calcPosition(t));
    }
}

void Graviton::setupDebugInterface()
{
    Scene::setupDebugInterface(); // add all interface params
    
//    mDebugParams.setOptions("Time Step", "step=0.0001 min=-1.0 max=1.0" );
//    mDebugParams.setOptions("Gravity", "step=0.1");
//    mDebugParams.setOptions("Alpha", "min=0.0 max=1.0 step=0.001");
//    
//    mDebugParams.setOptions("Particle Formation", "min=0 max=3 enum='0 {Sphere}, 1 {Shell}, 2 {Disc}, 3 {Galaxy}'" );
//    mDebugParams.setOptions("Node Formation", "min=0 max=2");
//    mDebugParams.setOptions("Damping", "step=0.0001");
//    mDebugParams.setOptions("EPS", "min=0.0 step=0.001");
//    
//    mDebugParams.setOptions("Color Mode", "min=0 max=2");
//    
//    mDebugParams.setOptions("Cam Type", "min=0 max=3");
//    mDebugParams.setOptions("Cam Radius", "min=1.0");
//    mDebugParams.setOptions("Cam Distance", "min=0.0");
//    mDebugParams.setOptions("Cam Turn Rate", "step=0.01");
//    mDebugParams.setOptions("Cam Slide Rate", "step=0.1");
//    
//    mDebugParams.addSeparator();
//    mDebugParams.addParam("Inv Square", &mUseInvSquareCalc );
//    mDebugParams.addParam("Motion Blur", &mUseMotionBlur);
//    mDebugParams.addParam("Point Smoothing", &mEnablePointSmoothing, "");
//    mDebugParams.addParam("Point Sprites", &mUseImageForPoints, "");
//    mDebugParams.addParam("Additive Blending", &mAdditiveBlending, "");
    
}

void Graviton::setupInterface()
{
    mInterface->addParam(CreateFloatParam( "Time Step", &mTimeStep )
                         .minValue(0.0f)
                         .maxValue(0.001f)
                         .oscReceiver(getName(), "timestep"));
    mInterface->addParam(CreateFloatParam( "Damping", &mDamping )
                         .oscReceiver(getName(), "damping"));
    mInterface->addParam(CreateFloatParam( "EPS", &mEps )
                         .minValue(0.0001)
                         .maxValue(1500)
                         .oscReceiver(getName(), "eps"));
    mInterface->addParam(CreateFloatParam( "Gravity", &mGravity )
                         .minValue(0.0f)
                         .maxValue(100.0f)
                         .oscReceiver(getName(), "gravity"));
    
    mInterface->gui()->addSeparator();
    
    vector<string> formationNames;
#define GRAVITON_FORMATION_ENTRY( nam, enm ) \
formationNames.push_back(nam);
    GRAVITON_FORMATION_TUPLE
#undef  GRAVITON_FORMATION_ENTRY
    mInterface->addEnum(CreateEnumParam( "Particle Formation", (int*)(&mInitialFormation) )
                         .maxValue(FORMATION_COUNT)
                         .oscReceiver(getName(), "pformation")
                         .isVertical(), formationNames);
    
    vector<string> nodeFormationNames;
#define GRAVITON_NODE_FORMATION_ENTRY( nam, enm ) \
nodeFormationNames.push_back(nam);
    GRAVITON_NODE_FORMATION_TUPLE
#undef  GRAVITON_NODE_FORMATION_ENTRY
    mInterface->addEnum(CreateEnumParam( "Node Formation", (int*)(&mGravityNodeFormation) )
                         .maxValue(NODE_FORMATION_COUNT)
                         .oscReceiver(getName(), "nformation")
                         .isVertical(), nodeFormationNames);
    mInterface->addParam(CreateFloatParam( "Formation Radius", &mFormationRadius )
                         .minValue(10.0f)
                         .maxValue(1000.0f)
                         .oscReceiver(getName(), "formradius"));
    
    mInterface->gui()->addColumn();
    mInterface->addEnum(CreateEnumParam( "Color Mode", (int*)(&mColorMode) )
                        .maxValue(COLOR_COUNT)
                        .oscReceiver(getName(), "colormode")
                        .isVertical());
    //mInterface->addParam(CreateFloatParam( "Alpha", &mParticleAlpha )
    //                     .oscReceiver(getName(), "alpha"));
    mInterface->addParam(CreateColorParam("Color Scale", &mColorScale, kMinColor, kMaxColor)
                         .oscReceiver(mName,"colorscale"));
    mInterface->addParam(CreateFloatParam( "Point Size", &mPointSize )
                         .minValue(0.01f)
                         .maxValue(2.0f)
                         .oscReceiver(getName(), "psize"));
    
    mInterface->gui()->addColumn();
    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
                         .maxValue(CAM_COUNT)
                         .oscReceiver(getName(), "camtype")
                         .isVertical());
    mInterface->addParam(CreateFloatParam( "Cam Radius", &mCamRadius )
                         .minValue(1.0f)
                         .maxValue(500.f)
                         .oscReceiver(getName(), "camradius"));
    mInterface->addParam(CreateFloatParam( "Cam Distance", &mCamMaxDistance )
                         .minValue(0.0f)
                         .maxValue(500.f));
    mInterface->addParam(CreateFloatParam( "Cam Turn Rate", &mCamTurnRate )
                         .minValue(0.0f)
                         .maxValue(5.0f)
                         .oscReceiver(getName(), "camturn"));
    mInterface->addParam(CreateFloatParam( "Cam Slide Rate", &mCamTranslateRate )
                         .minValue(0.0f)
                         .maxValue(5.0f)
                         .oscReceiver(getName(), "camspeed"));
    mInterface->addButton(CreateTriggerParam("Reset Spline", NULL)
                          .oscReceiver(mName,"resetspline"))->registerCallback( this, &Graviton::setupCameraSpline );
    
    // mindwave
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateBoolParam( "Reset Camera by Blink", &mResetCameraByBlink )
                         .oscReceiver(getName(), "blinkcamera"));
    mInterface->addParam(CreateBoolParam( "Color by Mindwave", &mColorByMindWave )
                         .oscReceiver(getName(), "mindwavecolor"));
    
    mInterface->addParam(CreateBoolParam( "ImagePoints", &mUseImageForPoints ));
}

void Graviton::initParticles()
{
//    mNumParticles = kNumParticles;
//    
//    const double r = mFormationRadius;
//    
//    for( size_t i = 0; i < kNumParticles; ++i )
//    {
//        if( (NODE_FORMATION_NONE != mGravityNodeFormation) && i < mNumNodes)
//        {
//            mPosAndMass[i].x = mGravityNodes[i].mPos.x;
//            mPosAndMass[i].y = mGravityNodes[i].mPos.y;
//            mPosAndMass[i].z = mGravityNodes[i].mPos.z;
//            mPosAndMass[i].w = 1.0f;
//            
//            mVel[i].x = mGravityNodes[i].mVel.x;
//            mVel[i].x = mGravityNodes[i].mVel.y;
//            mVel[i].x = mGravityNodes[i].mVel.z;
//            mVel[i].w = mGravityNodes[i].mMass;
//            
//            mColor[i].x = 1.0f;
//            mColor[i].y = 1.0f;
//            mColor[i].z = 1.0f;
//            mColor[i].w = 1.0f;
//        }
//        else
//        {
//            double x = 0.0f;
//            double y = 0.0f;
//            double z = 0.0f;
//            
//            double vx = 0.0f;
//            double vy = 0.0f;
//            double vz = 0.0f;
//            
//            double rho = 0.0f;
//            double theta = 0.0f;
//            
//            const float maxMass = 50.0f;
//            float mass = Rand::randFloat(1.0f,maxMass);
//            
//            switch( mInitialFormation )
//            {
//                case FORMATION_SPHERE:
//                {
//                    rho = Utils::randDouble() * (M_PI * 2.0);
//                    theta = Utils::randDouble() * (M_PI * 2.0);
//                    
//                    const float d = Rand::randFloat(10.0f, r);
//                    x = d * cos(rho) * sin(theta);
//                    y = d * sin(rho) * sin(theta);
//                    z = d * cos(theta);
//                }
//                    break;
//                    
//                case FORMATION_SPHERE_SHELL:
//                {
//                    rho = Utils::randDouble() * (M_PI * 2.0);
//                    theta = Utils::randDouble() * (M_PI * 2.0);
//                    
//                    x = r * cos(rho) * sin(theta);
//                    y = r * sin(rho) * sin(theta);
//                    z = r * cos(theta);
//                }
//                    break;
//                    
//                case FORMATION_DISC:
//                {
//                    rho = r * Utils::randDouble();//pow(Utils::randDouble(), 0.75);
//                    theta = Utils::randDouble() * (M_PI * 2.0);
//                    theta = (0.5 * cos(2.0 * theta) + theta - 1e-2 * rho);
//                    
//                    const float thickness = 1.0f;
//                    
//                    x = rho * cos(theta);
//                    y = rho * sin(theta);
//                    z = thickness * 2.0 * Utils::randDouble() - 1.0;
//                    
//                    const double a = 1.0e0 * (rho <= 1e-1 ? 0.0 : rho);
//                    vx = -a * sin(theta);
//                    vy = a * cos(theta);
//                    vz = 0.0f;
//                }
//                    break;
//                    
//                case FORMATION_GALAXY:
//                {
//                    rho = r * pow(Utils::randDouble(), 0.75);
//                    theta = Utils::randDouble() * (M_PI * 2.0);
//                    theta = (0.5 * cos(2.0 * theta) + theta - 1e-2 * rho);
//                    
//                    x = rho * cos(theta);
//                    y = rho * sin(theta);
//                    
//                    const float dist = sqrt(x*x + y*y);
//                    const float maxThickness = r / 8.0f;
//                    const float coreDistanceRatio = EaseInOutQuad()(1.0f - dist / r);
//                    const float thickness =  maxThickness * coreDistanceRatio;
//                    
//                    z = thickness * (2.0 * Utils::randDouble() - 1.0);
//                    
//                    const double a = 1.0e0 * (rho <= 1e-1 ? 0.0 : rho);
//                    vx = -a * sin(theta);
//                    vy = a * cos(theta);
//                    vz = 0.0f;
//                    
//                    mass = maxMass * coreDistanceRatio;
//                }
//                    break;
//                    
//                default:
//                    break;
//            }
//            
//            // pos
//            mPosAndMass[i].x = x;
//            mPosAndMass[i].y = y;
//            mPosAndMass[i].z = z;
//            mPosAndMass[i].w = 1.0f; //scale??
//            
//            // vel
//            
//            mVel[i].x = vx;
//            mVel[i].y = vy;
//            mVel[i].z = vz;
//            mVel[i].w = mass;
//            
//            // color
//            mColor[i].x = 1.0f;
//            mColor[i].y = 1.0f;
//            mColor[i].z = 1.0f;
//            mColor[i].w = 1.0f;
//        }
//    }
}
                              
void Graviton::resetGravityNodes(const eNodeFormation formation)
{
    mGravityNodes.clear();
    
    switch( formation )
    {
        case NODE_FORMATION_SYMMETRY:
        {
            mNumNodes = 3;
            const float r = mFormationRadius;
            
            for(int i = 0; i < mNumNodes; ++i )
            {
                Vec3f pos;
                
                switch(i)
                {
                    case 0:
                        pos.x = 0.0f;
                        pos.y = 0.0f;
                        pos.z = 0.0f;
                        break;
                        
                    case 1:
                        
                        pos.x = r*0.5f;
                        pos.y = -r*0.5f;
                        pos.z = r / 4.0f;
                        break;
                    case 2:
                        pos.x = -r*0.5f;
                        pos.y = r*0.5f;
                        pos.z = -r / 4.0f;
                        break;
                }
                
                const float mass = 10000.f;
                mGravityNodes.push_back( tGravityNode( pos, Vec3f::zero(), mass ) );
            }
        } break;
            
            
        case NODE_FORMATION_BLACKHOLE_STAR:
        {
            mNumNodes = 1;
            
            const float mass = 10000.f;
            const float d = mFormationRadius * 3.0f;
            
            //Vec3f pos( d, d, d );
            Vec3f pos(0,0,0);
            mGravityNodes.push_back( tGravityNode( pos, Vec3f::zero(), mass ) );
        } break;
            
        default:
            break;
    }
    
    // random spline
    setupCameraSpline();
}

void Graviton::reset()
{
    resetGravityNodes(mGravityNodeFormation);
    initParticles();
    
    // camera
    mCamAngle = 0.0f;
    mCamLateralPosition = -mCamMaxDistance;
    mCamTarget = Vec3f::zero();
    mCam.setFov(60.0f);
    mCam.setAspectRatio(mApp->getViewportAspectRatio());
}

void Graviton::resize()
{
    Scene::resize();
    mMotionBlurRenderer.resize(mApp->getViewportSize());
}

void Graviton::update(double dt)
{
    updateAudioResponse();
    updateNeuralResponse();

    // update particle system
    computeAttractorPosition();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mParticlesFbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( mParticlesFbo.getBounds() );
    
    mParticlesFbo.bindUpdate();
    
    mParticlesShader.bind();
    mParticlesShader.uniform( "positions", 0 );
    mParticlesShader.uniform( "velocities", 1 );
    mParticlesShader.uniform( "attractorPos", mAttractor);
    gl::drawSolidRect(mParticlesFbo.getBounds());
    mParticlesShader.unbind();
    
    mParticlesFbo.unbindUpdate();
    gl::popMatrices();
    //
    
    //mSwap = !mSwap;
    
    updateCamera(dt);
    
    //updateHud();
    
    Scene::update(dt);
}

//
// handleKeyDown
//
bool Graviton::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {
        case ' ':
            reset();
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;
}

void Graviton::handleMouseDown( const MouseEvent& event )
{
	mIsMousePressed = true;
	mMousePos.x = event.getPos().x;
    mMousePos.y = event.getPos().y;
}

void Graviton::handleMouseUp( const MouseEvent& event )
{
	mIsMousePressed = false;
}

void Graviton::handleMouseDrag( const MouseEvent& event )
{
	mMousePos.x = event.getPos().x;
    mMousePos.y = event.getPos().y;
}

//
//
//
void Graviton::updateAudioResponse()
{
}

void Graviton::updateNeuralResponse()
{
    MindWave& mindWave = mApp->getMindWave();
    if( mindWave.hasData() )
    {
        if( mindWave.getBlink() > 0.0f )
        {
            if( mResetCameraByBlink )
            {
                setupCameraSpline();
            }
        }
        
        if( mColorByMindWave )
        {
            const float attention = mindWave.getAttention();
            const float meditation = mindWave.getMeditation();
            
            
        }
    }
}

void Graviton::updateCamera(const double dt)
{
    switch( mCamType )
    {
        case CAM_SPLINE:
        {
            mCamSplineValue += dt * (mCamTranslateRate*0.01f);
            Vec3f pos = mCamSpline.getPosition( mCamSplineValue );
            Vec3f delta = pos - mCamLastPos;
            Vec3f up = delta.cross(pos);
            //up.normalize();
            mCam.lookAt( pos, mCamTarget, up );
            mCamLastPos = pos;
        }
            break;
            
        case CAM_SPIRAL:
        {
            //TODO: use quaternion
            //Quatf incQuat( normal, rotation );
            //mQuat *= incQuat;
            //mQuat.normalize();
            mCamAngle += dt * mCamTurnRate;
            mCamLateralPosition += dt * mCamTranslateRate;
            if( ( (mCamLateralPosition > mCamMaxDistance) && mCamTranslateRate > 0.0f ) ||
               ( (mCamLateralPosition < -mCamMaxDistance) && mCamTranslateRate < 0.0f ) )
            {
                mCamTranslateRate = -mCamTranslateRate;
            }
            
            Vec3f pos(mCamRadius * cos(mCamAngle),
                      mCamRadius * sin(mCamAngle),
                      mCamLateralPosition );
            
            Vec3f up( pos.x, pos.y, 0.0f );
            up.normalize();
            mCam.lookAt( pos, mCamTarget, up );
        }
            break;
            
        default:
            break;
    }
    
}

bool Graviton::setupCameraSpline()
{
    vector<Vec3f> points;
	int numPoints = 4 + ( Rand::randInt(4) );
	for( int p = 0; p < numPoints; ++p )
    {
		points.push_back( Vec3f( Rand::randFloat(-mCamRadius/2.0f, mCamRadius/2.0f), Rand::randFloat(-mCamRadius/2.0f, mCamRadius/2.0f), Rand::randFloat(-mCamRadius/2.0f, mCamRadius/2.0f) ) );
    }
	mCamSpline = BSpline3f( points, 3, true, false );
    
	mCamSplineValue = 0.0f;
	mCamRotation = Quatf::identity();
	mCamLastPos = mCamSpline.getPosition( 0 );
    
    return false;
}

void Graviton::drawCamSpline()
{
    gl::pushMatrices();
    gl::setMatrices( ( mCamType == CAM_MANUAL ) ? mApp->getMayaCam() : mCam );
	const int numSegments = 100;
	gl::color( ColorA( 0.2f, 0.85f, 0.8f, 0.85f ) );
	glLineWidth( 2.0f );
	gl::begin( GL_LINE_STRIP );
	for( int s = 0; s <= numSegments; ++s ) 
    {
		float t = s / (float)numSegments;
		gl::vertex( mCamSpline.getPosition( t ) );
	}
	gl::end();
    gl::popMatrices();
}

//
// MARK: Render
//
void Graviton::draw()
{
    gl::pushMatrices();
    
    if( mUseMotionBlur )
    {
        mMotionBlurRenderer.draw();
    }
    else
    {
        drawParticles();
    }
    
    gl::popMatrices();
}

const Camera& Graviton::getCamera()
{
    switch( mCamType )
    {
        case CAM_SPLINE:
        case CAM_SPIRAL:
            return( mCam );
            
        case CAM_ORBITER:
        {
            Scene* orbiterScene = mApp->getScene("orbiter");
            
            if( orbiterScene && orbiterScene->isRunning() )
            {
                return( orbiterScene->getCamera() );
            }
            else
            {
                return( mApp->getMayaCam() );
            }
        }
            break;
            
        default:
            return mApp->getMayaCam();
    }
}

void Graviton::preRender() 
{
    gl::setMatrices( getCamera() );
    gl::setViewport( mApp->getViewportBounds() );

    gl::enable(GL_POINT_SPRITE);
    gl::enable(GL_PROGRAM_POINT_SIZE_EXT);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	
	if(mAdditiveBlending)
    {
        gl::enableAdditiveBlending();
	}
    else 
    {
		gl::enableAlphaBlending();
	}
//
//	if(mEnableLineSmoothing) 
//    {
//		glEnable(GL_LINE_SMOOTH);
//	}
//    else 
//    {
//		glDisable(GL_LINE_SMOOTH);
//	}
//	
//	if(mEnablePointSmoothing) 
//    {
//		glEnable(GL_POINT_SMOOTH);
//	} 
//    else 
//    {
//		glDisable(GL_POINT_SMOOTH);
//	}
//
//	glLineWidth(mLineWidth);
    
    gl::disableDepthWrite();
    
}


void Graviton::drawParticles() 
{
    preRender();
    glEnable(GL_TEXTURE_2D);
    if(mUseImageForPoints)
    {
        mParticleTexture1.bind(2);
    }
    else
    {
        mParticleTexture2.bind(2);
    }
    
//    glDrawArrays(GL_POINTS, 0, mNumParticles);
    mParticlesFbo.bindTexture(0);
    mParticlesFbo.bindTexture(1);
    mDisplacementShader.bind();
    mDisplacementShader.uniform("displacementMap", 0 );
    mDisplacementShader.uniform("velocityMap", 1);
    mDisplacementShader.uniform("pointSpriteTex", 2);
    mDisplacementShader.uniform("screenWidth", (float)getWindowWidth());
    mDisplacementShader.uniform("spriteWidth", mPointSize);
    mDisplacementShader.uniform("MV", getCamera().getModelViewMatrix());
    mDisplacementShader.uniform("P", getCamera().getProjectionMatrix());
    
    gl::draw( mVboMesh );
    mDisplacementShader.unbind();
    mParticlesFbo.unbindTexture();
    
    if(mUseImageForPoints)
    {
        mParticleTexture1.unbind();
    }
    else
    {
        mParticleTexture2.unbind();
    }
}

void Graviton::drawDebug()
{
    Scene::drawDebug();
    
    if( NODE_FORMATION_NONE != mGravityNodeFormation )
    {
        gl::pushMatrices();
    
        CameraOrtho textCam(0.0f, mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f, 0.0f, 10.f);
        gl::setMatrices(textCam);
//    
//    
//        for( int i = 0; i < mNumNodes; ++i )
//        {
//            Vec3f worldCoords( mPosAndMass[i].x, mPosAndMass[i].y, mPosAndMass[i].z );
//            Vec2f textCoords = getCamera().worldToScreen(worldCoords, mApp->getViewportWidth(), mApp->getViewportHeight());
//            
//            gl::drawString(toString(mGravityNodes[i].mMass),textCoords,ColorAf(1.0,1.0,1.0,0.4));
//        }
//        
        gl::popMatrices();
    }
    
    drawCamSpline();
}
