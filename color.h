#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

#include <iostream>



// demofox glossy series

// purpose is to non-linearly stretch and compress rgb colors
// to better emulate human color sensitivity 
// http://hyperphysics.phy-astr.gsu.edu/hbase/vision/colcon.html
// https://en.wikipedia.org/wiki/SRGB 
vec3 linear_to_srgb(color c){
    c[0] = clamp(c.r(), 0, 1.0);
    c[1] = clamp(c.g(), 0, 1.0);
    c[2] = clamp(c.b(), 0 ,1.0);

    return mix(
        c^vec3(1.0/2.4, 1.0/2.4, 1.0/2.4) * 1.055 - vec3(0.055, 0.055, 0.055), 
        c * 12.92,
        less_than(c, 0.0031308)
        );

}

vec3 srgb_to_linear(color c){
    c[0] = clamp(c.r(), 0, 1.0);
    c[1] = clamp(c.g(), 0, 1.0);
    c[2] = clamp(c.b(), 0 ,1.0);

    return mix(
        ((c + (vec3(1,1,1) * 0.055)) / 1.055)^vec3(2.4, 2.4, 2.4), 
        c / 12.92,
        less_than(c, 0.04045)
        );


}



void write_color(std::ostream &out, color pixel_color, int samples_per_pixel) { 
    auto scale = 1.0 / samples_per_pixel;
    
    // pixel_color[0] = sqrt(pixel_color.r());
    // pixel_color[1] = sqrt(pixel_color.g());
    // pixel_color[2] = sqrt(pixel_color.b());

    // pixel_color = linear_to_srgb(pixel_color);
    // pixel_color = pixel_color * scale;

    

    auto r = pixel_color.r();
    auto g = pixel_color.g();
    auto b = pixel_color.b();
    


    
    r = sqrt(r * scale);
    g = sqrt(g * scale);
    b = sqrt(b * scale);

    out << static_cast<int>(256 * clamp(r,0,0.999)) << ' '
        << static_cast<int>(256* clamp(g,0,0.999)) << ' '
        << static_cast<int>(256 * clamp(b,0,0.999)) << '\n';
    
}




#endif