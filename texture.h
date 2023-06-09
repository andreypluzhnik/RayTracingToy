#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtweekend.h"
#include "vec3.h"
#include "perlin.h"
#include "rtw_stb_image.h"


class texture {
    public:
        virtual color value(double u, double v, const point3& p) const = 0;
};


class solid_color : public texture {
    public:
        solid_color() {}
        solid_color(color c) : color_value(c) {}

        solid_color(double red, double green, double blue)
          : solid_color(color(red,green,blue)) {}

        virtual color value(double u, double v, const vec3& p) const override {
            return color_value;

        }

    private:
        color color_value;
};

class checker_texture : public texture {
    public:
        checker_texture(){}

        checker_texture(shared_ptr<texture>  _even, shared_ptr<texture> _odd) : even(_even), odd(_odd) {}

        checker_texture(color c1, color c2) : even(make_shared<solid_color>(c1)) , odd(make_shared<solid_color>(c2)) {}

        virtual color value(double u, double v, const point3& p) const override {
            //shirleys texturing
            auto sines = sin(10*p.x())*sin(10*p.y())*sin(10*p.z());
            if (sines < 0){
                return odd->value(u, v, p);
            }else{
                return even->value(u, v, p);
            }


            // my texturing
            // if((((int)(u/0.1)) + (int)(v/0.1) % 2) % 2){
            //     return odd->value(u,v,p);
            // }else{
            //     return even->value(u,v,p);
            // }
        }



    public:
        shared_ptr<texture> odd;
        shared_ptr<texture> even;
};



class noise_texture : public texture {
    public:
        noise_texture() {}
        noise_texture(double sc): scale(sc){}
        noise_texture(double sc, bool flip): scale(sc), anti(flip){} 


        virtual color value(double u, double v, const point3& p) const override{
             if(anti){
                color marbling =  (color(1,1,1) * ( a * sin(k *  p.x() + 50.0 * noise.turb(0.2 * p,10)) - c));
                if(marbling.x() < 0.2) marbling = color(0.2,0.2,0.2);
                return marbling;
             }
             return color(1,1,1) * 0.55 * (1 + sin(scale*((p.z() + p.x())/500.0) + 3.0*noise.turb(p, 4)));

        }

    public:
        double scale;
        double anti = false;
        perlin noise;
                    
        // black marbling variables
        double width = 0.4;
        double separation = 5.0;
        double intensity_at_peak = 0.8;
        double intensity_at_width = 0.5;
        double k = pi / (separation + width);
        double alpha = sin(pi*0.5 / (width/separation + 1.0));
        double c = (intensity_at_peak * alpha - intensity_at_width)/(1 - alpha);
        double a = c + intensity_at_peak;

};

class barycentric_intrp : public texture {
    public:
        barycentric_intrp(const color& vert0, const color& vert1, const color& vert2) : v0(vert0), v1(vert1), v2(vert2){}

        virtual color value(double u, double v, const point3& p) const override {
           
            if(u < 0) u = 0; 
            if(v < 0) v = 0;
            if(u + v > 1){
                double normalization = 1 / (u + v);
                u = u * normalization;
                v = v * normalization;
            }


            // return new color using barycentric coordinates
            return (1 - u - v) * v2 + u * v0 + v * v1;
        }
    
    public: 
        color v0, v1, v2;

};


class image_texture : public texture {
    public:
        const int bytes_per_pixel = 3;

        image_texture()
            : data(nullptr), width(0), height(0), bytes_per_scanline(0){}

        image_texture(const char* filename){
            auto components_per_pixel = bytes_per_pixel;

            data = stbi_load(filename, &width, &height, &components_per_pixel, components_per_pixel);

            if(!data){
                std::cerr<< "ERROR: Could not load texture image file '"<<filename<<"'.\n";
                width = height = 0;
            }

            bytes_per_scanline = bytes_per_pixel * width;

        }


        ~image_texture(){
            delete data;
        }


        virtual color value(double u, double v, const point3& p) const override{
            if(data == nullptr){
                return color(0,1,1);
            }

            u = clamp(u,0.0,1.0);
            v = clamp(v,0.0,1.0);
            // v = 1.0 - clamp(v, 0.0, 1.0);

            auto i = static_cast<int>(u * width);
            auto j = static_cast<int>(v * width);

            if(i >= width) i = width - 1;
            if(j >= height) j = height - 1;

            const auto color_scale = 1.0/255.0;
            auto pixel = data + j * bytes_per_scanline + i * bytes_per_pixel;

            return color(color_scale*pixel[0], color_scale*pixel[1], color_scale*pixel[2]);


        }

        private:
            unsigned char* data;
            int width, height;
            int bytes_per_scanline;




};

#endif