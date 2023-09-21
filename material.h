#ifndef MATERIAL_H
#define MATERIAL_H


#include <unordered_map>
#include "rtweekend.h"
#include "ray.h"
#include "hittable.h"
#include "texture.h"
#include "onb.h"
#include "pdf.h"



struct hit_record;


struct scatter_record {

        color attenuation;
        shared_ptr<pdf> pdf_ptr;
        bool skip_pdf; 
        ray skip_pdf_ray;

};


class material {
    public:
        virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const = 0;
        virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const {
            return color(0,0,0);
        }
        virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const {
            return 0;
        }

};

// note about diamond problem and virtual inheritance https://www.sandordargo.com/blog/2020/12/23/virtual-inheritance
class lambertian : virtual public material{
    public:
        lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
        lambertian(const shared_ptr<texture> a) : albedo(a){}

        virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override{
            // vec3 scatter_direction = rec.normal + random_unit_vector(); 
            srec.attenuation = albedo -> value(rec.u,rec.v, rec.p);
            srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
            srec.skip_pdf = false;

            return true;
        }   

        double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const  {
            auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
            return cosine < 0 ? 0 : cosine / pi;
        }

    public:
        shared_ptr<texture> albedo;



};

class metal : virtual public material {
    public:
        metal (const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

        virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override{
            srec.attenuation = albedo;
            srec.pdf_ptr = nullptr;
            srec.skip_pdf = true;
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            srec.skip_pdf_ray = ray(rec.p, reflected + fuzz*random_in_unit_sphere(), r_in.time());
            return true;
            
            // vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            // scattered =  ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
            // attenuation = albedo;
            // return (dot(scattered.direction(),rec.normal) > 0);
        }

    public:
        color albedo; 
        double fuzz;
};

class glossy : public lambertian, public metal {

    public:

         glossy(color diffuse, color specular, double roughness, double percentSpec) : 
         metal(specular, roughness), lambertian(diffuse), percentSpecular(percentSpec) {}
                                            

        virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override{
            // roll to try specular lights
            bool do_scatter;
            double specular_roll = random_double();
            if(random_double() < percentSpecular)
                do_scatter = metal::scatter(r_in, rec, srec);
            else{
                do_scatter = lambertian::scatter(r_in, rec, srec);
            }

            srec.attenuation = mix(lambertian::albedo -> value(rec.u,rec.v, rec.p), metal::albedo, vec3(1,1,1) * specular_roll);

            return do_scatter;
            

        }

        // double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const  {
        //     return diff->scattering_pdf(r_in, rec, scattered);
        // }
    
    public:
        double percentSpecular;

    

};


class dielectric : public material {
    public:
        dielectric(double index_of_refraction) : ir(index_of_refraction), att(color(1.0, 1.0, 1.0)){}
        dielectric(double index_of_refraction, double r) : ir(index_of_refraction), att(color(1.0, 1.0, 1.0)), roughness(r < 1 ? r : 1){
            delta_theta_in = 2 * asin(roughness / 2);
            asin_refraction_ratio = (index_of_refraction > 1) ? asin(1/index_of_refraction) : asin(index_of_refraction);

        }
        dielectric(double index_of_refraction, double r, color a) :  dielectric(index_of_refraction, r){
            att = a;
        }
        dielectric(double index_of_refraction, double r, double spec_chance, color a) : dielectric(index_of_refraction, r, a) {
            specular_chance = spec_chance; 
        }


        virtual bool scatter( const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
            
            srec.attenuation = att;
            double refraction_ratio = rec.front_face ? (1.0/ir) : ir;
            srec.pdf_ptr = nullptr;
            srec.skip_pdf = true;
            vec3 unit_direction = unit_vector(r_in.direction());
            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
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
            // if(cannot_refract || reflectance(cos_theta, sin_theta, refraction_ratio) > random_double()){
            //     direction = reflect(unit_direction, rec.normal) * (1 - roughness) + roughness * (random_in_unit_sphere());
            // }else{
            //     direction = refract(unit_direction, rec.normal, refraction_ratio) * (1 - roughness/2) + roughness/2 * (random_in_unit_sphere());
            // }



            /*
            * Specular and refraction mixing
            */
            
            if(random_double() < reflectance(cos_theta, sin_theta, refraction_ratio, specular_chance)){
                direction = reflect(unit_direction, rec.normal) * (1 - roughness) + roughness * (random_in_unit_sphere());
            }else{
                /*
                * Calculation of transmission roughness
                */
                double transmission_roughness = roughness * sqrt(1 - refraction_ratio * sin_theta ) / cos_theta;


                direction = refract(unit_direction, rec.normal, refraction_ratio) * (1 - roughness) + roughness * (random_in_unit_sphere());
            }

            

            //scattered ray
            srec.skip_pdf_ray = ray(rec.p, direction, r_in.time());
            return true;



        }

    public:
        color att; 
        double ir, roughness = 0, specular_chance = 0;
        double delta_theta_in;
        double asin_refraction_ratio; // maps refraction ratio to arcsin of refraction ratio
        

    private:
        static double reflection_coefficient(const double& cos_theta, const double& sin_theta, const double& refraction_ratio){
            double s_reflection = (refraction_ratio * cos_theta - sqrt(1 - refraction_ratio * refraction_ratio * sin_theta * sin_theta))
                                  / (refraction_ratio * cos_theta + sqrt(1 - refraction_ratio * refraction_ratio * sin_theta * sin_theta));

            double p_reflection = (refraction_ratio * sqrt(1 - refraction_ratio * refraction_ratio * sin_theta * sin_theta) - cos_theta)
                                  / (refraction_ratio * sqrt(1 - refraction_ratio * refraction_ratio * sin_theta * sin_theta) + cos_theta);

            return 0.5 * (s_reflection * s_reflection  + p_reflection * p_reflection);
        }


        static double reflectance(double cosine, double sin, double refraction_ratio, double min_reflectance){
            //Schlick's approximation
            
            // total internal reflection
            if(refraction_ratio * sin > 1)
                return 1;

            double r0 = (1 - 1/refraction_ratio) / (1 + refraction_ratio) * refraction_ratio;
            r0 = r0 * r0;
            r0 = r0 + (1 - r0) * pow(1 - cosine, 5);
            return min_reflectance + r0 * (1 - min_reflectance);
        }

    
};


class diffuse_light : public material {
    public:
        diffuse_light(shared_ptr<texture> a) : emit(a) {}
        diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

        virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec)
        const override{ return false; }

       virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const override {

            if (rec.front_face)
                return emit->value(u, v, p);
            else
                return color(0,0,0);
        }

    public:
        shared_ptr<texture> emit;

};

class isotropic : public material {
    public:
        isotropic(color c): albedo(make_shared<solid_color>(c)) {}
        isotropic(shared_ptr<texture> a): albedo(a) {}

        virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override {
            srec.attenuation = albedo->value(rec.u, rec.v, rec.p);
            srec.pdf_ptr = make_shared<sphere_pdf>();
            srec.skip_pdf = false;
            return true;
        }

    public:
        shared_ptr<texture> albedo;


};  




#endif