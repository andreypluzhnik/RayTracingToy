#ifndef CONSTANT_MEDIUM_H
#define CONSANT_MEDIUM_H

#include "rtweekend.h"
#include "hittable.h"
#include "material.h"
#include "texture.h"

class constant_medium : public hittable{
    public:
        constant_medium(shared_ptr<hittable> b, double d, shared_ptr<texture> a) : boundary(b), neg_inv_density(-1/d), phase_function(make_shared<isotropic>(a)){}

        constant_medium(shared_ptr<hittable> b, double d, color c) : boundary(b), neg_inv_density(-1/d),  phase_function(make_shared<isotropic>(c)) {}

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override; 

        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override{
            return boundary -> bounding_box(time0, time1, output_box);
        }

    public:
        shared_ptr<material> phase_function;
        // shape of the smoke
        shared_ptr<hittable> boundary; 
        double neg_inv_density;


};

/*
Exerpt from ray tracing the next week:
 The below code assumes that once a ray exits the constant medium boundary, 
 it will continue forever outside the boundary. Put another way, 
 it assumes that the boundary shape is convex. 
 So this particular implementation will work for boundaries like boxes or spheres, 
 but will not work with toruses or shapes that contain voids.
 It's possible to write an implementation that handles arbitrary shapes, 
 but we'll leave that as an exercise for the reader. 

In other words, the problem with non-convex objects, like tori, are their voids; if a ray starts within
the bulk, a hit could register on the inner part of the torus and a diametric region of the inner part. In 
such a case a the hit will be errouneously registered.

*/
bool constant_medium::hit(const ray& r, double t_min, double t_max, hit_record& rec) const{
    const bool enableDebug = false;
    const bool debugging = enableDebug && random_double() < 0.00001; // ?

    hit_record rec1, rec2;
    double ray_length, distance_inside_boundary, hit_distance;
    // rule: at most one scattering per exterior-to-interior boundary penetration
    // corollary: any ray origin within the bulk does not register a hit
    // implementation: return false if front face is false
    // only consider hits from outside, i.e. needs to enter and leave a convex boundary. fails for non-convex objects
    // addendum: if the ray enters from the exterior, scattering is checked through all 'layers' of the surface
    
    rec2.t = -infinity;

    while(true){
        if(!boundary->hit(r, rec2.t+0.0001, infinity, rec1) || !rec1.front_face){
            return false;
        }
        if(!boundary->hit(r, rec1.t+0.0001, infinity, rec2) || !rec2.front_face){
            return false;
        }

        if(debugging) std::cerr << "\nt_min=" << rec1.t << ", t_max=" << rec2.t <<'\n';
        if(rec1.t < t_min) rec1.t = t_min;
        if(rec2.t > t_max) rec2.t = t_max;

        // case not possible? 
        if(rec1.t >= rec2.t)
            return false;
        
        if(rec1.t < 0)
            rec1.t = 0;

        ray_length = r.direction().length(); // used to calculate hit time
        distance_inside_boundary = (rec2.t - rec1.t) * ray_length; // x-ray distance
        hit_distance = neg_inv_density * log(random_double()); // distance at scattering

        if(hit_distance <= distance_inside_boundary)
            break;
        
    
    }

    rec.t = rec1.t + hit_distance / ray_length;
    rec.p = r.at(rec.t);

    if(debugging) {
        std::cerr << "hit distance = " << hit_distance << '\n'
                  << "rec.t = " <<  rec.t << '\n'
                  << "rec.p = " <<  rec.p << '\n';        
    }

    rec.normal = vec3(1,0,0);
    rec.front_face = true;
    rec.mat_ptr = phase_function;
    // std::cerr<<"solid hit"<<'\n'; 
    return true;

}


#endif