#ifndef ONB_H
#define ONB_H

#include "vec3.h"


class onb {
    
    public:

        inline vec3 operator[](int i) const { return axis[i]; }

        vec3 local(double a, double b, double c) const{
            return axis[0] * a + axis[1] * b + axis[2] * c;
        }
        
        vec3 local(const vec3 xyz) const{
            return axis[0] * xyz[0] + axis[1] * xyz[1] + axis[2] * xyz[2];
        }

        vec3 u() const{
            return axis[0];
        }

        vec3 v() const{
            return axis[1];
        }

        vec3 w() const{
            return axis[2];
        }



        void build_from_normal(const vec3& normal){
            axis[2] = unit_vector(normal);
            vec3 crossable = (fabs(axis[2].x()) > 0.9) ? vec3(0,1,0) : vec3(1,0,0);
            vec3 v = unit_vector(cross(axis[2], crossable));
            vec3 u = unit_vector(cross(axis[2], v));

            axis[0] = u;
            axis[1] = v;

        }


    public:
        vec3 axis[3]; 
    
};


#endif