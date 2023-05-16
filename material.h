#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "ray.h"
#include "hittable.h"
#include "texture.h"

struct hit_record;

class material {
    public:
        virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
        virtual color emitted(double u, double v, const point3& p) const {
            return color(0,0,0);
        }
};

class lambertian : public material{
    public:
        lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
        lambertian(const shared_ptr<texture> a) : albedo(a){}

        bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override{
            vec3 scatter_direction = rec.normal + random_unit_vector(); 

            if(scatter_direction.near_zero()){
                scatter_direction = rec.normal;
            }

            scattered = ray(rec.p, scatter_direction, r_in.time());
            attenuation = albedo -> value(rec.u,rec.v, rec.p);
            return true;
        }   


    public:
        shared_ptr<texture> albedo;



};

class metal : public material {
    public:
        metal (const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

        bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override{
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered =  ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
            attenuation = albedo;
            return (dot(scattered.direction(),rec.normal) > 0);
        }

    public:
        color albedo; 
        double fuzz;
};


class dielectric : public material {
    public:
        dielectric(double index_of_refraction) : ir(index_of_refraction) {}


        virtual bool scatter( const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
            
            attenuation = color(1.0, 1.0, 1.0);
            double refraction_ratio = rec.front_face ? (1.0/ir) : ir;

            vec3 unit_direction = unit_vector(r_in.direction());
            double cos_theta = -fmin(dot(unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1 - cos_theta * cos_theta);
            
            bool cannot_refract = refraction_ratio * sin_theta > 1.0;

            vec3 direction;
            /*
            * fresnel reflection version
            */ 
            // if(cannot_refract || reflection_coefficient(cos_theta, sin_theta, refraction_ratio) > random_double()){
            //     direction = reflect(unit_direction, rec.normal);
            // }else{
            //     direction = refract(unit_direction, rec.normal, refraction_ratio);
            // }

            /*
            * Schlick's approximation version
            */
            if(cannot_refract || reflection_coefficient(cos_theta, sin_theta, refraction_ratio) > random_double()){
                direction = reflect(unit_direction, rec.normal);
            }else{
                direction = refract(unit_direction, rec.normal, refraction_ratio);
            }
           
            
            

            scattered = ray(rec.p, direction, r_in.time());



            return true;



        }

    public:
        double ir;

    private:
        static double reflection_coefficient(const double& cos_theta, const double& sin_theta, const double& refraction_ratio){
            double s_reflection = (refraction_ratio * cos_theta - sqrt(1 - refraction_ratio * refraction_ratio * sin_theta * sin_theta))
                                  / (refraction_ratio * cos_theta + sqrt(1 - refraction_ratio * refraction_ratio * sin_theta * sin_theta));

            double p_reflection = (refraction_ratio * sqrt(1 - refraction_ratio * refraction_ratio * sin_theta * sin_theta) - cos_theta)
                                  / (refraction_ratio * sqrt(1 - refraction_ratio * refraction_ratio * sin_theta * sin_theta) + cos_theta);

            return 0.5 * (s_reflection * s_reflection  + p_reflection * p_reflection);
        }


        static double reflectance(double cosine, double ref_idx){
            //Schlick's approximation
            double r0 = (1 - ref_idx) / (1 + ref_idx);
            r0 = r0 * r0;
            return r0 + (1 - r0) * pow(1 - cosine, 5);
        }

    
};


class diffuse_light : public material {
    public:
        diffuse_light(shared_ptr<texture> a) : emit(a) {}
        diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

        virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
        const override{ return false; }

        virtual color emitted(double u, double v, const point3& p) const override {
            return emit -> value(u,v,p);
        }

    public:
        shared_ptr<texture> emit;

};

class isotropic : public material {
    public:
        isotropic(color c): albedo(make_shared<solid_color>(c)) {}
        isotropic(shared_ptr<texture> a): albedo(a) {}

        virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
            scattered = ray(rec.p, random_in_unit_sphere(), r_in.time());
            attenuation = albedo->value(rec.u, rec.v, rec.p);
            return true;
        }

    public:
        shared_ptr<texture> albedo;


};  




#endif