//
//  Contours.h
//  Oculon1
//
//  Created by Ehsan Rezaie on 2015-05-08.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "TextureShaders.h"
#include "FragShader.h"

//
// Contours
//
class Contours : public TextureShaders
{
public:
    Contours();
    virtual ~Contours();
    
private:
    void setupShaders() override;
    
private:
    
#pragma mark -
    class Elevation: public FragShader
    {
    public:
        Elevation();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
        private:
    };
    
#pragma mark -
    class Warping: public FragShader
    {
    public:
        Warping();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
    };
    
#pragma mark -
    class Marble: public FragShader
    {
    public:
        Marble();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
    };
    
};
