#ifndef HITTABLE_H
#define HITTABLE_H

#include "aabb.h"

class material;

struct hit_record{
    point3 p;
    vec3 normal;
    double t;
    double u;
    double v;
    double pdf; // pdf of BRDF
    shared_ptr<material> mat_ptr;
    bool front_face;

    inline void set_face_normal(const ray& r, const vec3& outward_normal){
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal; 
    }
};

class hittable{
    /*
        const = 0 note: A virtual function in a class makes it a POLYMORPHIC base class, where as a 
        pure virtual function (w/ const = 0) makes the base class ABSTRACT, i.e. one that has to be implemented.

        A polymorphic base class is one that can be instantiated but whose virtual members can
        only be accessed through a 'base' reference pointing to a derived class, one that implements
        the virtual methods.

        For instance:
        class Base{
            virtual virtualMethod(){
                ...
            }
            void otherMethod(){
                ...
            }
        
        }
        
        Derived d;
        Base& b = d;
        b->virtualMember(); // calls implementation of virtualMethod by Derived
        b->otherMethod(); // calls implementation of otherMethod by Base
        
        References:
        https://stackoverflow.com/questions/2652198/difference-between-a-virtual-function-and-a-pure-virtual-function
        https://www.programiz.com/cpp-programming/function-overriding
    */
    // 
    public:
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const = 0;
        virtual double pdf_value(const point3& p, const vec3& v) const{
            return 0.0;
        }
        virtual vec3 random(const vec3& o) const {
            return vec3(1,0,0);
        }



        
    

};


class flip_face : public hittable {
    public:
        flip_face(shared_ptr<hittable> p) : ptr(p) {}

        virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override {

            if (!ptr->hit(r, t_min, t_max, rec))
                return false;

            rec.front_face = !rec.front_face;
            return true;
        }

        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            return ptr->bounding_box(time0, time1, output_box);
        }

    public:
        shared_ptr<hittable> ptr;
};



class translate : public hittable {
    public:
        translate(shared_ptr<hittable> p, const vec3& displacement) : ptr(p), offset(displacement) {}

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

    public:
        shared_ptr<hittable> ptr;
        vec3 offset;
};

class scale : public hittable {
    public:
        scale(shared_ptr<hittable> p, double s);
        
         virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override;

        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            output_box = bbox;
            return hasbox;
        }

    public:
        shared_ptr<hittable> ptr;
        double scaling;
        bool hasbox;
        aabb bbox;

};

class rotate_y : public hittable {
    public:
        rotate_y(shared_ptr<hittable> p, double angle);

        virtual bool hit(
            const ray& r, double t_min, double t_max, hit_record& rec) const override;

        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            output_box = bbox;
            return hasbox;
        }

    public:
        shared_ptr<hittable> ptr;
        double sin_theta;
        double cos_theta;
        bool hasbox;
        aabb bbox;
};

class rotate_z : public hittable
{
    public:
        rotate_z(shared_ptr<hittable> p, double angle);

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            output_box = bbox;
            return hasbox;
        }

    public:
        shared_ptr<hittable> ptr;
        double sin_theta;
        double cos_theta;
        bool hasbox;
        aabb bbox;
};


class rotate_x : public hittable
{
    public:
        rotate_x(shared_ptr<hittable> p, double angle);

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
            output_box = bbox;
            return hasbox;
        }

    public:
        shared_ptr<hittable> ptr;
        double sin_theta;
        double cos_theta;
        bool hasbox;
        aabb bbox;
};



scale::scale(shared_ptr<hittable> p, double s) : scaling(s), ptr(p)
{
    hasbox = p->bounding_box(0, 1, bbox);
    bbox.minimum = bbox.minimum * scaling;
    bbox.maximum = bbox.maximum * scaling;
}

bool scale::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto origin = r.origin();
    origin[0] = origin[0] / scaling;
    origin[1] = origin[1] / scaling;
    origin[2] = origin[2] / scaling;
    
    ray scaled_r(origin, r.direction(), r.time());

    if(!ptr->hit(scaled_r, t_min, t_max, rec)) return false;

    rec.p *= scaling;
    
    return true;

    

}

rotate_x::rotate_x(shared_ptr<hittable> p, double angle) : ptr(p) {
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);

    point3 min( infinity,  infinity,  infinity);
    point3 max(-infinity, -infinity, -infinity);

    vec3 test_vector(0,0,0);

    for(int i  = 0; i < 2; i++){
        for(int j = 0; j < 2; j++){
            for(int k = 0; k < 2; k ++){
                auto x = (1 - i) * bbox.min()[0] + i * bbox.max()[0];
                auto y = (1 - j) * bbox.min()[1] + j * bbox.max()[1];
                auto z = (1 - k) * bbox.min()[2] + k * bbox.max()[2];

                test_vector[0] = x * cos_theta + y * sin_theta;
                test_vector[1] = y * cos_theta - x * sin_theta;

                for(int l = 0; l < 3; l++){
                    min[l] = fmin(min[l], test_vector[l]);
                    max[l] = fmax(max[l], test_vector[l]); 
                }

            }
        }
    }

    bbox = aabb(min, max);
}



rotate_y::rotate_y(shared_ptr<hittable> p, double angle) : ptr(p) {
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);

    point3 min( infinity,  infinity,  infinity);
    point3 max(-infinity, -infinity, -infinity);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                auto x = i*bbox.max().x() + (1-i)*bbox.min().x();
                auto y = j*bbox.max().y() + (1-j)*bbox.min().y();
                auto z = k*bbox.max().z() + (1-k)*bbox.min().z();

                auto newx =  cos_theta*x + sin_theta*z;
                auto newz = -sin_theta*x + cos_theta*z;

                vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
                }
            }
        }
    }

    bbox = aabb(min, max);
}

rotate_z::rotate_z(shared_ptr<hittable> p, double angle) : ptr(p) {
    auto radians = degrees_to_radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);

    point3 min( infinity,  infinity,  infinity);
    point3 max(-infinity, -infinity, -infinity);

    vec3 test_vector(0,0,0);

    for(int i  = 0; i < 2; i++){
        for(int j = 0; j < 2; j++){
            for(int k = 0; k < 2; k ++){
                auto x = (1 - i) * bbox.min()[0] + i * bbox.max()[0];
                auto y = (1 - j) * bbox.min()[1] + j * bbox.max()[1];
                auto z = (1 - k) * bbox.min()[2] + k * bbox.max()[2];

                test_vector[0] = y * cos_theta + z * sin_theta;
                test_vector[1] = z * cos_theta - y * sin_theta;

                for(int l = 0; l < 3; l++){
                    min[l] = fmin(min[l], test_vector[l]);
                    max[l] = fmax(max[l], test_vector[l]); 
                }

            }
        }
    }

    bbox = aabb(min, max);
}



bool rotate_x::hit(const ray& r, double t_min, double t_max, hit_record& rec) const{
    auto origin = r.origin();
    auto direction = r.direction();

    origin[1] = r.origin()[1] * cos_theta + r.origin()[2] * sin_theta;
    origin[2] = r.origin()[2] * cos_theta - r.origin()[1] * sin_theta;

    direction[1] = r.direction()[1] * cos_theta + r.direction()[2] * sin_theta;
    direction[2] = r.direction()[2] * cos_theta - r.direction()[1] * sin_theta;

    ray rotated_r(origin, direction, r.time());

    if(!ptr->hit(rotated_r, t_min, t_max, rec)){
        return false;
    }

    auto p = rec.p;
    auto normal = rec.normal;

    p[1] = rec.p[1] * cos_theta - rec.p[2] * sin_theta;
    p[2] = rec.p[2] * cos_theta + rec.p[1] * sin_theta;

    normal[1] = rec.normal[1] * cos_theta - rec.normal[2] * sin_theta;
    normal[2] = rec.normal[2] * cos_theta + rec.normal[1] * sin_theta;

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;

}

bool rotate_y::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto origin = r.origin();
    auto direction = r.direction();

    origin[0] = cos_theta*r.origin()[0] - sin_theta*r.origin()[2];
    origin[2] = sin_theta*r.origin()[0] + cos_theta*r.origin()[2];

    direction[0] = cos_theta*r.direction()[0] - sin_theta*r.direction()[2];
    direction[2] = sin_theta*r.direction()[0] + cos_theta*r.direction()[2];

    ray rotated_r(origin, direction, r.time());

    if (!ptr->hit(rotated_r, t_min, t_max, rec))
        return false;

    auto p = rec.p;
    auto normal = rec.normal;

    p[0] =  cos_theta*rec.p[0] + sin_theta*rec.p[2];
    p[2] = -sin_theta*rec.p[0] + cos_theta*rec.p[2];

    normal[0] =  cos_theta*rec.normal[0] + sin_theta*rec.normal[2];
    normal[2] = -sin_theta*rec.normal[0] + cos_theta*rec.normal[2];

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;
}

bool rotate_z::hit(const ray& r, double t_min, double t_max, hit_record& rec) const{
    auto origin = r.origin();
    auto direction = r.direction();

    origin[0] = r.origin()[0] * cos_theta + r.origin()[1] * sin_theta;
    origin[1] = r.origin()[1] * cos_theta - r.origin()[0] * sin_theta;

    direction[0] = r.direction()[0] * cos_theta + r.direction()[1] * sin_theta;
    direction[1] = r.direction()[1] * cos_theta - r.direction()[0] * sin_theta;

    ray rotated_r(origin, direction, r.time());

    if(!ptr->hit(rotated_r, t_min, t_max, rec)){
        return false;
    }

    auto p = rec.p;
    auto normal = rec.normal;

    p[0] = rec.p[0] * cos_theta - rec.p[1] * sin_theta;
    p[1] = rec.p[1] * cos_theta + rec.p[0] * sin_theta;

    normal[0] = rec.normal[0] * cos_theta - rec.normal[1] * sin_theta;
    normal[1] = rec.normal[1] * cos_theta + rec.normal[0] * sin_theta;

    rec.p = p;
    rec.set_face_normal(rotated_r, normal);

    return true;

}

bool translate::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    ray moved_r(r.origin() - offset, r.direction(), r.time());
    if(!ptr->hit(moved_r, t_min, t_max, rec)){
        return false;
    }

    rec.p += offset;
    rec.set_face_normal(moved_r, rec.normal);

    return true;
}

bool translate::bounding_box(double time0, double time1, aabb& output_box) const {
    if(!ptr -> bounding_box(time0, time1, output_box)){
        return false;
    }

    output_box = aabb(output_box.min() + offset, output_box.max() + offset);
    return true;
}





#endif