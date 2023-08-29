#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "ray.h"

class sphere : public hittable {
    public:
        sphere(){}
        sphere(point3 cen, double r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {};

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;


    public:
        point3 center;
        double radius;
        shared_ptr<material> mat_ptr;

    private:
        // manifold chart of sphere which generates [0,1] - normalized theta phi coordinates
        // u: normalized phi coord 
        // v: normalized theta coord
        static void get_sphere_uv(const point3& p, double& u, double& v){
            auto theta = acos(p.y());
            auto phi = atan2(-p.z(),p.x()) + pi;
            
            u = phi / (2*pi);
            v = theta / pi;
        }

        // manifold chart from projections of sphere onto R2


};

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {

    point3 op = r.origin() - center;
    double c = dot(op,op) - radius * radius;
    double half_b = dot(r.direction(),op);
    double a = dot(r.direction(), r.direction());

    double determinant = half_b * half_b - a * c;
    if(determinant < 0) return false;

    double sqrtd = sqrt(determinant);
    double zero = (- half_b - sqrtd) / a;

    if(zero < t_min || zero > t_max){
        zero = (-half_b + sqrtd) / a;
        if(zero > t_max || zero < t_min){
            return false;
        }
    }

    rec.t = zero;
    rec.p = r.at(zero);
    auto outward_normal = (r.at(zero) - center) / radius;
    rec.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, rec.u, rec.v);
    rec.mat_ptr = mat_ptr;
    

    return true;

}

bool sphere::bounding_box(double time0, double time1, aabb& output_box) const{
    output_box  = aabb(center - point3(radius,radius,radius), center + point3(radius,radius,radius));  
    return true;
}








#endif