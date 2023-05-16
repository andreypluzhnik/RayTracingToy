#ifndef CHECKERBOARD_H
#define CHECKERBOARD_H

#include "hittable.h"
#include "ray.h"
class checkerboard : public hittable {
    public:
        checkerboard(){}
        checkerboard(point3 bottom_left, vec3 normal, double square_size, int x_squares, int y_squares, vec3 vup, shared_ptr<material> white_m, shared_ptr<material> black_m):bottom_left(bottom_left), normal(unit_vector(normal)),
                    square_size(square_size), x_squares(x_squares), y_squares(y_squares), w_ptr(white_m), b_ptr(black_m),vup(vup) {};
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

    public:
        vec3 normal;
        point3 bottom_left;
        double square_size;
        int x_squares;
        int y_squares; 
        shared_ptr<material> w_ptr;
        shared_ptr<material> b_ptr;
        vec3 vup;

        vec3 u = unit_vector(cross(vup,normal));
        vec3 v = unit_vector(cross(normal, u));
        vec3 top_right = bottom_left + u * x_squares * square_size + v * y_squares * square_size;

    private:
        double depth = 0.1;


};


bool checkerboard::hit(const ray& r, double t_min, double t_max, hit_record& rec) const{

    double zero = dot(normal, bottom_left - r.origin()) / dot(normal,r.direction());
    if(zero < t_min || zero > t_max){
        return false;
    }
    vec3 intersection = r.at(zero);

    if( dot(intersection - bottom_left, u) > x_squares * square_size || dot(intersection - bottom_left, u) < 0){
        return false;
    }else if ( dot(intersection - bottom_left, v) > y_squares * square_size || dot(intersection - bottom_left, v) < 0 ){
        return false;
    }

    rec.t = zero;
    rec.p = intersection;
    rec.set_face_normal(r, -normal);
    rec.u = dot(intersection - bottom_left,u) / (square_size * x_squares); 
    rec.v = dot(intersection - bottom_left,v) / (square_size * y_squares);
    if(((int)(dot(intersection - bottom_left,u)/square_size) + (int)(dot(intersection - bottom_left,v) / square_size) % 2) %2 ){
        rec.mat_ptr = b_ptr;
    }else{
        rec.mat_ptr = w_ptr;
    }

    // rec.mat_ptr = b_ptr;

    return true;
}

bool checkerboard::bounding_box(double time0, double time1, aabb& output_box) const{
    output_box = aabb(bottom_left -  normal * depth, bottom_left + u * x_squares * square_size + v * y_squares * square_size);
    return true;
}



#endif