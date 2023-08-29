#ifndef PDF_H
#define PDF_H

#include "vec3.h"
#include "onb.h"
#include "hittable.h"

/*
Note: 'const' after function indicates that the function is not going to change the contents of the class.
The ' = 0' are for methods of an abstract class, it indicates that the method HAS to be overwritten by a subclass. 
*/


class pdf {
    public:
        virtual ~pdf(){}
        virtual double value(const vec3& direction) const = 0;
        virtual vec3 generate() const = 0;

};

class mixture_pdf : public pdf{
    public:
        mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
            p[0] = p0;
            p[1] = p1;
        }

        virtual double value(const vec3& direction) const override {
            return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
        }

        virtual vec3 generate() const override {
            if (random_double() < 0.5)
                return p[0]->generate();
            else
                return p[1]->generate();
        }

    public:
        shared_ptr<pdf> p[2];

};

class cosine_pdf : public pdf {

    public: 
        cosine_pdf(const vec3& normal){
            local_basis.build_from_normal(normal);
        }

        virtual double value(const vec3& direction) const override{
            auto cosine = dot(unit_vector(direction), local_basis.w());
            return (cosine <= 0 ) ? 0 : cosine / pi;
            
        }   

        virtual vec3 generate() const override{
            vec3 scatter_direction = local_basis.local(random_cosine_direction());
            return scatter_direction;
        }

    public:
        onb local_basis;

};  

class sphere_pdf : public pdf{
    public:
        sphere_pdf(){}

        double value(const vec3& direction) const override {
            return 1/(4 * pi);
        }

        vec3 generate() const override {
            return random_unit_vector();
        }


};

class hittable_pdf : public pdf {
    public:
        hittable_pdf(shared_ptr<hittable> p, const point3& origin) : o(origin), ptr(p) {}
        
        virtual double value(const vec3& direction) const override {
            return ptr->pdf_value(o, direction);
        }  

        virtual vec3 generate() const override {
            return ptr->random(o);
        } 
    
    public:
        point3 o;
        shared_ptr<hittable> ptr;


};




#endif