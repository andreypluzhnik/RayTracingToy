#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"
#include "ray.h"

class camera {
    public:
        camera(point3 lookfrom,
               point3 lookat,
               vec3 vup,
               double vfov, //vertical field of view
               double aspect_ratio,
               double aperture,
               double focus_dist,
               double _time0 = 0,
               double _time1 = 0
       ){


            auto theta = degrees_to_radians(vfov);
            auto h = tan(theta/2);
            auto viewport_height = 2.0 * h;
            auto viewport_width = aspect_ratio * viewport_height;

            w = unit_vector(lookfrom - lookat);   
            u = unit_vector(cross(vup,w));
            v = cross(w,u);
            
            origin = lookfrom;
            horizontal = u * viewport_width * focus_dist;
            vertical = v * viewport_height * focus_dist;
            lower_left_corner = origin - w * focus_dist - horizontal / 2 - vertical / 2;
            // horizontal = vec3(viewport_width, 0, 0);
            // vertical = vec3(0, viewport_height, 0);
            // lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0,0,focal_length);

            lens_radius = aperture / 2;
            time0 = _time0;
            time1 = _time1;

        }

         ray get_ray(double s, double t) const {
            vec3 rd = lens_radius * random_in_unit_disk();
            vec3 offset = u * rd.x() + v * rd.y();
            return ray(origin + offset, lower_left_corner + s*horizontal + t*vertical - origin, random_double(time0,time1));
        }


    public:
        vec3 origin;
        vec3 horizontal;
        vec3 vertical;
        vec3 lower_left_corner;
        vec3 u, v, w;
        double lens_radius;
        double time0, time1;

};


#endif