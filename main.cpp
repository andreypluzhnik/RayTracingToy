#include <iostream>

#include "rtweekend.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "vec3.h"
#include "checkerboard.h"
#include "moving_sphere.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "torus.h"
#include "triangle.h"
#include "triangle_mesh.h"
#include "pdf.h"

 
// implement multiple importance sampling 

color ray_color(const ray& r, const color& background , const hittable& world, shared_ptr<hittable>& lights, int depth){ 
    hit_record rec;
    
    if(depth <= 0)
        return color(0,0,0);

    if(!world.hit(r, 0.001, infinity, rec))
        return background;

    scatter_record srec;
    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
    
    if (!rec.mat_ptr->scatter(r, rec, srec))
        return emitted;
    
    if(srec.skip_pdf) {
        return srec.attenuation * ray_color(srec.skip_pdf_ray, background, world, lights, depth - 1);
    }


    auto lights_pdf = make_shared<hittable_pdf>(lights, rec.p);
    mixture_pdf mix(lights_pdf, srec.pdf_ptr);

    ray scattered = ray(rec.p, mix.generate(), r.time());
    auto pdf_val = mix.value(scattered.direction());
    // pdf_val = 0.5 * (brdf->value(scattered.direction()) + lights_pdf->value(scattered.direction())); 
    
    // mixture_pdf mpdf(brdf, lights_pdf);
    
    // scattered = ray(rec.p, mpdf.generate(), r.time());
    // pdf_val = mpdf.value(scattered.direction());
    
    return emitted + 
           srec.attenuation * ray_color(scattered, background, world, lights, depth - 1) * rec.mat_ptr->scattering_pdf(r, rec, scattered)/ pdf_val;
 // attempt at two ray samping
    // // send out two rays, one that samples BRDF, one that samples the light only on the first bounce
    // // hit_record rec_brdf, rec_light;
    // hit_record rec;


    // if(depth <= 0)
    //     return color(0,0,0);
    

    // if(!world.hit(r,0.001, infinity, rec))
    //     return background;
    


    // double radiance;
    // ray scattered_brdf;
    // ray scattered_lights; 
    // color attenuation;
    // color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
    // double brdf_value, lights_value;
    // if(!rec.mat_ptr->scatter(r, rec, attenuation, scattered_brdf, brdf_value))
    //     return emitted;


    // hittable_pdf light_pdf(lights, rec.p);
    // scattered_lights = ray(rec.p, light_pdf.generate(), r.time());
    // lights_value = light_pdf.value(scattered_lights.direction());
   
   
    // if(lights_value == 0){
    //     return emitted + attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered_brdf) * ray_color(scattered_brdf, background, world, lights, depth - 1) / brdf_value;
    // }

    // return emitted + attenuation * (rec.mat_ptr->scattering_pdf(r, rec, scattered_brdf) * ray_color(scattered_brdf, background, world, lights, depth - 1) +
    //                                 rec.mat_ptr->scattering_pdf(r, rec, scattered_lights) * ray_color(scattered_lights, background, world, lights, depth - 1) ) / (brdf_value + lights_value);


//    scattered = ray(rec.p, brdf_pdf.generate(), r.time());
   

    // multiple sampling equation
    
    // light sampling
    // construct a pdf from all light sources, then sample. PDF is parameterized by solid angle
    /*
    simple version:
    1) mat brightness
    2) area of light source
    
    better scene convergence:
    3) solid angle on unit sphere

    PS Code
    
    mixture_density md
    for i in lights:
        pdf_of_light = i.get_pdf
        surface_area = i.get_surface_area
        md.add(pdf_of_light, surface_area)

    direction = md.generate()
    &
        vec3 generate():
            each pdf has a continuous interval within 0 to 1, interval size determined by area
            roll number, pick pdf assigned to the range rolled number falls into
            
            picked_pdf
            return picked_pdf.generate()


    &
    
    val = md.value(direction)
    &
        vec3 value(direction)

            
            return picked_pdf.value() * interval

    &
    
    */



}

// color ray_color(const ray& r, const color& background , const hittable& world, int depth){ 
//     hit_record rec;
    
//     if(depth <= 0){
//         return color(0,0,0);
//     }

//     if(!world.hit(r,0.001, infinity, rec))
//         return background;
    
//     ray scattered; 
//     color attenuation;
//     color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
    

//     if(!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
//         return emitted;
  

   
//     return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
// }

hittable_list two_spheres(){
    hittable_list objects;
    auto checker = make_shared<checker_texture>(color(0.2,0.3,0.1), color(0.9,0.9,0.9));
    objects.add(make_shared<sphere>(point3(0,-10,0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0, 10,0), 10, make_shared<lambertian>(checker)));

    return objects;

}

hittable_list two_perlin_spheres(){
    hittable_list objects;
    auto anti_pertex = make_shared<noise_texture>(12.0,true);
    auto pertex = make_shared<noise_texture>(12.0);

    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(anti_pertex)));
    objects.add(make_shared<sphere>(point3(0, 2,0), 2, make_shared<lambertian>(pertex)));

    return objects;

}

hittable_list marble_board(){
    hittable_list objects;
    auto anti_pertex = make_shared<noise_texture>(12.0,true);
    auto pertex = make_shared<noise_texture>(12.0);

    auto cboard = make_shared<checkerboard>(point3(-32,-2,0), point3(0, 1, 0.8), 0.5, 128, 128, vec3(0,0,-1), make_shared<lambertian>(pertex), make_shared<lambertian>(anti_pertex));
    
    objects.add(cboard);

    return objects;


}

hittable_list glossy_sphere_cornell_box(shared_ptr<hittable>& lights){
    hittable_list objects;
    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(20 * color(1, 0.9, 0.7));

    // glossy sphere
    auto glossy_yellow = make_shared<glossy>(color(0.9, 0.9, 0.5), color(0.9, 0.9, 0.9), 0.1, 0.2);
    
    // color pastel_green = color(0.76, 0.88,  0.76);
    lights = make_shared<xz_rect>(113, 443, 127, 432, 554, light);


    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0,555, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(lights));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
    
    // add sphere
    objects.add(make_shared<sphere>(point3(277,100,300), 100, glossy_yellow));

    return objects;


}

hittable_list gloss_pallette_cornell_box(shared_ptr<hittable>& lights){
    hittable_list objects;
    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(12 * color(1, 0.9, 0.7));

    
    
    // color pastel_green = color(0.76, 0.88,  0.76);
    lights = make_shared<xz_rect>(113, 443, 127, 432, 554, light);


    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0,555, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(lights));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    double roughness = 0.01;
    double index_ref = 1.3;
    color c = color(0.31, 0.26, 0.95);

    auto blue_glass = make_shared<dielectric>(index_ref, roughness, c);
    objects.add(make_shared<sphere>(point3(300, 300, 300), 100, blue_glass));
    // // glossy sphere grid
    // double spec_chance_arr[] = {0.05, 0.15, 0.35, 0.47, 0.63, 0.95};
    // for(int x = 0; x < 5; x++){
    //     for(int y = 0; y < 5; y++){
    //         auto roughness = x / 8.0;
    //         auto spec_chance = spec_chance_arr[y];
    //         auto glossy_yellow = make_shared<glossy>(color(0.9, 0.9, 0.5), color(0.9, 0.9, 0.7), roughness, spec_chance);
    //         // add sphere
    //         objects.add(make_shared<sphere>(point3(45 + x * 116.25, 45 + y * 70 , 190 + 50 * y), 35, glossy_yellow));

    //     }
    // }


    return objects;


}



hittable_list plato_bust_cornell_box(){
    hittable_list objects;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0,555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    /* Textured Triangle Mesh*/
    auto mesh_tex = make_shared<image_texture>("plato.jpg");
    auto mesh_mat = make_shared<lambertian>(mesh_tex);
    shared_ptr<hittable> mesh = make_shared<triangle_mesh>("plato.obj", mesh_mat,-1); 
    mesh = make_shared<rotate_y>(mesh,180);
    // mesh = make_shared<rotate_z>(mesh,90);
    // mesh = make_shared<rotate_y>(mesh,-90);
    mesh = make_shared<scale>(mesh, 15);


    mesh = make_shared<translate>(mesh, vec3(315, 150, 320));
    objects.add(mesh);
    


    return objects;
}


hittable_list textured_triangle_mesh_cornell_box(){
    hittable_list objects;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0,555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    /* Textured Triangle Mesh*/
    // auto mesh_tex = make_shared<barycentric_intrp>(color(1,0,0), color(0,1,0), color(0,0,1));
    auto mesh_tex = make_shared<image_texture>("Pepsi_2023.png");
    auto mesh_mat = make_shared<lambertian>(mesh_tex);
    shared_ptr<hittable> mesh = make_shared<triangle_mesh>("bepsi.obj", mesh_mat,-1); 
    mesh = make_shared<rotate_y>(mesh,90);
    mesh = make_shared<rotate_z>(mesh,90);
    mesh = make_shared<rotate_y>(mesh,-90);
    mesh = make_shared<scale>(mesh, 90);


    mesh = make_shared<translate>(mesh, vec3(315, 150, 320));
    objects.add(mesh);
    


    return objects;
}

hittable_list textured_triangle_cornell_box(){
    /* Cornell Box */
    
    hittable_list objects;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0,555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    // /* Textured Triangle */
    auto triangle_color = make_shared<barycentric_intrp>(color(1,0,0), color(0,1,0), color(0,0,1));
    auto triangle_mat = make_shared<lambertian>(triangle_color);
    shared_ptr<hittable> equilateral = make_shared<triangle>(point3(0, 0, 0), point3(50, 100 * sqrt(3) / 2.0, 0), point3(100, 0, 0), triangle_mat, true);
    equilateral = make_shared<scale>(equilateral, 2);
    equilateral = make_shared<rotate_y>(equilateral, 15);
    equilateral = make_shared<translate>(equilateral, vec3(265, 20, 430));
    objects.add(equilateral);
    


    return hittable_list(objects);


}

hittable_list moo_moo_sphere(){
    hittable_list objects;
    auto mm = make_shared<image_texture>("moomoo.jpg");
    auto mm_surface = make_shared<lambertian>(mm);
    auto mm_ball = make_shared<sphere>(point3(0,2,0), 2, mm_surface);

    
    //auto cboard = make_shared<checkerboard>(point3(-32,-2,0), point3(0, 1, 0.8), 0.5, 128, 128, vec3(0,0,-1), material_marble, material_black);
    //objects.add(cboard);
    auto pertex = make_shared<noise_texture>(12.0);
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertex)));
    objects.add(mm_ball);

    return hittable_list(objects);
}

hittable_list simple_light(){
    hittable_list objects;

    auto pertex = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertex)));
    objects.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertex)));
    
    auto difflight = make_shared<diffuse_light>(color(4,4,4));
    objects.add(make_shared<xy_rect>(3,5,1,3,-2, difflight));

    return objects;
}

hittable_list cornell_box(shared_ptr<hittable>& lights){
    hittable_list objects;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    lights = make_shared<xz_rect>(213, 343, 227, 332, 554, light);
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0,555, 0, 555, 0, red));
    objects.add(make_shared<flip_face>(lights));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));
    objects.add(box2);

    
    return objects;


}
hittable_list cornell_smoke() {
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));

    objects.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    objects.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    return objects;
}

hittable_list doughnuts_and_smoke(){
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    auto sunyellow = make_shared<lambertian>(color(0.95,0.82,0.25));


    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));

    shared_ptr<hittable> doughnut = make_shared<torus>(point3(0,0,0),90,30, sunyellow);
    doughnut = make_shared<translate>(doughnut, vec3(180,230,280));
    doughnut = make_shared<rotate_y>(doughnut, - 12);
    objects.add(doughnut);
    // objects.add(make_shared<constant_medium>( doughnut,0.1,color(0.95,0.82,0.25)));
    // objects.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    // objects.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    return objects;
}

hittable_list smoke_box_and_torus(){
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    auto sunyellow = make_shared<lambertian>(color(0.95,0.82,0.25));


    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));

    shared_ptr<hittable> doughnut = make_shared<torus>(point3(0,0,0),90,30, sunyellow);
    doughnut = make_shared<translate>(doughnut, vec3(180,230,280));
    doughnut = make_shared<rotate_y>(doughnut, - 25);
    objects.add(make_shared<constant_medium>( doughnut,0.007,color(0.95,0.82,0.25)));
    objects.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    // objects.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    return objects;

}

hittable_list random_scene(){
    hittable_list world;
    
    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = random() * random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0,.5), 0);
                    world.add(make_shared<moving_sphere>(
                        center, center2, 0.0, 1.0, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;

}




int main(){

    // Image
    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 600; // 3840
    int image_height = static_cast<int>(image_width / aspect_ratio);
    int samples_per_pixel = 80;
    int sqrt_ssp = (int)sqrt(samples_per_pixel);
    int max_depth = 5;

    //world
    hittable_list world;
    //lights
    shared_ptr<hittable> lights;

    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    color background(0,0,0);



    switch(15){
        case 1:
            world = random_scene();
            background = color(0.7,0.8,1);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            aperture = 0.1;
            break;

        case 2:
            world = two_spheres();
            background = color(0.7,0.8,1);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
            break; 
        
        
        case 3:
            world = moo_moo_sphere();
            background = color(0.7,0.8,1);
            lookfrom = point3(5,1,-4);
            lookat = point3(0,0,0);
            vfov = 40.0;
        case 4:           
            world = two_perlin_spheres();
            background = color(0.7,0.8,1);
            lookfrom = point3(13,2,3);
            lookat = point3(0,0,0);
            vfov = 20.0;
        case 5:
            world = marble_board();
            background = color(0.7,0.8,1);
            lookfrom = point3(0, 0.7, 0);
            lookat = point3(0,0.75,-1);
            vfov = 20.0;
        case 6:
            world = simple_light();
            background = color(0,0,0);
            lookfrom = point3(26,3,6);
            lookat = point3(0,2,0);
            vfov = 20.0;
            break;

        case 7:
            world = cornell_box(lights);
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 10;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            background = color(0,0,0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            max_depth = 50;
            break;

        case 8:
            world = cornell_smoke();
            aspect_ratio = 1.0;
            image_width = 1920;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 500;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;
            
        case 9:
            world = doughnuts_and_smoke();
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 50;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;

        case 10:
            world = smoke_box_and_torus();
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 10;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;

        case 11:
            world = textured_triangle_cornell_box();
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 100;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;
        
        case 12:
            world = textured_triangle_mesh_cornell_box();
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 700;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;
        
        case 13:
            world = plato_bust_cornell_box();
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 1000;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            lookfrom = point3(278,278,-800);
            lookat = point3(278,278,0);
            vfov = 40.0;
            break;
        
        case 14:
            world = glossy_sphere_cornell_box(lights);
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 20;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            background = color(0,0,0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            max_depth = 50;
            break;

        default:
        case 15:
            world = gloss_pallette_cornell_box(lights);
            aspect_ratio = 1.0;
            image_width = 600;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 100;
            sqrt_ssp = (int)sqrt(samples_per_pixel);
            background = color(0,0,0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            max_depth = 20;
            break;





        
            

    }

    
    auto dist_to_focus = 1.0;
    vec3 vup(0,1,0);

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    std::cout<<"P3\n"<<image_width<<' '<<image_height<<'\n'<<255<<'\n';
    for(int j = image_height - 1; j>=0; j--){
        std::cerr << "\rScanlines remaining: "<< j << ' '<<std::flush;
        for(int i = 0; i < image_width; i++){
            
            color pixel_color(0,0,0);
            // for (int s = 0; s < samples_per_pixel; ++s) {
            //     auto u = (i + random_double()) / (image_width-1);
            //     auto v = (j + random_double()) / (image_height-1);
            //     ray r = cam.get_ray(u, v);
            //     pixel_color += ray_color(r, world, max_depth);
            // }
            // snippet distributing samples_per_pixel rays EVENLY over a pixel
            auto u = double(i) / (image_width - 1);
            auto v = double(j) / (image_height - 1);
            for(int shift_u = 0; shift_u < sqrt_ssp; shift_u ++){
                u += 1.0/(sqrt_ssp * (image_width - 1));
                for(int shift_v = 0; shift_v < sqrt_ssp; shift_v ++){
                    v += 1.0/(sqrt_ssp * (image_height - 1));
                
                    ray r = cam.get_ray(u,v);
                    pixel_color += ray_color(r, background, world, lights, max_depth);
                }
                v = double(j) / (image_height - 1);
            }
            write_color(std::cout, pixel_color, samples_per_pixel); 

            

        }
    }


}
