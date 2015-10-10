//
//  Lines.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 11/21/2013.
//
//


#include "cinder/Perlin.h"
#include "cinder/Rand.h"

#include "Lines.h"
#include "OculonApp.h"
#include "Interface.h"
#include "SpinCam.h"
#include "SplineCam.h"
#include "OtherSceneCam.h"

#include "Flock.h"

using namespace ci;
using namespace ci::app;
using namespace std;


#pragma mark - Construction

Lines::Lines()
: Scene("lines")
{
}

Lines::~Lines()
{
}

#pragma mark - Setup

void Lines::setup()
{
    Scene::setup();
    
    mReset = false;
    
    // params
    mMotion = MOTION_NOISE;
    
    mContainmentRadius = 0.0f;
    
    mAudioTime = false;
    
    // nodes
    mNodeSimShader = loadVertAndFragShaders("lines_simulation_vert.glsl", "lines_nodesim_frag.glsl");
    mNodeBufSize = 48;
    setupNodes(mNodeBufSize); // 256 nodes
    mNodeMotion = NODE_MOTION_STATIC;
    
    // simulation
    mSimulationShader = loadVertAndFragShaders("lines_simulation_vert.glsl", "lines_simulation_frag.glsl");
    
    const int bufSize = 64;
    setupParticles(bufSize);
    
    // rendering
    mParticleController.addRenderer( new LinesRenderer() );
//    mParticleController.addRenderer( new GravitonRenderer() );
//    mParticleController.addRenderer( new DustRenderer() );
    
    mDynamicTexture.setup(bufSize, bufSize);
    
    mCameraController.setup(mApp);
    mCameraController.addCamera( new SpinCam(mApp->getViewportAspectRatio()) );
    mCameraController.addCamera( new SplineCam(mApp->getViewportAspectRatio()) );
    mCameraController.addCamera( new OtherSceneCam(mApp, "graviton") );
    mCameraController.addCamera( new OtherSceneCam(mApp, "parsec") );
    mCameraController.setCamIndex(1);
    
    
    mApp->setCamera(Vec3f(100.0f, 0.0f, 0.0f), Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f,1.0f,0.0f));
    
    // audio
    mAudioInputHandler.setup(true);
    
    mTimeController.setTimeScale(0.01f);
}

void Lines::setupParticles(const int bufSize)
{
    int numParticles = bufSize*bufSize;
    console() << "[lines] initializing " << (numParticles/2) << " lines..." << std::endl;
    
    mParticleController.setup(bufSize);
    
    vector<Vec4f> positions;
    vector<Vec4f> velocities;
    vector<Vec4f> data;
    
    const float r = 100.0f;
    
    // random
    {
        for (int i = 0; i < numParticles; ++i)
        {
            int nodeIndex = Rand::randInt(mNodePositions.size());//Rand::randInt(mNodeBufSize*mNodeBufSize);
            
            // position + mass
            float x = r * Rand::randFloat(-1.0f,1.0f);
            float y = r * Rand::randFloat(-1.0f,1.0f);
            float z = r * Rand::randFloat(-1.0f,1.0f);
            float mass = Rand::randFloat(0.01f,1.0f);
            positions.push_back(Vec4f(x,y,z,mass));
            
            // velocity + age
            float vx = Rand::randFloat(-.005f,.005f);
            float vy = Rand::randFloat(-.005f,.005f);
            float vz = Rand::randFloat(-.005f,.005f);
            float age = Rand::randFloat(.007f,0.9f);
            velocities.push_back(Vec4f(vx,vy,vz,age));
            
            // extra info
//            float decay = Rand::randFloat(.01f,10.00f);
            data.push_back(Vec4f(x,y,z,(float)nodeIndex));
        }
        mParticleController.addFormation(new ParticleFormation("random", bufSize, positions, velocities, data));
        positions.clear();
        data.clear();
    }
    
    // straight lines
    {
        bool pair = false;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        
        for (int i = 0; i < numParticles; ++i)
        {
            if (pair)
            {
                int axis = Rand::randInt(3); // x=0,y=1,z=2
                switch(axis)
                {
                    case 0:
                        x = -x;
                        break;
                    case 1:
                        y = -y;
                        break;
                    case 2:
                        z = -z;
                        break;
                    default:
                        break;
                }
            }
            else
            {
                x = r * Rand::randFloat(-1.0f,1.0f);
                y = r * Rand::randFloat(-1.0f,1.0f);
                z = r * Rand::randFloat(-1.0f,1.0f);
            }
            
            float mass = Rand::randFloat(0.01f,1.0f);
            
            // position + mass
            positions.push_back(Vec4f(x,y,z,mass));
            
            data.push_back(Vec4f(x,y,z,0.0f));
            
            pair = !pair;
        }
        
        mParticleController.addFormation(new ParticleFormation("straight", bufSize, positions, velocities, data));
        positions.clear();
        velocities.clear();
    }
    
    // inner and outer cirlces
    {
        bool pair = false;
        
        float ratio = 0.5f;
        
        float rho = Rand::randFloat() * (M_PI * 2.0);
        float theta = Rand::randFloat() * (M_PI * 2.0);
        
        for (int i = 0; i < numParticles; ++i)
        {
            float radius = r;
            
            if (pair)
            {
                radius *= ratio;
            }
            else
            {
                rho = Rand::randFloat() * (M_PI * 2.0);
                theta = Rand::randFloat() * (M_PI * 2.0);
            }
            
            float x = radius * cos(rho) * sin(theta);
            float y = radius * sin(rho) * sin(theta);
            float z = radius * cos(theta);
            
            float mass = Rand::randFloat(0.01f,1.0f);
            
            // position + mass
            positions.push_back(Vec4f(x,y,z,mass));
        
            velocities.push_back(Vec4f(Rand::randVec3f(),1.0f));
            
            pair = !pair;
        }
        
        mParticleController.addFormation(new ParticleFormation("shell", bufSize, positions, velocities, data));
        positions.clear();
    }
    
    // cone
//    {
//        bool pair = false;
//        
//        float rho = Rand::randFloat() * (M_PI * 2.0);
//        float theta = Rand::randFloat() * (M_PI * 2.0);
//        
//        for (int i = 0; i < numParticles; ++i)
//        {
//            float radius = 0.0f;
//            
//            if (pair)
//            {
//                radius = r;
//            }
//            else
//            {
//                rho = Rand::randFloat() * (M_PI * 0.75f);
//                theta = Rand::randFloat() * (M_PI * 0.75f);
//                radius = -r;
//            }
//            
//            float x = radius * cos(rho) * sin(theta);
//            float y = radius * sin(rho) * sin(theta);
//            float z = radius * cos(theta);
//            
//            float mass = Rand::randFloat(0.01f,1.0f);
//            
//            // position + mass
//            positions.push_back(Vec4f(x,y,z,mass));
//            
//            pair = !pair;
//        }
//        
//        mParticleController.addFormation(new ParticleFormation("cone", bufSize, positions, velocities, data));
//        positions.clear();
//    }
    
    // connected
    {
        positions.clear();
        data.clear();
        
        // connecting points on a sphere
        int numPoints = mNodeBufSize * mNodeBufSize;
        vector<Vec3f> connectors;
        
        float radius = r;
        
        for (int i = 0; i < numPoints; ++i)
        {
            float rho = Rand::randFloat() * (M_PI * 2.0);
            float theta = Rand::randFloat() * (M_PI * 2.0);
            
            float x = radius * cos(rho) * sin(theta);
            float y = radius * sin(rho) * sin(theta);
            float z = radius * cos(theta);
            
            connectors.push_back(Vec3f(x,y,z));
        }
        
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        
        // start
        positions.push_back(Vec4f(x,y,z,0.0f));
        
        for (int i = 0; i < numParticles; ++i)
        {
            int nodeIndex = Rand::randInt(mNodePositions.size());//Rand::randInt(connectors.size());
            Vec3f pos = mNodePositions[nodeIndex];//connectors[nodeIndex];
        
            float mass = (float)nodeIndex;//Rand::randFloat(0.01f,1.0f);
        
            // position + mass
            positions.push_back(Vec4f(pos.x,pos.y,pos.z,mass));
            
            // extra info
            data.push_back(Vec4f(x,y,z,float(nodeIndex)));
        }
        
        mParticleController.addFormation(new ParticleFormation("connected", bufSize, positions, velocities, data));
        positions.clear();
    }
    
//    {
//        // connected lines
//        bool pair = false;
//        float x = 0.0f;
//        float y = 0.0f;
//        float z = 0.0f;
//        
//        // start
//        positions.push_back(Vec4f(x,y,z,0.0f));
//        
//        for (int i = 0; i < numParticles; ++i)
//        {
//            if (!pair)
//            {
//                x = r * Rand::randFloat(-1.0f,1.0f);
//                y = r * Rand::randFloat(-1.0f,1.0f);
//                z = r * Rand::randFloat(-1.0f,1.0f);
//            }
//            
//            float mass = Rand::randFloat(0.01f,1.0f);
//            
//            // position + mass
//            positions.push_back(Vec4f(x,y,z,mass));
//            
//            pair = !pair;
//        }
//        
//        mParticleController.addFormation(new ParticleFormation("connected", bufSize, positions, velocities, data));
//        positions.clear();
//    }
//    
//    // square snake
//    {
//        bool pair = false;
//        float x = 0.0f;
//        float y = 0.0f;
//        float z = 0.0f;
//        
//        int axis = 0;
//        
//        // start
//        positions.push_back(Vec4f(x,y,z,0.0f));
//        
//        for (int i = 0; i < numParticles; ++i)
//        {
//            if (pair)
//            {
//                axis = (axis + 1) % 3;
//            }
//            else
//            {
//                axis = Rand::randInt(3); // x=0,y=1,z=2
//            }
//            
//            switch(axis)
//            {
//                case 0:
//                    x = r * Rand::randFloat(-1.0f,1.0f);
//                    break;
//                case 1:
//                    y = r * Rand::randFloat(-1.0f,1.0f);
//                    break;
//                case 2:
//                    z = r * Rand::randFloat(-1.0f,1.0f);
//                    break;
//                default:
//                    break;
//            }
//            
//            float mass = Rand::randFloat(0.01f,1.0f);
//            
//            // position + mass
//            positions.push_back(Vec4f(x,y,z,mass));
//            
//            pair = !pair;
//        }
//        
//        mParticleController.addFormation(new ParticleFormation("snake", bufSize, positions, velocities, data));
//        positions.clear();
//    }
    
    // cube
    {
        int count = 0;
        const int d = 24;
        positions.clear();
        //velocities.clear();
        data.clear();
        float distPer = r / (d*0.5);
        
        float x1 = -0.5 * d * distPer;
        float y1 = -0.5 * d * distPer;
        float z1 = -0.5 * d * distPer;
        
        float x2 = 0.5 * d * distPer;
        float y2 = 0.5 * d * distPer;
        float z2 = 0.5 * d * distPer;
        
        // lines
        for (int i = 0; i < d*0.5; ++i)
        {
            z1 = -0.5 * d * distPer;
            x2 = 0.5 * d * distPer;
            
            for (int j = 0; j < d*0.5; ++j)
            {
                x1 = -0.5 * d * distPer;
                z2 = 0.5 * d * distPer;
                
                for (int k = 0; k < d*0.5; ++k)
                {
                    float mass = Rand::randFloat(0.01f,1.0f);
                    positions.push_back(Vec4f(x1,y1,z1,mass));
                    
                    // velocity + age
                    //float age = Rand::randFloat(.000001f,0.00005f);
                    //velocities.push_back(Vec4f(0.0f, 0.0f, 0.0f, age));
                    
                    // extra info
                    float decay = Rand::randFloat(.01f,10.00f);
                    data.push_back(Vec4f(x1,y1,z1,decay));
                    
                    positions.push_back(Vec4f(x2,y2,z2,mass));
                    data.push_back(Vec4f(x2,y2,z2,decay));
                    
                    x1 += distPer;
                    z2 -= distPer;
                    
                    ++count;
                }
                
                z1 += distPer;
                x2 -= distPer;
            }
            
            y1 += distPer;
            y2 -= distPer;
        }
        
        for (int i = count; i < numParticles; ++i) {
            positions.push_back(Vec4f::zero());
            //velocities.push_back(Vec4f::zero());
            data.push_back(Vec4f::zero());
        }
        
        mParticleController.addFormation(new ParticleFormation("cube", bufSize, positions, velocities, data));
    }
    
    // cube
    {
        int count = 0;
        const int d = 24;
        positions.clear();
        //velocities.clear();
        data.clear();
        float distPer = r / (d*0.5);
        
        float x1 = -0.5 * d * distPer;
        float y1 = -0.5 * d * distPer;
        float z1 = -0.5 * d * distPer;
        
        float x2 = 0.5 * d * distPer;
        float y2 = 0.5 * d * distPer;
        float z2 = 0.5 * d * distPer;
        
        // lines
        for (int i = 0; i < d*0.5; ++i)
        {
            z1 = -0.5 * d * distPer;
            y2 = 0.5 * d * distPer;
            
            for (int j = 0; j < d*0.5; ++j)
            {
                x1 = -0.5 * d * distPer;
                z2 = 0.5 * d * distPer;
                
                for (int k = 0; k < d*0.5; ++k)
                {
                    float mass = Rand::randFloat(0.01f,1.0f);
                    positions.push_back(Vec4f(x1,y1,z1,mass));
                    
                    // velocity + age
                    //float age = Rand::randFloat(.000001f,0.00005f);
                    //velocities.push_back(Vec4f(0.0f, 0.0f, 0.0f, age));
                    
                    // extra info
                    float decay = Rand::randFloat(.01f,10.00f);
                    data.push_back(Vec4f(x1,y1,z1,decay));
                    
                    positions.push_back(Vec4f(x2,y2,z2,mass));
                    data.push_back(Vec4f(x2,y2,z2,decay));
                    
                    x1 += distPer;
                    z2 -= distPer;
                    
                    ++count;
                }
                
                z1 += distPer;
                
                y2 -= distPer;
            }
            
            y1 += distPer;
            x2 -= distPer;
        }
        
        for (int i = count; i < numParticles; ++i) {
            positions.push_back(Vec4f::zero());
            //velocities.push_back(Vec4f::zero());
            data.push_back(Vec4f::zero());
        }
        
        mParticleController.addFormation(new ParticleFormation("cube2", bufSize, positions, velocities, data));
    }

    // TODO: refactor into a ParticleController::completeSetup method... is there a better way? first update?
    mParticleController.resetToFormation(0);
}

//void Lines::addParsecFormation( )

void Lines::reset()
{
    mReset = true;
    
    mApp->setCamera(Vec3f(580.0f, 0.0f, 0.0f), Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f,1.0f,0.0f));
}

#pragma mark - Interface

void Lines::setupInterface()
{
    mTimeController.setupInterface(mInterface, getName(), 1, 18);
    
    mInterface->gui()->addColumn();
    vector<string> motionNames;
#define MOTION_ENTRY( nam, enm ) \
    motionNames.push_back(nam);
    MOTION_TUPLE
#undef  MOTION_ENTRY
    mInterface->addEnum(CreateEnumParam( "behavior", (int*)(&mMotion) )
                        .maxValue(MOTION_COUNT)
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), motionNames);
    
    mDynamicTexture.setupInterface(mInterface, getName());
    
    mInterface->addParam(CreateFloatParam( "contain_radius", &mContainmentRadius )
                         .maxValue(10000.0f)
                         .oscReceiver(getName())
                         .sendFeedback());
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("nodes");
    vector<string> nodeMotionNames;
#define NODE_MOTION_ENTRY( nam, enm ) \
nodeMotionNames.push_back(nam);
    NODE_MOTION_TUPLE
#undef  NODE_MOTION_ENTRY
    mInterface->addEnum(CreateEnumParam( "behavior", (int*)(&mNodeMotion) )
                        .maxValue(NODE_MOTION_COUNT)
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), nodeMotionNames);
    mNodeController.setupInterface(mInterface, getName());
    
    mInterface->gui()->addColumn();
    mParticleController.setupInterface(mInterface, getName());
    
    mCameraController.setupInterface(mInterface, getName());
    mAudioInputHandler.setupInterface(mInterface, getName(), 1, 19, 2, 19);
    
    // FIXME: MIDI HACK
    mowa::sgui::PanelControl* hiddenPanel = mInterface->gui()->addPanel();
    hiddenPanel->enabled = false;
    mInterface->addButton(CreateTriggerParam("formation0", NULL)
                          .midiInput(0, 2, 17))->registerCallback( boost::bind( &ParticleController::setFormation, &mParticleController, 0) );
    mInterface->addButton(CreateTriggerParam("formation1", NULL)
                          .midiInput(0, 2, 18))->registerCallback( boost::bind( &ParticleController::setFormation, &mParticleController, 1) );
    mInterface->addParam(CreateFloatParam("anim_time", mParticleController.getAnimTimePtr())
                                          .minValue(0.0f)
                                          .maxValue(120.0f)
                                          .midiInput(0, 1, 17));
}

#pragma mark - Update

void Lines::update(double dt)
{
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mCameraController.update(dt);
    
    gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
    
    mDynamicTexture.update(dt);
    
    updateNodes(dt);
    
    updateParticles(dt);
    
    mReset = false;
    
    Scene::update(dt);
}

void Lines::updateParticles(double dt)
{
    // TODO: refactor
    PingPongFbo& fbo = mParticleController.getParticleFbo();
    
    gl::pushMatrices();
    gl::setMatricesWindow( fbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( fbo.getBounds() );
    
    fbo.bindUpdate();
    
    mParticleController.getFormation().getPositionTex().bind(3);
    mParticleController.getFormation().getVelocityTex().bind(4);
    
    mDynamicTexture.bindTexture(5);
    
    if (mAudioInputHandler.hasTexture())
    {
        mAudioInputHandler.getFbo().bindTexture(7);
    }
    
    // bind node position tex
    bool flockPredatorNodes = true;
    
    Flock *flock = static_cast<Flock*>(mApp->getScene("flock"));
    if (flockPredatorNodes && flock)
    {
        flock->mPositionFbos[ flock->mThisFbo ].bindTexture(6, 0);
    }
    else
    {
        mNodeController.getParticleFbo().bindTexture(6, 0);
    }
    
    float simdt = mTimeController.getDelta();
    if (mAudioTime) simdt *= (1.0 - mAudioInputHandler.getAverageVolumeLowFreq());
    mSimulationShader.bind();
    mSimulationShader.uniform( "positions", 0 );
    mSimulationShader.uniform( "velocities", 1 );
    mSimulationShader.uniform( "information", 2);
    mSimulationShader.uniform( "oPositions", 3);
	mSimulationShader.uniform( "oVelocities", 4);
  	mSimulationShader.uniform( "noiseTex", 5);
    mSimulationShader.uniform( "audioData", 7);
  	mSimulationShader.uniform( "nodePosTex", 6);
    mSimulationShader.uniform( "nodeBufSize", (float)mNodeBufSize);
    mSimulationShader.uniform( "gain", mAudioInputHandler.getGain());
    mSimulationShader.uniform( "dt", (float)simdt );
    mSimulationShader.uniform( "reset", mReset );
    mSimulationShader.uniform( "startAnim", mParticleController.isStartingAnim() );
    mSimulationShader.uniform( "formationStep", mParticleController.getFormationStep() );
    mSimulationShader.uniform( "motion", mMotion );
    mSimulationShader.uniform( "containmentSize", mContainmentRadius );
    
    gl::drawSolidRect(fbo.getBounds());
    
    mSimulationShader.unbind();
    
    mNodeController.getFormation().getPositionTex().unbind();
    
    mDynamicTexture.unbindTexture();
    
    mParticleController.getFormation().getPositionTex().unbind();
    mParticleController.getFormation().getVelocityTex().unbind();
    
    fbo.unbindUpdate();
    gl::popMatrices();
}

#pragma mark - Draw

const Camera& Lines::getCamera()
{
    return mCameraController.getCamera();
}

void Lines::draw()
{
    gl::pushMatrices();
    if (mApp->outputToOculus())
    {
        // render left eye
        Area leftViewport = Area( Vec2f( 0.0f, 0.0f ), Vec2f( getFbo().getWidth() / 2.0f, getFbo().getHeight() ) );
        gl::setViewport(leftViewport);
        mApp->getOculusCam().enableStereoLeft();
        mParticleController.draw(leftViewport.getSize(), mApp->getOculusCam().getCamera(), mAudioInputHandler);
        
        Area rightViewport = Area( Area( Vec2f( getFbo().getWidth() / 2.0f, 0.0f ), Vec2f( getFbo().getWidth(), getFbo().getHeight() ) ) );
        gl::setViewport(rightViewport);
        mApp->getOculusCam().enableStereoRight();
        mParticleController.draw(rightViewport.getSize(), mApp->getOculusCam().getCamera(), mAudioInputHandler);
    }
    else
    {
        mParticleController.draw(mApp->getViewportSize(), getCamera(), mAudioInputHandler);
    }
    gl::popMatrices();
}

void Lines::drawDebug()
{
    Scene::drawDebug();
    
    gl::pushMatrices();
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
    gl::enable( GL_TEXTURE_2D );
    const Vec2f& windowSize( getWindowSize() );
    gl::setMatricesWindow( windowSize );
    
    const float size = 80.0f;
    const float paddingX = 20.0f;
    const float paddingY = 240.0f;
    
    //HACK
    PingPongFbo& fbo = mParticleController.getParticleFbo();
    
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    gl::draw( fbo.getTexture(0), preview );
    
    Rectf preview2 = preview - Vec2f(size+paddingX, 0.0f);
    gl::draw( fbo.getTexture(1), preview2 );
    
    Rectf preview3 = preview2 - Vec2f(size+paddingX, 0.0f);
    gl::draw(mDynamicTexture.getTexture(), preview3);
    
    glPopAttrib();
    gl::popMatrices();
}

#pragma mark - Nodes

void Lines::setupNodes(const int bufSize)
{
    int numNodes = bufSize*bufSize;
    console() << "[lines] initializing " << numNodes << " nodes..." << std::endl;
    
    mNodeController.setup(bufSize);
    
    vector<Vec4f> positions;
    vector<Vec4f> velocities;
    vector<Vec4f> data;
    
    const float r = 100.0f;
    
    // random
    {
        for (int i = 0; i < numNodes; ++i)
        {
            float rho = Rand::randFloat() * (M_PI * 2.0);
            float theta = Rand::randFloat() * (M_PI * 2.0);
            
            float x = r * cos(rho) * sin(theta);
            float y = r * sin(rho) * sin(theta);
            float z = r * cos(theta);
            
            float mass = Rand::randFloat(0.01f,1.0f);
            positions.push_back(Vec4f(x,y,z,mass));
            
            mNodePositions.push_back(Vec3f(x,y,z));
            
            // velocity + age
            float vx = Rand::randFloat(-.005f,.005f);
            float vy = Rand::randFloat(-.005f,.005f);
            float vz = Rand::randFloat(-.005f,.005f);
            float age = Rand::randFloat(.007f,0.9f);
            velocities.push_back(Vec4f(vx,vy,vz,age));
            
            // extra info
            float decay = Rand::randFloat(.01f,10.00f);
            data.push_back(Vec4f(x,y,z,decay));
        }
        mNodeController.addFormation(new ParticleFormation("random", bufSize, positions, velocities, data));
    }
    
    {
        positions.clear();
        
        for (int i = 0; i < numNodes; ++i)
        {
            float rho = Rand::randFloat() * (M_PI * 2.0);
            float theta = Rand::randFloat() * (M_PI * 2.0);
            
            float x = r * cos(rho) * sin(theta);
            float y = r * sin(rho) * sin(theta);
            float z = r * cos(theta);
            
            float mass = Rand::randFloat(0.01f,1.0f);
            positions.push_back(Vec4f(x,y,z,mass));
        }
        mNodeController.addFormation(new ParticleFormation("random2", bufSize, positions, velocities, data));
    }
    
//    // straight lines
//    {
//        positions.clear();
//        bool pair = false;
//        float x = 0.0f;
//        float y = 0.0f;
//        float z = 0.0f;
//        
//        for (int i = 0; i < numNodes; ++i)
//        {
//            if (pair)
//            {
//                int axis = Rand::randInt(3); // x=0,y=1,z=2
//                switch(axis)
//                {
//                    case 0:
//                        x = -x;
//                        break;
//                    case 1:
//                        y = -y;
//                        break;
//                    case 2:
//                        z = -z;
//                        break;
//                    default:
//                        break;
//                }
//            }
//            else
//            {
//                x = r * Rand::randFloat(-1.0f,1.0f);
//                y = r * Rand::randFloat(-1.0f,1.0f);
//                z = r * Rand::randFloat(-1.0f,1.0f);
//            }
//            
//            float mass = Rand::randFloat(0.01f,1.0f);
//            
//            // position + mass
//            positions.push_back(Vec4f(x,y,z,mass));
//            
//            pair = !pair;
//        }
//        
//        mNodeController.addFormation(new ParticleFormation("straight", bufSize, positions, velocities, data));
//        positions.clear();
//        velocities.clear();
//    }
//    
//    // inner and outer cirlces
//    {
//        bool pair = false;
//        
////        float ratio = 0.5f;
//        
//        float rho = Rand::randFloat() * (M_PI * 2.0);
//        float theta = Rand::randFloat() * (M_PI * 2.0);
//        
//        for (int i = 0; i < numNodes; ++i)
//        {
//            float radius = r;
//            
////            if (pair)
////            {
////                radius *= ratio;
////            }
////            else
//            {
//                rho = Rand::randFloat() * (M_PI * 2.0);
//                theta = Rand::randFloat() * (M_PI * 2.0);
//            }
//            
//            float x = radius * cos(rho) * sin(theta);
//            float y = radius * sin(rho) * sin(theta);
//            float z = radius * cos(theta);
//            
//            float mass = Rand::randFloat(0.01f,1.0f);
//            
//            // position + mass
//            positions.push_back(Vec4f(x,y,z,mass));
//            
//            velocities.push_back(Vec4f(Rand::randVec3f(),1.0f));
//            
//            pair = !pair;
//        }
//        
//        mNodeController.addFormation(new ParticleFormation("shell", bufSize, positions, velocities, data));
//    }
 
    // TODO: refactor into a ParticleController::completeSetup method... is there a better way? first update?
    mNodeController.resetToFormation(0);
}

void Lines::updateNodes(double dt)
{
    // TODO: refactor
    PingPongFbo& fbo = mNodeController.getParticleFbo();
    
    gl::pushMatrices();
    gl::setMatricesWindow( fbo.getSize(), false ); // false to prevent vertical flipping
    gl::setViewport( fbo.getBounds() );
    
    fbo.bindUpdate();
    
    mNodeController.getFormation().getPositionTex().bind(3);
    mNodeController.getFormation().getVelocityTex().bind(4);
    
    mDynamicTexture.bindTexture(5);
  
    if (mAudioInputHandler.hasTexture())
    {
        mAudioInputHandler.getFbo().bindTexture(6);
    }
    
    float simdt = mTimeController.getDelta();
    if (mAudioTime) simdt *= (1.0 - mAudioInputHandler.getAverageVolumeLowFreq());
    mNodeSimShader.bind();
    mNodeSimShader.uniform( "positions", 0 );
    mNodeSimShader.uniform( "velocities", 1 );
    mNodeSimShader.uniform( "information", 2);
    mNodeSimShader.uniform( "oPositions", 3);
	mNodeSimShader.uniform( "oVelocities", 4);
  	mNodeSimShader.uniform( "noiseTex", 5);
    mNodeSimShader.uniform( "audioData", 6);
    mNodeSimShader.uniform( "gain", mAudioInputHandler.getGain());
    mNodeSimShader.uniform( "dt", (float)simdt );
    mNodeSimShader.uniform( "reset", mReset );
    mNodeSimShader.uniform( "startAnim", mNodeController.isStartingAnim() );
    mNodeSimShader.uniform( "formationStep", mNodeController.getFormationStep() );
    mNodeSimShader.uniform( "motion", mNodeMotion );
    mNodeSimShader.uniform( "containmentRadius", mContainmentRadius );
    
    gl::drawSolidRect(fbo.getBounds());
    
    mNodeSimShader.unbind();
    
    mDynamicTexture.unbindTexture();
    
    mNodeController.getFormation().getPositionTex().unbind();
    mNodeController.getFormation().getVelocityTex().unbind();
    
    fbo.unbindUpdate();
    gl::popMatrices();
}
