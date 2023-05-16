#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "ray.h"
#include "rtweekend.h"
#include "vec2.h"


// refer to https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection.html
// for MT triangle intersection algorithm

class triangle : public hittable {
    public:
        triangle(){}

        // normal computed using edges
        triangle(const vec3& vert0, const vec3& vert1, const vec3& vert2, 
        shared_ptr<material> m, const bool dface,
        const vec2& u_offset, const vec2& v_offset, const vec2& w_offset) : 
        v0(vert0), v1(vert1), v2(vert2), 
        vt0(v_offset), vt1(u_offset), vt2(w_offset),
        doubleface(dface), normal(cross(v1 - v0, v2 - v0)), mat_ptr(m){};
        
        // normal precomputed and passed as parameter
        triangle(const vec3& vert0, const vec3& vert1, const vec3& vert2,
        const point3& n, shared_ptr<material> m, const bool dface,
        const vec2& u_offset, const vec2& v_offset, const vec2& w_offset) : 
        v0(vert0), v1(vert1), v2(vert2), 
        vt0(v_offset), vt1(u_offset), vt2(w_offset),
        doubleface(dface), normal(n), mat_ptr(m){};

        // normal computed using edges
        triangle(const vec3& vert0, const vec3& vert1, const vec3& vert2,
        shared_ptr<material> m, const bool dface) : 
        v0(vert0), v1(vert1), v2(vert2), 
        doubleface(dface), normal(cross(v1 - v0, v2 - v0)), mat_ptr(m)
        {
            vt0 = vec2(1.0,0);
            vt1 = vec2(0,1.0);
            vt2 = vec2(0,0);
        };

        // normal precomputed and passed as parameter
        triangle(const vec3& vert0, const vec3& vert1, const vec3& vert2,
        const point3& n, shared_ptr<material> m, const bool dface) : 
        v0(vert0), v1(vert1), v2(vert2), 
        doubleface(dface), normal(n), mat_ptr(m)
        {
            vt0 = vec2(1.0,0);
            vt1 = vec2(0,1.0);
            vt2 = vec2(0,0);
        };

        


        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
        

        // to be implemented
        virtual bool bounding_box(double time0, double time1, aabb& output_rect) const override {
            output_rect = aabb(point3(-1,-1,-1),point3(1,1,1));
            return true;
        }

    public:
        vec3 v0, v1, v2;
        vec2 vt0, vt1, vt2;
        bool doubleface;
        vec3 normal;
        shared_ptr<material> mat_ptr;
        

};

bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const{
            vec3 v01 = v1 - v0;
            vec3 v02 = v2 - v0;
            vec3 p = cross(r.direction(), v02);
            float det = dot(v01, p);

            // no hit if ray is perpendicular to triangle normal
            if(is_zero(det)){
                return false;
            // no hit if backface culling
            }else if(!doubleface && det > 0){
                return false;
            }

            float invDet = 1 / det;
            vec3 vt = r.origin() - v0;
            float u = dot(p,vt) * invDet;
            if(u < 0 || u > 1) return false;

            vec3 q = cross(vt, v01);
            float v = dot(r.direction(), q) * invDet;
            if(v < 0 || u + v > 1) return false;
            float t = dot(q, v02) * invDet;

            if(t < t_min || t > t_max) return false;

            vec2 vt_r = u * vt0 + v * vt1 + (1 - u - v) * vt2;
            rec.t = t;
            rec.u = vt_r[0];
            rec.v = vt_r[1];
            rec.p = r.at(rec.t);
            rec.set_face_normal(r, unit_vector(normal));
            rec.mat_ptr = mat_ptr;

            return true;   


        }


#endif