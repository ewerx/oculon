//
//  Waves.h
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-05.
//
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "TextureShaders.h"
#include "FragShader.h"

//
// Waves
//
class Waves : public TextureShaders
{
public:
    Waves();
    virtual ~Waves();
    
private:
    void setupShaders() override;
    
private:
    
    class MultiWave : public FragShader
    {
    public:
        MultiWave();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };
    
    class SparkWave : public FragShader
    {
    public:
        SparkWave();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };
    
    class SineWave : public FragShader
    {
    public:
        SineWave();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };
    
    class Oscilloscope : public FragShader
    {
    public:
        Oscilloscope();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        
    };
};
