/*
    gradient.cpp
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

#include <math.h>

#include "gradient.h"

#define H2C(_r, _g, _b) ColorA((float) _r/255.0, (float) _g/255.0, (float) _b/255.0, 1)

using namespace ci;

Gradient::Gradient()
{
    Randomer.randomize();
    nodes.reserve(10);
};

Gradient::Gradient(int defaultgradient)
{
    Randomer.randomize();
    nodes.reserve(10);
    makeDefaultGradient(defaultgradient);
}

void Gradient::addNode(float location, const ColorA& c)
{
    nodes.push_back(Gradient::GradientNode(location, c));
    
    //all local algorithms rely on the fact, that locations are sorted ascending
    std::sort(nodes.begin(), nodes.end());
}

void Gradient::clear()
{
    nodes.clear();
    nodes.reserve(10);
}

void Gradient::makeDefaultGradient(int defaultgradient, int random_nodes)
{
    clear();
    
    switch (defaultgradient)
    {
        case cBLACK_TO_WHITE:
            addNode(0,      ColorA(0, 0, 0, 1));
            addNode(1,      ColorA(1, 1, 1, 1));
            break;
            
        case cSPECTRUM:
            addNode(0,      ColorA(1, 0, 0, 1));
            addNode(0.25,   ColorA(1, 1, 0, 1)),
            addNode(0.5,    ColorA(0, 1, 0, 1));
            addNode(0.75,   ColorA(0, 1, 1, 1));
            addNode(1,      ColorA(0, 0, 1, 1));
            break;
            
        
        case cINFRARED:
            addNode(0,      ColorA(0.0, 0.0, 0.0, 1));
            addNode(1.0/6,  ColorA(0.0, 0.0, 0.5, 1));
            addNode(2.0/6,  ColorA(0.5, 0.0, 0.5, 1));
            addNode(3.0/6,  ColorA(1.0, 0.0, 0.0, 1));
            addNode(4.0/6,  ColorA(1.0, 0.5, 0.0, 1));
            addNode(5.0/6,  ColorA(1.0, 1.0, 0.0, 1));
            addNode(1,      ColorA(1.0, 1.0, 1.0, 1));
            break;
            
        case cBLACKBODY:
            addNode(0,      ColorA(0.00, 0.00, 0.00, 1));
            addNode(1.0/5,  ColorA(0.00, 0.25, 1.00, 1));
            addNode(2.0/5,  ColorA(0.00, 0.75, 1.00, 1));
            addNode(3.0/5,  ColorA(1.00, 0.25, 0.00, 1));
            addNode(4.0/5,  ColorA(1.00, 0.75, 0.00, 1));
            addNode(1,      ColorA(1.00, 1.00, 1.00, 1));
            break;
            
        case cNEON:
            addNode(0,      H2C(0x00, 0x00, 0x00));
            addNode(0.25,   H2C(0x33, 0x33, 0xFF));
            addNode(0.5,    H2C(0x00, 0x99, 0xFF));
            addNode(0.75,   H2C(0xE6, 0x00, 0x80));
            addNode (1,     H2C(0xFF, 0x00, 0xFF));
            break;
            
        case cWINTER:
            addNode(0,      H2C(0x4C, 0x80, 0xFF));
            addNode(0.5,    H2C(0xE6, 0xE6, 0xE6));
            addNode(1,      H2C(0x99, 0x99, 0x99));
            break;
            
        case cSUMMER:
            addNode(0,      H2C(0x33, 0x4C, 0xFF));
            addNode(0.25,   H2C(0xFF, 0x00, 0x80));
            addNode(0.5,    H2C(0xFF, 0x80, 0x33));
            addNode(0.75,   H2C(0xCC, 0x4C, 0x00));
            addNode(1,      H2C(0xFF, 0xCC, 0x00));
            break;      

        case cRANDOM:
        default:
            makeRandomGradient(random_nodes);
            break;
    }
}

void Gradient::makeRandomGradient(int numberofnodes)
{
    float location, locationMin, locationMax;
    
    clear();
    
    for (int n=0; n < numberofnodes; n++)
    {
        if (n == 0)
            location = 0.0;
        else if (n == numberofnodes-1)
            location = 1.0;
        else
        {
            locationMin = (float) n / (float) numberofnodes;
            locationMax = (float) (n+1) / (float) numberofnodes;
            location = Randomer.randFloat(locationMin, locationMax);
        }
        
        addNode(location, ColorA(Randomer.randFloat(), Randomer.randFloat(), Randomer.randFloat(), 1));
    }  
}

void Gradient::getColor(float location, ColorA& col) const   
{
    float BandWidth, BandLocation, ScaleFactor;
    
    for (int c=0; c < nodes.size() - 1; c++)
    {
        if (location >= nodes[c].location && location <= nodes[c+1].location)
        {
            BandWidth = nodes[c+1].location - nodes[c].location;
            BandLocation = location - nodes[c].location;
            ScaleFactor = BandLocation / BandWidth;
            
            col.r = ScaleFactor * (nodes[c+1].col.r - nodes[c].col.r) + nodes[c].col.r;
            col.g = ScaleFactor * (nodes[c+1].col.g - nodes[c].col.g) + nodes[c].col.g;
            col.b = ScaleFactor * (nodes[c+1].col.b - nodes[c].col.b) + nodes[c].col.b;
            col.a = ScaleFactor * (nodes[c+1].col.a - nodes[c].col.a) + nodes[c].col.a;
            return;
        }
    }
    
    col.r = 0;
    col.g = 0;
    col.b = 0;
    col.a = 0;
}

void Gradient::fillVectorOfColors(int numberofcolors, ColorVector& colors) const
{
    ColorA current_color;
    
    colors.clear();
    colors.reserve(numberofcolors);
    
    for (int i=0; i < numberofcolors; i++)
    {
        getColor((float) i / (float) (numberofcolors - 1), current_color);
        colors.push_back(current_color);
    }
}

#ifdef _WIN32
static inline double round(double val)
{    
    return floor(val + 0.5);
}
#endif

void Gradient::fillVectorOfColors(int numberofcolors, ColorVector8u& colors) const
{
    ColorA current_color;
    
    colors.clear();
    colors.reserve(numberofcolors);
    
    for (int i=0; i < numberofcolors; i++)
    {
        getColor((float) i / (float) (numberofcolors - 1), current_color);
        colors.push_back(Color8u(round(current_color.r * 255), round(current_color.g * 255), round(current_color.b * 255)));
    }
}
