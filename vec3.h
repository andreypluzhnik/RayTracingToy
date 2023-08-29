#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>
#include "rtweekend.h"

using std::sqrt;

class vec3 {
    public:
        double e[3];
        vec3() : e{0,0,0} {}
        vec3(double e0, double e1, double e2) : e{e0,e1,e2} {}

        double x() const {return e[0];}
        double y() const {return e[1];}
        double z() const {return e[2];}

        double r() {return e[0];}
        double g() const {return e[1];}
        double b() const {return e[2];}

        vec3 operator-() const {return vec3(-e[0], -e[1], -e[2]); }
        double operator[](int i) const {return e[i];}
        double& operator[](int i) {return e[i];}

        vec3& operator+=(const vec3 &v){
            e[0] += v.e[0];
            e[1] += v.e[1];
            e[2] += v.e[2];
            return *this;
        }

        vec3& operator*=(const double t){
            e[0] *= t;
            e[1] *= t;
            e[2] *= t;
            return *this;
        }

        vec3& operator/=(const double t){
            return *this *= 1/t;
        }

        double length() const {
            return sqrt(length_squared());
        }

        double length_squared() const {
            return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
        }

        bool near_zero() const {
            const auto s = 1e-8;
            // return true if vector small in all dimensions 
            return (fabs(e[0] < s) && fabs(e[1] < s) && fabs(e[2] < s));
        }


};


using point3 = vec3;
using color = vec3;

// vec3 utility functions

inline std::ostream& operator<<(std::ostream &out, const vec3 &v){
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3 &u, const vec3 &v){
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3&u, const vec3&v){
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3 &u, const vec3 &v){
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3& v){
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator* (const vec3 &v, double t){
    return t * v;
}

inline vec3 operator/(vec3 v, double t){
    return (1/t) * v;
}

inline vec3 operator^(const vec3& u, const vec3& v){
    return vec3(pow(u.x(), v.x()), pow(u.y(), v.y()), pow(u.z(), v.z()));
}


inline double dot(const vec3 &u, const vec3 &v){
    return u.e[0] * v.e[0]
    + u.e[1] * v.e[1]
    + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3 &u, const vec3 &v){
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1], u.e[2] * v.e[0] - u.e[0] * v.e[2], u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(vec3 v){
    return v / v.length();
}

inline static vec3 random(){
    return vec3(random_double(), random_double(), random_double());
}

inline static vec3 random(double min, double max){
    return vec3(random_double(min,max), random_double(min,max), random_double(min,max));
}

vec3 random_in_unit_sphere(){ 
    while (true) {
            auto p = random(-1,1);
            if (p.length_squared() >= 1) continue;
            return p;
        }
    // black grains when using w/ trilinear interpolated perlin noise for some reason
    // double r = random_double();// generate random point in spherical -> convert to cartesian
    // double theta = random_double(-pi, pi);
    // double phi = random_double(0, 2 * pi);

    // return vec3( r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));

}


vec3 random_cosine_direction(){
    double r1 = random_double();
    double r2 = random_double();

    double x = cos(2 * pi * r1) * sqrt(r2);
    double y = sin(2 * pi * r1) * sqrt(r2);
    double z = sqrt(1 - r2);

    return vec3(x,y,z);

}


vec3 random_unit_vector(){
    return unit_vector(random_in_unit_sphere());
}


vec3 random_in_unit_disk(){
    while(true){
        auto p = vec3(random_double(-1,1), random_double(-1,1), 0);
        if(p.length_squared() >= 1) continue;
        return p;
    }
}
vec3 reflect(const vec3& v, const vec3& n){
    return v - 2 * dot(v,n) * n;
}

vec3 refract(const vec3& uv , const vec3& n, double ir_ratio){ // ir_ratio = index of refraction ratio = n/n', also assume that uv is unit vector
    double cos_theta = fmin(1,dot(uv,n));

    vec3 perp = ir_ratio * (uv - cos_theta * n);
    vec3 par = -sqrt(fabs(1 - perp.length_squared())) * n;
    return perp + par;
}

// linear interpolation between two 3d points
vec3 mix(const vec3& x, const vec3& y, const vec3& a){
    return x * a + y * (vec3(1,1,1) - a);
}

vec3 less_than(const vec3& v, float f){
    return vec3(v.x() < f ? 1 : 0, v.y() < f ? 1 : 0, v.z() < f ? 1 : 0);
}


#endif