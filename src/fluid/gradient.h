/*
    gradient.h
    Dynamic color gradient generator and mananger
 
    syfluid - Mirko's interactive fluidum
    done by sy2002 in 2011 and 2013

    Class for generating gradients of arbitrary colours.
    Colours can be positioned to move them closer together or further apart.
    Default and random gradients can be created.
    An array of color objects of any size can be created.
    Colors are internally represented as Cinder ColorA.
    The class can output them as Color8u as well as ColorA
 
    contact me at code@sy2002.de
 
    Licensed under CC BY 3.0: http://creativecommons.org/licenses/by/3.0/
    You are free to Share (to copy, distribute and transmit the work),
    to Remix (to adapt the work), to make commercial use of the work
    as long as you credit it to sy2002 and link to http://www.sy2002.de

    Gradient class inspired by Phagor's & Toxi's gradient classes
    http://openprocessing.org/visuals/?visualID=197
    http://toxi.co.uk/p5/gradient/gradient.pde
*/

#pragma once

#include <vector>

#include "cinder/Color.h"
#include "cinder/Rand.h"

class Gradient
{
  public:
    
    typedef std::vector<ci::ColorA> ColorVector;
    typedef std::vector<ci::Color8u> ColorVector8u;
    
    //default gradients
    static const int cSPECTRUM          = 0;
    static const int cBLACK_TO_WHITE    = 1;
    static const int cINFRARED          = 2;
    static const int cBLACKBODY         = 3;
    static const int cNEON              = 4;
    static const int cWINTER            = 5;
    static const int cSUMMER            = 6;
    static const int cRANDOM            = 7;
    
    Gradient();
    Gradient(int defaultgradient);

    //clear all nodes, completely reset gradient
    void clear();    
    
    //apply one of the default gradients (including randomly generating one)
    void makeDefaultGradient(int defaultgradient, int random_nodes = 4);
    
    //randomly generate a gradient having specified number of nodes
    void makeRandomGradient(int numberofnodes);
    
    //sort insert a new node
    void addNode(float location, const ci::ColorA& c);

    //performs a linear interpolation to calculate 
    void getColor(float location, ci::ColorA& col) const;
    
    //Calculate a linearily interpolated gradiant vector having specified number of colors
    void fillVectorOfColors(int numberofcolors, ColorVector& colors) const;
    void fillVectorOfColors(int numberofcolors, ColorVector8u& colors) const;
    
  private:
    
    //a gradient is consisting of an arbitrary amount of nodes
    //between the nodes, the gradient is linearily interpolated
    struct GradientNode
    {
        float   location; //placement of this node within the gradient 0.0 to 1.0
        ci::ColorA  col; //color of this node (float/alpha format);
        
        GradientNode(float l, const ci::ColorA& c)
        {
            location = l;
            col = c;
        };
        
        bool operator<(const GradientNode& rhs) const
        {
            return location < rhs.location;
        };
    };

    ci::Rand Randomer;                  //Cinder random generator
    std::vector<GradientNode> nodes;    //list of all color nodes of the gradient 
};

