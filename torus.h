#ifndef TORUS_H
#define TORUS_H

#include <algorithm>
#include "hittable.h"
#include "ray.h"

class torus : public hittable {
    public:
        torus(){}
        torus(point3 cen, double r_mjr, double r_mnr, shared_ptr<material> m) : center(cen), radius_major(r_mjr), radius_minor(r_mnr), mat_ptr(m) {};

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;


    public:
        point3 center;
        double radius_major;
        double radius_minor;
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

        // manifold chart from projections of torus onto plane


};

bool torus::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    // solve ray - torus intersection
    // solution is given by quartic ax^4 + bx^3 + cx^2 + dx + e
    int num_roots;
    double major_sq = radius_major * radius_major;
    double minor_sq = radius_minor * radius_minor;
    double alpha = major_sq + minor_sq;
    double o_dot_d = dot(r.origin(),r.direction());
    double o_sq = dot(r.origin(),r.origin());
    double d_sq = dot(r.direction(),r.direction());

    // double c[5] = {o_sq * o_sq -2 * o_sq * alpha - 4 * major_sq * minor_sq + 4 * r.origin().z() * r.origin().z() * major_sq + alpha * alpha,
    //               4 * (o_dot_d * (o_sq - alpha) + 2 * r.origin().z() * r.direction().z() * major_sq),
    //               4 * d_sq * o_dot_d,
    //               d_sq * d_sq};

    double c[5] = {(o_sq - alpha) * (o_sq - alpha) - 4*major_sq * (minor_sq - r.origin().z() * r.origin().z()),
                 4 * (o_sq  - alpha) * o_dot_d + 8 * major_sq * r.origin().z() * r.direction().z(),
                 2 * d_sq * (o_sq - alpha) + 4 * o_dot_d * o_dot_d + 4 * major_sq * r.direction().z() * r.direction().z(),
                 4 * d_sq * o_dot_d,
                 d_sq * d_sq};

    double s[4];
    num_roots = solve_quartic(c,s);
    std::sort(s,s+num_roots);
    for(int i = 0; i < num_roots; i++){
        if(s[i] >= t_min && s[i] <= t_max){
            rec.t = s[i];
            rec.p = r.at(s[i]);
            auto point_on_torus = rec.p - center;
            auto outward_normal = ( point_on_torus - radius_major * unit_vector(point_on_torus - vec3(0,0,point_on_torus.z())) ) / radius_minor;
            rec.set_face_normal(r, outward_normal);
            rec.mat_ptr = mat_ptr;
            return true;
        }
    }

    return false;
}

bool torus::bounding_box(double time0, double time1, aabb& output_box) const{
    output_box  = aabb(center - point3(radius_major + radius_minor,radius_major+radius_minor,radius_major+radius_minor), center + point3(radius_major + radius_minor,radius_major+radius_minor,radius_major+radius_minor));  
    return true;
}



#endif